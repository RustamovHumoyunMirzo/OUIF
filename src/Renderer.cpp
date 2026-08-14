#include <OUIF/Renderer.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
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
    return std::clamp(radius, 0.0f, std::min(rect.width, rect.height) * 0.5f);
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

void append_arc(std::vector<Point>& points, float cx, float cy, float radius, float start, float end)
{
    constexpr int segments = 6;
    if (radius <= 0.0f) {
        points.push_back({ cx, cy });
        return;
    }

    for (int index = 0; index <= segments; ++index) {
        const float t = static_cast<float>(index) / static_cast<float>(segments);
        const float angle = start + (end - start) * t;
        points.push_back({
            cx + std::cos(angle) * radius,
            cy + std::sin(angle) * radius,
        });
    }
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

} // namespace
#endif

struct Renderer::Impl {
    bool initialized = false;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
#if OUIF_WITH_BGFX
    bgfx::ProgramHandle rect_program = BGFX_INVALID_HANDLE;
#endif
};

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
    PosColorVertex::layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    bgfx::Init init;
    init.type = bgfx::RendererType::Count;
    init.resolution.width = config.width;
    init.resolution.height = config.height;
    init.resolution.reset = config.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;

    if (config.native_window != nullptr) {
        init.platformData.nwh = config.native_window;
    }

    if (!bgfx::init(init)) {
        throw std::runtime_error("bgfx initialization failed");
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x121418ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, config.width, config.height);

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
    impl_->width = config.width;
    impl_->height = config.height;
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
    impl_->width = width;
    impl_->height = height;

#if OUIF_WITH_BGFX
    if (impl_->initialized) {
        bgfx::reset(width, height, BGFX_RESET_VSYNC);
        bgfx::setViewRect(0, 0, 0, width, height);
    }
#endif
}

void Renderer::begin_frame(Color clear_color)
{
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
    bgfx::submit(0, impl_->rect_program);
#else
    (void)rect;
    (void)color;
#endif
}

void Renderer::fill_rounded_rect(Rect rect, CornerRadius radius, Color color)
{
#if OUIF_WITH_BGFX
    const float top_left = clamp_radius(radius.top_left, rect);
    const float top_right = clamp_radius(radius.top_right, rect);
    const float bottom_right = clamp_radius(radius.bottom_right, rect);
    const float bottom_left = clamp_radius(radius.bottom_left, rect);

    if (top_left == 0.0f && top_right == 0.0f && bottom_right == 0.0f && bottom_left == 0.0f) {
        fill_rect(rect, color);
        return;
    }

    if (!bgfx::isValid(impl_->rect_program) || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    constexpr float pi = 3.14159265358979323846f;
    std::vector<Point> points;
    points.reserve(32);
    append_arc(points, rect.x + rect.width - top_right, rect.y + top_right, top_right, -pi * 0.5f, 0.0f);
    append_arc(points, rect.x + rect.width - bottom_right, rect.y + rect.height - bottom_right, bottom_right, 0.0f, pi * 0.5f);
    append_arc(points, rect.x + bottom_left, rect.y + rect.height - bottom_left, bottom_left, pi * 0.5f, pi);
    append_arc(points, rect.x + top_left, rect.y + top_left, top_left, pi, pi * 1.5f);

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
