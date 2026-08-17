#include <OUIF/Renderer.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#if OUIF_WITH_BGFX
#include <bgfx/bgfx.h>
#endif

namespace ouif {

#if OUIF_WITH_BGFX
namespace {

struct PosColorVertex {
    float x;
    float y;
    float z;
    std::uint32_t abgr;

    static bgfx::VertexLayout layout;
};

bgfx::VertexLayout PosColorVertex::layout;

std::uint32_t pack_abgr(Color color)
{
    const auto clamp_channel = [](float value) {
        if (value <= 0.0f) {
            return 0U;
        }
        if (value >= 1.0f) {
            return 255U;
        }
        return static_cast<std::uint32_t>(value * 255.0f);
    };

    const auto r = clamp_channel(color.r);
    const auto g = clamp_channel(color.g);
    const auto b = clamp_channel(color.b);
    const auto a = clamp_channel(color.a);
    return (a << 24U) | (b << 16U) | (g << 8U) | r;
}

float clamp_radius(float radius, Rect rect)
{
    const float maximum = std::max(0.0f, std::min(rect.width, rect.height) * 0.5f);
    return std::clamp(radius, 0.0f, maximum);
}

PosColorVertex vertex_from_point(Point point, std::uint32_t width, std::uint32_t height, std::uint32_t abgr)
{
    return {
        (point.x / static_cast<float>(width)) * 2.0f - 1.0f,
        1.0f - (point.y / static_cast<float>(height)) * 2.0f,
        0.0f,
        abgr,
    };
}

Color mix(Color from, Color to, float amount)
{
    const float t = std::clamp(amount, 0.0f, 1.0f);
    return {
        from.r + (to.r - from.r) * t,
        from.g + (to.g - from.g) * t,
        from.b + (to.b - from.b) * t,
        from.a + (to.a - from.a) * t,
    };
}

void append_arc(std::vector<Point>& points, float cx, float cy, float radius, float start, float end, std::uint16_t segments)
{
    if (radius <= 0.0f) {
        points.push_back({ cx, cy });
        return;
    }

    for (std::uint16_t index = 0; index <= segments; ++index) {
        const float t = static_cast<float>(index) / static_cast<float>(segments);
        const float angle = start + (end - start) * t;
        points.push_back({
            cx + std::cos(angle) * radius,
            cy + std::sin(angle) * radius,
        });
    }
}

struct BorderRingPoint {
    Point outer;
    Point inner;
    Color color;
};

void append_border_line(std::vector<BorderRingPoint>& points, Point start, Point end, Point inward, float width, Color color)
{
    if (width <= 0.0f) {
        return;
    }

    if (!points.empty() && points.back().outer.x == start.x && points.back().outer.y == start.y) {
        points.back().color = mix(points.back().color, color, 0.5f);
    } else {
        points.push_back({
            start,
            { start.x + inward.x * width, start.y + inward.y * width },
            color,
        });
    }

    points.push_back({
        end,
        { end.x + inward.x * width, end.y + inward.y * width },
        color,
    });
}

void append_border_arc(
    std::vector<BorderRingPoint>& points,
    float cx,
    float cy,
    float radius,
    float start,
    float end,
    float start_width,
    float end_width,
    Color start_color,
    Color end_color,
    std::uint16_t segments
)
{
    if (radius <= 0.0f) {
        return;
    }

    for (std::uint16_t index = 0; index <= segments; ++index) {
        const float t = static_cast<float>(index) / static_cast<float>(segments);
        const float angle = start + (end - start) * t;
        const float width = start_width + (end_width - start_width) * t;
        const Point normal { std::cos(angle), std::sin(angle) };
        const Point outer { cx + normal.x * radius, cy + normal.y * radius };
        const Point inner { outer.x - normal.x * width, outer.y - normal.y * width };
        const Color color = mix(start_color, end_color, t);

        if (!points.empty() && points.back().outer.x == outer.x && points.back().outer.y == outer.y) {
            points.back().color = mix(points.back().color, color, 0.5f);
            points.back().inner = inner;
        } else {
            points.push_back({ outer, inner, color });
        }
    }
}

std::vector<Point> rounded_rect_points(Rect rect, CornerRadius radius, std::uint16_t segments)
{
    constexpr float pi = 3.14159265358979323846f;
    const float top_left = clamp_radius(radius.top_left, rect);
    const float top_right = clamp_radius(radius.top_right, rect);
    const float bottom_right = clamp_radius(radius.bottom_right, rect);
    const float bottom_left = clamp_radius(radius.bottom_left, rect);

    std::vector<Point> points;
    points.reserve(32);
    append_arc(points, rect.x + rect.width - top_right, rect.y + top_right, top_right, -pi * 0.5f, 0.0f, segments);
    append_arc(points, rect.x + rect.width - bottom_right, rect.y + rect.height - bottom_right, bottom_right, 0.0f, pi * 0.5f, segments);
    append_arc(points, rect.x + bottom_left, rect.y + rect.height - bottom_left, bottom_left, pi * 0.5f, pi, segments);
    append_arc(points, rect.x + top_left, rect.y + top_left, top_left, pi, pi * 1.5f, segments);
    return points;
}

std::vector<char> read_file(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {};
    }

    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> data(static_cast<std::size_t>(size));
    file.read(data.data(), size);
    return data;
}

bgfx::ShaderHandle load_shader(const std::filesystem::path& path)
{
    auto data = read_file(path);
    if (data.empty()) {
        return BGFX_INVALID_HANDLE;
    }

    const bgfx::Memory* memory = bgfx::copy(data.data(), static_cast<std::uint32_t>(data.size()));
    return bgfx::createShader(memory);
}

const char* shader_backend_directory()
{
    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12:
        return "dx11";
    case bgfx::RendererType::Metal:
        return "metal";
    case bgfx::RendererType::OpenGL:
        return "glsl";
    case bgfx::RendererType::Vulkan:
        return "spirv";
    default:
        return "glsl";
    }
}

RendererQualityConfig normalized_quality(RendererQualityConfig quality)
{
    switch (quality.preset) {
    case RendererQuality::Low:
        quality.curve_segments = quality.curve_segments == 12 ? 6 : quality.curve_segments;
        quality.border_curve_segments = quality.border_curve_segments == 24 ? 8 : quality.border_curve_segments;
        quality.msaa_samples = quality.msaa_samples == 4 ? 1 : quality.msaa_samples;
        break;
    case RendererQuality::Balanced:
        quality.curve_segments = quality.curve_segments == 12 ? 10 : quality.curve_segments;
        quality.border_curve_segments = quality.border_curve_segments == 24 ? 16 : quality.border_curve_segments;
        quality.msaa_samples = quality.msaa_samples == 4 ? 2 : quality.msaa_samples;
        break;
    case RendererQuality::Ultra:
        quality.curve_segments = quality.curve_segments == 12 ? 24 : quality.curve_segments;
        quality.border_curve_segments = quality.border_curve_segments == 24 ? 48 : quality.border_curve_segments;
        quality.msaa_samples = quality.msaa_samples == 4 ? 8 : quality.msaa_samples;
        break;
    case RendererQuality::High:
    default:
        break;
    }

    if (!quality.smoothing) {
        quality.msaa_samples = 1;
    }
    quality.curve_segments = std::clamp<std::uint16_t>(quality.curve_segments, 3, 96);
    quality.border_curve_segments = std::clamp<std::uint16_t>(quality.border_curve_segments, 3, 128);
    return quality;
}

std::uint32_t reset_flags(const RendererQualityConfig& quality, bool vsync)
{
    std::uint32_t flags = vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
    if (quality.smoothing) {
        if (quality.msaa_samples >= 16) {
            flags |= BGFX_RESET_MSAA_X16;
        } else if (quality.msaa_samples >= 8) {
            flags |= BGFX_RESET_MSAA_X8;
        } else if (quality.msaa_samples >= 4) {
            flags |= BGFX_RESET_MSAA_X4;
        } else if (quality.msaa_samples >= 2) {
            flags |= BGFX_RESET_MSAA_X2;
        }
    }
    return flags;
}

} // namespace
#endif

namespace {

std::array<std::string_view, 7> glyph_pattern(char raw) noexcept
{
    const char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(raw)));
    switch (ch) {
    case 'A': return { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
    case 'B': return { "11110", "10001", "10001", "11110", "10001", "10001", "11110" };
    case 'C': return { "01111", "10000", "10000", "10000", "10000", "10000", "01111" };
    case 'D': return { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
    case 'E': return { "11111", "10000", "10000", "11110", "10000", "10000", "11111" };
    case 'F': return { "11111", "10000", "10000", "11110", "10000", "10000", "10000" };
    case 'G': return { "01111", "10000", "10000", "10111", "10001", "10001", "01111" };
    case 'H': return { "10001", "10001", "10001", "11111", "10001", "10001", "10001" };
    case 'I': return { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
    case 'J': return { "00111", "00010", "00010", "00010", "00010", "10010", "01100" };
    case 'K': return { "10001", "10010", "10100", "11000", "10100", "10010", "10001" };
    case 'L': return { "10000", "10000", "10000", "10000", "10000", "10000", "11111" };
    case 'M': return { "10001", "11011", "10101", "10101", "10001", "10001", "10001" };
    case 'N': return { "10001", "11001", "10101", "10011", "10001", "10001", "10001" };
    case 'O': return { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
    case 'P': return { "11110", "10001", "10001", "11110", "10000", "10000", "10000" };
    case 'Q': return { "01110", "10001", "10001", "10001", "10101", "10010", "01101" };
    case 'R': return { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
    case 'S': return { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
    case 'T': return { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
    case 'U': return { "10001", "10001", "10001", "10001", "10001", "10001", "01110" };
    case 'V': return { "10001", "10001", "10001", "10001", "10001", "01010", "00100" };
    case 'W': return { "10001", "10001", "10001", "10101", "10101", "10101", "01010" };
    case 'X': return { "10001", "10001", "01010", "00100", "01010", "10001", "10001" };
    case 'Y': return { "10001", "10001", "01010", "00100", "00100", "00100", "00100" };
    case 'Z': return { "11111", "00001", "00010", "00100", "01000", "10000", "11111" };
    case '0': return { "01110", "10001", "10011", "10101", "11001", "10001", "01110" };
    case '1': return { "00100", "01100", "00100", "00100", "00100", "00100", "01110" };
    case '2': return { "01110", "10001", "00001", "00010", "00100", "01000", "11111" };
    case '3': return { "11110", "00001", "00001", "01110", "00001", "00001", "11110" };
    case '4': return { "00010", "00110", "01010", "10010", "11111", "00010", "00010" };
    case '5': return { "11111", "10000", "10000", "11110", "00001", "00001", "11110" };
    case '6': return { "01110", "10000", "10000", "11110", "10001", "10001", "01110" };
    case '7': return { "11111", "00001", "00010", "00100", "01000", "01000", "01000" };
    case '8': return { "01110", "10001", "10001", "01110", "10001", "10001", "01110" };
    case '9': return { "01110", "10001", "10001", "01111", "00001", "00001", "01110" };
    case '.': return { "00000", "00000", "00000", "00000", "00000", "01100", "01100" };
    case ',': return { "00000", "00000", "00000", "00000", "01100", "00100", "01000" };
    case ':': return { "00000", "01100", "01100", "00000", "01100", "01100", "00000" };
    case ';': return { "00000", "01100", "01100", "00000", "01100", "00100", "01000" };
    case '!': return { "00100", "00100", "00100", "00100", "00100", "00000", "00100" };
    case '?': return { "01110", "10001", "00001", "00010", "00100", "00000", "00100" };
    case '-': return { "00000", "00000", "00000", "11111", "00000", "00000", "00000" };
    case '_': return { "00000", "00000", "00000", "00000", "00000", "00000", "11111" };
    case '+': return { "00000", "00100", "00100", "11111", "00100", "00100", "00000" };
    case '/': return { "00001", "00010", "00010", "00100", "01000", "01000", "10000" };
    case '\\': return { "10000", "01000", "01000", "00100", "00010", "00010", "00001" };
    case '(': return { "00010", "00100", "01000", "01000", "01000", "00100", "00010" };
    case ')': return { "01000", "00100", "00010", "00010", "00010", "00100", "01000" };
    case '[': return { "01110", "01000", "01000", "01000", "01000", "01000", "01110" };
    case ']': return { "01110", "00010", "00010", "00010", "00010", "00010", "01110" };
    case '#': return { "01010", "01010", "11111", "01010", "11111", "01010", "01010" };
    case '*': return { "00000", "10101", "01110", "11111", "01110", "10101", "00000" };
    default: return { "11111", "10001", "00010", "00100", "00100", "00000", "00100" };
    }
}

float text_cell_size(const TextStyle& style) noexcept
{
    return std::max(1.0f, style.font_size / 7.0f);
}

float glyph_advance(const TextStyle& style) noexcept
{
    return text_cell_size(style) * 6.0f + style.letter_spacing;
}

float line_height_px(const TextStyle& style) noexcept
{
    return std::max(1.0f, style.font_size * std::max(0.1f, style.line_height));
}

} // namespace

struct Renderer::Impl {
    bool initialized = false;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    RendererQualityConfig quality {};
    std::uint32_t reset_flags = 0;
    std::vector<Rect> clip_stack;
#if OUIF_WITH_BGFX
    bgfx::ProgramHandle rect_program = BGFX_INVALID_HANDLE;
#endif
};

#if OUIF_WITH_BGFX
template <typename Impl>
void apply_scissor(const Impl& impl)
{
    if (impl.clip_stack.empty()) {
        return;
    }

    const auto rect = impl.clip_stack.back();
    const auto x = static_cast<std::uint16_t>(std::clamp(rect.x, 0.0f, static_cast<float>(impl.width)));
    const auto y = static_cast<std::uint16_t>(std::clamp(rect.y, 0.0f, static_cast<float>(impl.height)));
    const auto right = static_cast<std::uint16_t>(std::clamp(rect.x + rect.width, 0.0f, static_cast<float>(impl.width)));
    const auto bottom = static_cast<std::uint16_t>(std::clamp(rect.y + rect.height, 0.0f, static_cast<float>(impl.height)));
    bgfx::setScissor(x, y, static_cast<std::uint16_t>(right - x), static_cast<std::uint16_t>(bottom - y));
}
#endif

Renderer::Renderer()
    : impl_(std::make_unique<Impl>())
{
}

Renderer::~Renderer()
{
    shutdown();
}

Renderer::Renderer(Renderer&&) noexcept = default;
Renderer& Renderer::operator=(Renderer&&) noexcept = default;

void Renderer::initialize(const RendererConfig& config)
{
    if (impl_->initialized) {
        return;
    }

#if OUIF_WITH_BGFX
    impl_->quality = normalized_quality(config.quality);
    impl_->reset_flags = reset_flags(impl_->quality, config.vsync);

    PosColorVertex::layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    bgfx::Init init;
    init.type = config.quality.hardware_acceleration ? bgfx::RendererType::Count : bgfx::RendererType::Noop;
    const auto width = std::max<std::uint32_t>(1U, config.width);
    const auto height = std::max<std::uint32_t>(1U, config.height);
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = impl_->reset_flags;

    if (config.native_window != nullptr) {
        init.platformData.nwh = config.native_window;
    }

    if (!bgfx::init(init)) {
        throw std::runtime_error("bgfx initialization failed");
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x121418ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, width, height);

    const std::filesystem::path shader_root = config.shader_directory != nullptr
        ? std::filesystem::path(config.shader_directory)
#ifdef OUIF_SHADER_DIR
        : std::filesystem::path(OUIF_SHADER_DIR);
#else
        : std::filesystem::path {};
#endif

    if (!shader_root.empty()) {
        const auto backend = shader_backend_directory();
        const auto vertex = load_shader(shader_root / backend / "vs_ouif_rect.bin");
        const auto fragment = load_shader(shader_root / backend / "fs_ouif_rect.bin");
        if (bgfx::isValid(vertex) && bgfx::isValid(fragment)) {
            impl_->rect_program = bgfx::createProgram(vertex, fragment, true);
        } else {
            if (bgfx::isValid(vertex)) {
                bgfx::destroy(vertex);
            }
            if (bgfx::isValid(fragment)) {
                bgfx::destroy(fragment);
            }
        }
    }
#endif

    impl_->initialized = true;
    impl_->width = width;
    impl_->height = height;
}

void Renderer::shutdown() noexcept
{
    if (!impl_ || !impl_->initialized) {
        return;
    }

#if OUIF_WITH_BGFX
    if (bgfx::isValid(impl_->rect_program)) {
        bgfx::destroy(impl_->rect_program);
        impl_->rect_program = BGFX_INVALID_HANDLE;
    }
    bgfx::shutdown();
#endif

    impl_->initialized = false;
}

void Renderer::resize(std::uint32_t width, std::uint32_t height)
{
    width = std::max<std::uint32_t>(1U, width);
    height = std::max<std::uint32_t>(1U, height);
    impl_->width = width;
    impl_->height = height;

#if OUIF_WITH_BGFX
    if (impl_->initialized) {
        bgfx::reset(width, height, impl_->reset_flags);
        bgfx::setViewRect(0, 0, 0, width, height);
    }
#endif
}

void Renderer::begin_frame(Color clear_color)
{
    impl_->clip_stack.clear();
#if OUIF_WITH_BGFX
    const auto r = static_cast<std::uint32_t>(clear_color.r * 255.0f) & 0xffU;
    const auto g = static_cast<std::uint32_t>(clear_color.g * 255.0f) & 0xffU;
    const auto b = static_cast<std::uint32_t>(clear_color.b * 255.0f) & 0xffU;
    const auto a = static_cast<std::uint32_t>(clear_color.a * 255.0f) & 0xffU;
    const auto rgba = (r << 24U) | (g << 16U) | (b << 8U) | a;

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, rgba, 1.0f, 0);
    bgfx::touch(0);
#else
    (void)clear_color;
#endif
}

void Renderer::push_clip(Rect rect)
{
    rect.width = std::max(0.0f, rect.width);
    rect.height = std::max(0.0f, rect.height);
    if (!impl_->clip_stack.empty()) {
        const auto parent = impl_->clip_stack.back();
        const float left = std::max(parent.x, rect.x);
        const float top = std::max(parent.y, rect.y);
        const float right = std::min(parent.x + parent.width, rect.x + rect.width);
        const float bottom = std::min(parent.y + parent.height, rect.y + rect.height);
        rect = {
            left,
            top,
            std::max(0.0f, right - left),
            std::max(0.0f, bottom - top),
        };
    }

    impl_->clip_stack.push_back(rect);

}

void Renderer::pop_clip()
{
    if (impl_->clip_stack.empty()) {
        return;
    }

    impl_->clip_stack.pop_back();
}

void Renderer::fill_rect(Rect rect, Color color)
{
#if OUIF_WITH_BGFX
    if (!bgfx::isValid(impl_->rect_program) || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    if (bgfx::getAvailTransientVertexBuffer(4, PosColorVertex::layout) < 4 || bgfx::getAvailTransientIndexBuffer(6) < 6) {
        return;
    }

    bgfx::TransientVertexBuffer vertices;
    bgfx::TransientIndexBuffer indices;
    bgfx::allocTransientVertexBuffer(&vertices, 4, PosColorVertex::layout);
    bgfx::allocTransientIndexBuffer(&indices, 6);

    const float left = (rect.x / static_cast<float>(impl_->width)) * 2.0f - 1.0f;
    const float right = ((rect.x + rect.width) / static_cast<float>(impl_->width)) * 2.0f - 1.0f;
    const float top = 1.0f - (rect.y / static_cast<float>(impl_->height)) * 2.0f;
    const float bottom = 1.0f - ((rect.y + rect.height) / static_cast<float>(impl_->height)) * 2.0f;
    const auto abgr = pack_abgr(color);

    auto* vertex_data = reinterpret_cast<PosColorVertex*>(vertices.data);
    vertex_data[0] = { left, top, 0.0f, abgr };
    vertex_data[1] = { right, top, 0.0f, abgr };
    vertex_data[2] = { right, bottom, 0.0f, abgr };
    vertex_data[3] = { left, bottom, 0.0f, abgr };

    auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
    const std::array<std::uint16_t, 6> quad_indices { 0, 1, 2, 0, 2, 3 };
    std::copy(quad_indices.begin(), quad_indices.end(), index_data);

    bgfx::setVertexBuffer(0, &vertices);
    bgfx::setIndexBuffer(&indices);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(*impl_);
    bgfx::submit(0, impl_->rect_program);
#else
    (void)rect;
    (void)color;
#endif
}

void Renderer::fill_rounded_rect(Rect rect, CornerRadius radius, Color color)
{
#if OUIF_WITH_BGFX
    if (!bgfx::isValid(impl_->rect_program) || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    const float top_left = clamp_radius(radius.top_left, rect);
    const float top_right = clamp_radius(radius.top_right, rect);
    const float bottom_right = clamp_radius(radius.bottom_right, rect);
    const float bottom_left = clamp_radius(radius.bottom_left, rect);

    if (top_left == 0.0f && top_right == 0.0f && bottom_right == 0.0f && bottom_left == 0.0f) {
        fill_rect(rect, color);
        return;
    }

    const auto points = rounded_rect_points(rect, radius, impl_->quality.curve_segments);

    const std::uint32_t vertex_count = static_cast<std::uint32_t>(points.size() + 1);
    const std::uint32_t index_count = static_cast<std::uint32_t>(points.size() * 3);
    if (vertex_count > bgfx::getAvailTransientVertexBuffer(vertex_count, PosColorVertex::layout)
        || index_count > bgfx::getAvailTransientIndexBuffer(index_count)) {
        return;
    }

    bgfx::TransientVertexBuffer vertices;
    bgfx::TransientIndexBuffer indices;
    bgfx::allocTransientVertexBuffer(&vertices, vertex_count, PosColorVertex::layout);
    bgfx::allocTransientIndexBuffer(&indices, index_count);

    const auto abgr = pack_abgr(color);
    auto* vertex_data = reinterpret_cast<PosColorVertex*>(vertices.data);
    vertex_data[0] = vertex_from_point({ rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f }, impl_->width, impl_->height, abgr);
    for (std::size_t index = 0; index < points.size(); ++index) {
        vertex_data[index + 1] = vertex_from_point(points[index], impl_->width, impl_->height, abgr);
    }

    auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
    for (std::uint16_t index = 0; index < static_cast<std::uint16_t>(points.size()); ++index) {
        const std::uint16_t next = static_cast<std::uint16_t>((index + 1U) % points.size());
        index_data[index * 3U + 0U] = 0;
        index_data[index * 3U + 1U] = static_cast<std::uint16_t>(index + 1U);
        index_data[index * 3U + 2U] = static_cast<std::uint16_t>(next + 1U);
    }

    bgfx::setVertexBuffer(0, &vertices);
    bgfx::setIndexBuffer(&indices);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(*impl_);
    bgfx::submit(0, impl_->rect_program);
#else
    (void)radius;
    fill_rect(rect, color);
#endif
}

void Renderer::stroke_rect(Rect rect, Color color, float width)
{
    if (width <= 0.0f) {
        return;
    }

    fill_rect({ rect.x, rect.y, rect.width, width }, color);
    fill_rect({ rect.x, rect.y + rect.height - width, rect.width, width }, color);
    fill_rect({ rect.x, rect.y, width, rect.height }, color);
    fill_rect({ rect.x + rect.width - width, rect.y, width, rect.height }, color);
}

void Renderer::stroke_rounded_rect(Rect rect, CornerRadius radius, Color color, float width)
{
    stroke_rounded_rect(rect, radius, BorderEdges(Border(color, width)));
}

void Renderer::stroke_rounded_rect(Rect rect, CornerRadius radius, BorderEdges borders)
{
#if OUIF_WITH_BGFX
    if (!bgfx::isValid(impl_->rect_program) || rect.width <= 0.0f || rect.height <= 0.0f || borders.empty()) {
        return;
    }

    const float max_width = std::min(rect.width, rect.height) * 0.5f;
    borders.left.width = std::clamp(borders.left.width, 0.0f, max_width);
    borders.top.width = std::clamp(borders.top.width, 0.0f, max_width);
    borders.right.width = std::clamp(borders.right.width, 0.0f, max_width);
    borders.bottom.width = std::clamp(borders.bottom.width, 0.0f, max_width);

    const float top_left = clamp_radius(radius.top_left, rect);
    const float top_right = clamp_radius(radius.top_right, rect);
    const float bottom_right = clamp_radius(radius.bottom_right, rect);
    const float bottom_left = clamp_radius(radius.bottom_left, rect);
    const auto visible_color = [](Border border) {
        Color color = border.color;
        if (border.width <= 0.0f) {
            color.a = 0.0f;
        }
        return color;
    };

    constexpr float pi = 3.14159265358979323846f;
    std::vector<BorderRingPoint> points;
    points.reserve(72);

    append_border_line(
        points,
        { rect.x + top_left, rect.y },
        { rect.x + rect.width - top_right, rect.y },
        { 0.0f, 1.0f },
        borders.top.width,
        visible_color(borders.top)
    );
    append_border_arc(
        points,
        rect.x + rect.width - top_right,
        rect.y + top_right,
        top_right,
        -pi * 0.5f,
        0.0f,
        borders.top.width,
        borders.right.width,
        visible_color(borders.top),
        visible_color(borders.right),
        impl_->quality.border_curve_segments
    );
    append_border_line(
        points,
        { rect.x + rect.width, rect.y + top_right },
        { rect.x + rect.width, rect.y + rect.height - bottom_right },
        { -1.0f, 0.0f },
        borders.right.width,
        visible_color(borders.right)
    );
    append_border_arc(
        points,
        rect.x + rect.width - bottom_right,
        rect.y + rect.height - bottom_right,
        bottom_right,
        0.0f,
        pi * 0.5f,
        borders.right.width,
        borders.bottom.width,
        visible_color(borders.right),
        visible_color(borders.bottom),
        impl_->quality.border_curve_segments
    );
    append_border_line(
        points,
        { rect.x + rect.width - bottom_right, rect.y + rect.height },
        { rect.x + bottom_left, rect.y + rect.height },
        { 0.0f, -1.0f },
        borders.bottom.width,
        visible_color(borders.bottom)
    );
    append_border_arc(
        points,
        rect.x + bottom_left,
        rect.y + rect.height - bottom_left,
        bottom_left,
        pi * 0.5f,
        pi,
        borders.bottom.width,
        borders.left.width,
        visible_color(borders.bottom),
        visible_color(borders.left),
        impl_->quality.border_curve_segments
    );
    append_border_line(
        points,
        { rect.x, rect.y + rect.height - bottom_left },
        { rect.x, rect.y + top_left },
        { 1.0f, 0.0f },
        borders.left.width,
        visible_color(borders.left)
    );
    append_border_arc(
        points,
        rect.x + top_left,
        rect.y + top_left,
        top_left,
        pi,
        pi * 1.5f,
        borders.left.width,
        borders.top.width,
        visible_color(borders.left),
        visible_color(borders.top),
        impl_->quality.border_curve_segments
    );

    if (points.size() < 2) {
        return;
    }

    const std::uint32_t vertex_count = static_cast<std::uint32_t>(points.size() * 2U);
    const std::uint32_t index_count = static_cast<std::uint32_t>(points.size() * 6U);
    if (vertex_count > bgfx::getAvailTransientVertexBuffer(vertex_count, PosColorVertex::layout)
        || index_count > bgfx::getAvailTransientIndexBuffer(index_count)) {
        return;
    }

    bgfx::TransientVertexBuffer vertices;
    bgfx::TransientIndexBuffer indices;
    bgfx::allocTransientVertexBuffer(&vertices, vertex_count, PosColorVertex::layout);
    bgfx::allocTransientIndexBuffer(&indices, index_count);

    auto* vertex_data = reinterpret_cast<PosColorVertex*>(vertices.data);
    for (std::size_t index = 0; index < points.size(); ++index) {
        const auto abgr = pack_abgr(points[index].color);
        vertex_data[index] = vertex_from_point(points[index].outer, impl_->width, impl_->height, abgr);
        vertex_data[index + points.size()] = vertex_from_point(points[index].inner, impl_->width, impl_->height, abgr);
    }

    auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
    for (std::uint16_t index = 0; index < static_cast<std::uint16_t>(points.size()); ++index) {
        const std::uint16_t next = static_cast<std::uint16_t>((index + 1U) % points.size());
        const std::uint16_t inner_index = static_cast<std::uint16_t>(index + points.size());
        const std::uint16_t inner_next = static_cast<std::uint16_t>(next + points.size());
        index_data[index * 6U + 0U] = index;
        index_data[index * 6U + 1U] = next;
        index_data[index * 6U + 2U] = inner_index;
        index_data[index * 6U + 3U] = next;
        index_data[index * 6U + 4U] = inner_next;
        index_data[index * 6U + 5U] = inner_index;
    }

    bgfx::setVertexBuffer(0, &vertices);
    bgfx::setIndexBuffer(&indices);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(*impl_);
    bgfx::submit(0, impl_->rect_program);
#else
    (void)radius;
    if (borders.top.width > 0.0f) {
        stroke_rect({ rect.x, rect.y, rect.width, rect.height }, borders.top.color, borders.top.width);
    }
#endif
}

Size Renderer::measure_text(std::string_view text, const TextStyle& style) const noexcept
{
    float line_width = 0.0f;
    float max_width = 0.0f;
    std::uint32_t line_count = 1;
    const float advance = glyph_advance(style);
    for (const char ch : text) {
        if (ch == '\n') {
            max_width = std::max(max_width, line_width);
            line_width = 0.0f;
            ++line_count;
            continue;
        }
        line_width += ch == ' ' ? advance * 0.75f : advance;
    }

    max_width = std::max(max_width, line_width);
    return {
        max_width,
        line_height_px(style) * static_cast<float>(line_count),
    };
}

void Renderer::draw_text(std::string_view text, Rect rect, const TextStyle& style)
{
    if (text.empty() || rect.width <= 0.0f || rect.height <= 0.0f || style.color.a <= 0.0f) {
        return;
    }

    push_clip(rect);

    const float cell = text_cell_size(style);
    const float advance = glyph_advance(style);
    const float line_height = line_height_px(style);
    const float baseline_height = cell * 7.0f;

    std::vector<std::string> lines;
    std::size_t start = 0;
    for (std::size_t index = 0; index <= text.size(); ++index) {
        if (index == text.size() || text[index] == '\n') {
            const auto source_line = text.substr(start, index - start);
            if (style.overflow == TextOverflow::Wrap && advance > 0.0f) {
                std::string current;
                float current_width = 0.0f;
                for (const char ch : source_line) {
                    const float ch_width = ch == ' ' ? advance * 0.75f : advance;
                    if (!current.empty() && current_width + ch_width > rect.width) {
                        lines.push_back(current);
                        current.clear();
                        current_width = 0.0f;
                    }
                    current.push_back(ch);
                    current_width += ch_width;
                }
                lines.push_back(std::move(current));
            } else {
                lines.emplace_back(source_line);
            }
            start = index + 1;
        }
    }

    float y = rect.y;
    for (const auto& line : lines) {
        if (y + baseline_height < rect.y) {
            y += line_height;
            continue;
        }
        if (y > rect.y + rect.height) {
            break;
        }

        const float line_width = measure_text(line, style).width;
        float x = rect.x;
        if (style.align == TextAlign::Center) {
            x += std::max(0.0f, (rect.width - line_width) * 0.5f);
        } else if (style.align == TextAlign::End) {
            x += std::max(0.0f, rect.width - line_width);
        }

        for (const char ch : line) {
            if (x > rect.x + rect.width) {
                break;
            }
            if (ch == ' ') {
                x += advance * 0.75f;
                continue;
            }

            const auto pattern = glyph_pattern(ch);
            for (std::size_t row = 0; row < pattern.size(); ++row) {
                for (std::size_t column = 0; column < pattern[row].size(); ++column) {
                    if (pattern[row][column] != '1') {
                        continue;
                    }
                    fill_rect({
                        x + static_cast<float>(column) * cell,
                        y + static_cast<float>(row) * cell,
                        std::max(1.0f, cell),
                        std::max(1.0f, cell),
                    },
                        style.color);
                }
            }

            x += advance;
        }

        y += line_height;
    }

    pop_clip();
}

void Renderer::end_frame()
{
#if OUIF_WITH_BGFX
    bgfx::frame();
#endif
}

bool Renderer::initialized() const noexcept
{
    return impl_ != nullptr && impl_->initialized;
}

Size Renderer::size() const noexcept
{
    return {
        static_cast<float>(impl_->width),
        static_cast<float>(impl_->height),
    };
}

} // namespace ouif
