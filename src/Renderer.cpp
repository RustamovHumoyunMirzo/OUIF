#include <OUIF/Renderer.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#if OUIF_WITH_BGFX
#include <bimg/bimg.h>
#include <bimg/decode.h>
#include <bgfx/bgfx.h>
#include <bx/allocator.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#endif

#if OUIF_WITH_VG_RENDERER
#include <vg/vg.h>
#endif

#if OUIF_WITH_PUGIXML
#include <pugixml.hpp>
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

struct PosColorTexVertex {
    float x;
    float y;
    float z;
    std::uint32_t abgr;
    float u;
    float v;

    static bgfx::VertexLayout layout;
};

bgfx::VertexLayout PosColorTexVertex::layout;

struct FontAtlas {
    int pixel_height = 0;
    int width = 512;
    int height = 512;
    float scale = 1.0f;
    float ascent = 0.0f;
    float descent = 0.0f;
    float line_gap = 0.0f;
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
    std::array<stbtt_packedchar, 95> chars {};
};

struct FontFace {
    std::string family;
    std::vector<unsigned char> data;
    stbtt_fontinfo info {};
    bool valid = false;
    std::unordered_map<int, FontAtlas> atlases;
};

struct RendererImage {
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
    Size size {};
};

enum class SvgShapeType {
    Path,
    Rect,
    Circle,
    Ellipse,
    Line,
    Polyline,
    Polygon,
};

struct SvgPathCommand {
    char command = 'M';
    std::array<float, 6> values {};
};

struct SvgPaint {
    bool fill = true;
    bool stroke = false;
    Color fill_color = Color::rgba(0, 0, 0, 255);
    Color stroke_color = Color::rgba(0, 0, 0, 255);
    float stroke_width = 1.0f;
    float opacity = 1.0f;
};

struct SvgShape {
    SvgShapeType type = SvgShapeType::Path;
    SvgPaint paint {};
    std::vector<SvgPathCommand> path;
    std::vector<float> points;
    Rect rect {};
    CornerRadius radius {};
};

struct RendererVectorImage {
    Size size {};
    Rect view_box {};
    std::vector<SvgShape> shapes;
};

struct Mat3 {
    float a = 1.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 1.0f;
    float tx = 0.0f;
    float ty = 0.0f;
};

Point apply_matrix(Mat3 matrix, Point point) noexcept
{
    return {
        matrix.a * point.x + matrix.c * point.y + matrix.tx,
        matrix.b * point.x + matrix.d * point.y + matrix.ty,
    };
}

std::uint8_t color_channel(float value) noexcept
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

Color multiply_color(Color color, Color tint) noexcept
{
    return {
        color.r * tint.r,
        color.g * tint.g,
        color.b * tint.b,
        color.a * tint.a,
    };
}

#if OUIF_WITH_VG_RENDERER
vg::Color to_vg_color(Color color) noexcept
{
    return vg::color4ub(color_channel(color.r), color_channel(color.g), color_channel(color.b), color_channel(color.a));
}

vg::LineCap::Enum to_vg_line_cap(VectorLineCap cap) noexcept
{
    switch (cap) {
    case VectorLineCap::Round:
        return vg::LineCap::Round;
    case VectorLineCap::Square:
        return vg::LineCap::Square;
    case VectorLineCap::Butt:
    default:
        return vg::LineCap::Butt;
    }
}

vg::LineJoin::Enum to_vg_line_join(VectorLineJoin join) noexcept
{
    switch (join) {
    case VectorLineJoin::Round:
        return vg::LineJoin::Round;
    case VectorLineJoin::Bevel:
        return vg::LineJoin::Bevel;
    case VectorLineJoin::Miter:
    default:
        return vg::LineJoin::Miter;
    }
}

vg::FillRule::Enum to_vg_fill_rule(VectorFillRule rule) noexcept
{
    return rule == VectorFillRule::EvenOdd ? vg::FillRule::EvenOdd : vg::FillRule::NonZero;
}
#endif

Mat3 multiply(Mat3 left, Mat3 right) noexcept
{
    return {
        left.a * right.a + left.c * right.b,
        left.b * right.a + left.d * right.b,
        left.a * right.c + left.c * right.d,
        left.b * right.c + left.d * right.d,
        left.a * right.tx + left.c * right.ty + left.tx,
        left.b * right.tx + left.d * right.ty + left.ty,
    };
}

Mat3 matrix_from_transform(Rect bounds, Transform transform) noexcept
{
    constexpr float pi = 3.14159265358979323846f;
    const float radians = transform.rotation_degrees * pi / 180.0f;
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    const Point origin {
        bounds.x + bounds.width * transform.origin_x,
        bounds.y + bounds.height * transform.origin_y,
    };

    const Mat3 to_origin { 1.0f, 0.0f, 0.0f, 1.0f, -origin.x, -origin.y };
    const Mat3 scaled { transform.scale_x, 0.0f, 0.0f, transform.scale_y, 0.0f, 0.0f };
    const Mat3 rotated { cosine, sine, -sine, cosine, 0.0f, 0.0f };
    const Mat3 back {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        origin.x + transform.translate_x,
        origin.y + transform.translate_y,
    };
    return multiply(back, multiply(rotated, multiply(scaled, to_origin)));
}

bool matrix_is_identity(Mat3 matrix) noexcept
{
    constexpr float epsilon = 0.00001f;
    return std::abs(matrix.a - 1.0f) <= epsilon
        && std::abs(matrix.b) <= epsilon
        && std::abs(matrix.c) <= epsilon
        && std::abs(matrix.d - 1.0f) <= epsilon
        && std::abs(matrix.tx) <= epsilon
        && std::abs(matrix.ty) <= epsilon;
}

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

PosColorTexVertex text_vertex_from_point(
    Point point,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t abgr,
    float u,
    float v
)
{
    return {
        (point.x / static_cast<float>(width)) * 2.0f - 1.0f,
        1.0f - (point.y / static_cast<float>(height)) * 2.0f,
        0.0f,
        abgr,
        u,
        v,
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

std::vector<unsigned char> read_binary_file(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {};
    }

    return {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>(),
    };
}

std::optional<float> parse_svg_float(std::string_view value) noexcept
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    if (value.empty()) {
        return std::nullopt;
    }

    float parsed = 0.0f;
    const auto* first = value.data();
    const auto* last = value.data() + value.size();
    const auto result = std::from_chars(first, last, parsed);
    return result.ec == std::errc() ? std::optional<float>(parsed) : std::nullopt;
}

std::string svg_trim(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return std::string(value);
}

std::string svg_lower(std::string_view value)
{
    std::string copy(value);
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return copy;
}

std::optional<Color> parse_svg_color(std::string_view value)
{
    auto text = svg_lower(svg_trim(value));
    if (text.empty() || text == "none") {
        return std::nullopt;
    }
    if (text == "white") {
        return Color::rgba(255, 255, 255, 255);
    }
    if (text == "black" || text == "currentcolor") {
        return Color::rgba(0, 0, 0, 255);
    }
    if (text == "red") {
        return Color::rgba(255, 0, 0, 255);
    }
    if (text == "green") {
        return Color::rgba(0, 128, 0, 255);
    }
    if (text == "blue") {
        return Color::rgba(0, 0, 255, 255);
    }
    if (text == "transparent") {
        return Color::rgba(0, 0, 0, 0);
    }
    return Color::from_hex(text);
}

void apply_svg_style_value(SvgPaint& paint, std::string_view key, std::string_view value)
{
    const auto name = svg_lower(svg_trim(key));
    const auto text = svg_trim(value);
    if (name == "fill") {
        if (svg_lower(text) == "none") {
            paint.fill = false;
        } else if (auto color = parse_svg_color(text)) {
            paint.fill = true;
            paint.fill_color = *color;
        }
    } else if (name == "stroke") {
        if (svg_lower(text) == "none") {
            paint.stroke = false;
        } else if (auto color = parse_svg_color(text)) {
            paint.stroke = true;
            paint.stroke_color = *color;
        }
    } else if (name == "stroke-width") {
        paint.stroke_width = std::max(0.0f, parse_svg_float(text).value_or(paint.stroke_width));
    } else if (name == "opacity") {
        paint.opacity = std::clamp(parse_svg_float(text).value_or(paint.opacity), 0.0f, 1.0f);
    } else if (name == "fill-opacity") {
        paint.fill_color.a *= std::clamp(parse_svg_float(text).value_or(1.0f), 0.0f, 1.0f);
    } else if (name == "stroke-opacity") {
        paint.stroke_color.a *= std::clamp(parse_svg_float(text).value_or(1.0f), 0.0f, 1.0f);
    }
}

struct SvgPathReader {
    std::string_view text;
    std::size_t index = 0;

    void skip() noexcept
    {
        while (index < text.size() && (std::isspace(static_cast<unsigned char>(text[index])) != 0 || text[index] == ',')) {
            ++index;
        }
    }

    [[nodiscard]] bool has_number() noexcept
    {
        skip();
        return index < text.size() && (std::isdigit(static_cast<unsigned char>(text[index])) != 0 || text[index] == '-' || text[index] == '+' || text[index] == '.');
    }

    std::optional<float> number() noexcept
    {
        skip();
        const auto start = index;
        if (index < text.size() && (text[index] == '-' || text[index] == '+')) {
            ++index;
        }
        while (index < text.size() && std::isdigit(static_cast<unsigned char>(text[index])) != 0) {
            ++index;
        }
        if (index < text.size() && text[index] == '.') {
            ++index;
            while (index < text.size() && std::isdigit(static_cast<unsigned char>(text[index])) != 0) {
                ++index;
            }
        }
        if (index < text.size() && (text[index] == 'e' || text[index] == 'E')) {
            ++index;
            if (index < text.size() && (text[index] == '-' || text[index] == '+')) {
                ++index;
            }
            while (index < text.size() && std::isdigit(static_cast<unsigned char>(text[index])) != 0) {
                ++index;
            }
        }
        return parse_svg_float(text.substr(start, index - start));
    }
};

std::vector<SvgPathCommand> parse_svg_path(std::string_view data)
{
    SvgPathReader reader { data };
    std::vector<SvgPathCommand> commands;
    char command = 'M';
    Point current {};
    Point subpath {};

    while (reader.index < reader.text.size()) {
        reader.skip();
        if (reader.index >= reader.text.size()) {
            break;
        }
        if (std::isalpha(static_cast<unsigned char>(reader.text[reader.index])) != 0) {
            command = reader.text[reader.index++];
        }
        const bool relative = std::islower(static_cast<unsigned char>(command)) != 0;
        const char op = static_cast<char>(std::toupper(static_cast<unsigned char>(command)));

        if (op == 'Z') {
            commands.push_back({ 'Z', {} });
            current = subpath;
            continue;
        }

        if (op != 'M' && op != 'L' && op != 'H' && op != 'V' && op != 'C' && op != 'Q') {
            while (reader.index < reader.text.size() && std::isalpha(static_cast<unsigned char>(reader.text[reader.index])) == 0) {
                ++reader.index;
            }
            continue;
        }

        while (reader.has_number()) {
            SvgPathCommand parsed;
            parsed.command = op;
            if (op == 'M' || op == 'L') {
                auto x = reader.number();
                auto y = reader.number();
                if (!x || !y) {
                    break;
                }
                Point point { *x, *y };
                if (relative) {
                    point.x += current.x;
                    point.y += current.y;
                }
                parsed.values[0] = point.x;
                parsed.values[1] = point.y;
                current = point;
                if (op == 'M') {
                    subpath = point;
                    command = relative ? 'l' : 'L';
                }
            } else if (op == 'H') {
                auto x = reader.number();
                if (!x) {
                    break;
                }
                current.x = relative ? current.x + *x : *x;
                parsed.command = 'L';
                parsed.values[0] = current.x;
                parsed.values[1] = current.y;
            } else if (op == 'V') {
                auto y = reader.number();
                if (!y) {
                    break;
                }
                current.y = relative ? current.y + *y : *y;
                parsed.command = 'L';
                parsed.values[0] = current.x;
                parsed.values[1] = current.y;
            } else if (op == 'C') {
                std::array<float, 6> values {};
                bool ok = true;
                for (auto& value : values) {
                    auto parsed_value = reader.number();
                    if (!parsed_value) {
                        ok = false;
                        break;
                    }
                    value = *parsed_value;
                }
                if (!ok) {
                    break;
                }
                if (relative) {
                    for (int index = 0; index < 6; index += 2) {
                        values[index] += current.x;
                        values[index + 1] += current.y;
                    }
                }
                parsed.values = values;
                current = { values[4], values[5] };
            } else if (op == 'Q') {
                std::array<float, 6> values {};
                bool ok = true;
                for (int index = 0; index < 4; ++index) {
                    auto parsed_value = reader.number();
                    if (!parsed_value) {
                        ok = false;
                        break;
                    }
                    values[index] = *parsed_value;
                }
                if (!ok) {
                    break;
                }
                if (relative) {
                    values[0] += current.x;
                    values[1] += current.y;
                    values[2] += current.x;
                    values[3] += current.y;
                }
                parsed.values = values;
                current = { values[2], values[3] };
            } else {
                break;
            }
            commands.push_back(parsed);
        }
    }

    return commands;
}

std::vector<float> parse_svg_points(std::string_view text)
{
    SvgPathReader reader { text };
    std::vector<float> points;
    while (reader.has_number()) {
        auto x = reader.number();
        auto y = reader.number();
        if (!x || !y) {
            break;
        }
        points.push_back(*x);
        points.push_back(*y);
    }
    return points;
}

#if OUIF_WITH_PUGIXML
SvgPaint svg_paint_from_node(pugi::xml_node node, SvgPaint inherited)
{
    for (auto attr : node.attributes()) {
        apply_svg_style_value(inherited, attr.name(), attr.value());
    }
    if (auto style = node.attribute("style")) {
        std::string_view css = style.value();
        while (!css.empty()) {
            const auto semicolon = css.find(';');
            const auto declaration = semicolon == std::string_view::npos ? css : css.substr(0, semicolon);
            if (const auto colon = declaration.find(':'); colon != std::string_view::npos) {
                apply_svg_style_value(inherited, declaration.substr(0, colon), declaration.substr(colon + 1U));
            }
            if (semicolon == std::string_view::npos) {
                break;
            }
            css.remove_prefix(semicolon + 1U);
        }
    }
    return inherited;
}

void collect_svg_shapes(pugi::xml_node node, SvgPaint paint, std::vector<SvgShape>& shapes)
{
    const auto node_name = svg_lower(node.name());
    if (node_name == "defs" || node_name == "style" || node_name == "metadata" || svg_lower(node.attribute("display").as_string()) == "none") {
        return;
    }

    paint = svg_paint_from_node(node, paint);
    SvgShape shape;
    shape.paint = paint;
    bool append = false;

    if (node_name == "path" && node.attribute("d")) {
        shape.type = SvgShapeType::Path;
        shape.path = parse_svg_path(node.attribute("d").value());
        append = !shape.path.empty();
    } else if (node_name == "rect") {
        shape.type = SvgShapeType::Rect;
        const float x = parse_svg_float(node.attribute("x").value()).value_or(0.0f);
        const float y = parse_svg_float(node.attribute("y").value()).value_or(0.0f);
        const float width = parse_svg_float(node.attribute("width").value()).value_or(0.0f);
        const float height = parse_svg_float(node.attribute("height").value()).value_or(0.0f);
        const float radius = std::max(parse_svg_float(node.attribute("rx").value()).value_or(0.0f), parse_svg_float(node.attribute("ry").value()).value_or(0.0f));
        shape.rect = { x, y, width, height };
        shape.radius = CornerRadius(radius);
        append = width > 0.0f && height > 0.0f;
    } else if (node_name == "circle") {
        shape.type = SvgShapeType::Circle;
        shape.rect = {
            parse_svg_float(node.attribute("cx").value()).value_or(0.0f),
            parse_svg_float(node.attribute("cy").value()).value_or(0.0f),
            parse_svg_float(node.attribute("r").value()).value_or(0.0f),
            0.0f,
        };
        append = shape.rect.width > 0.0f;
    } else if (node_name == "ellipse") {
        shape.type = SvgShapeType::Ellipse;
        shape.rect = {
            parse_svg_float(node.attribute("cx").value()).value_or(0.0f),
            parse_svg_float(node.attribute("cy").value()).value_or(0.0f),
            parse_svg_float(node.attribute("rx").value()).value_or(0.0f),
            parse_svg_float(node.attribute("ry").value()).value_or(0.0f),
        };
        append = shape.rect.width > 0.0f && shape.rect.height > 0.0f;
    } else if (node_name == "line") {
        shape.type = SvgShapeType::Line;
        shape.paint.fill = false;
        if (!shape.paint.stroke) {
            shape.paint.stroke = true;
        }
        shape.rect = {
            parse_svg_float(node.attribute("x1").value()).value_or(0.0f),
            parse_svg_float(node.attribute("y1").value()).value_or(0.0f),
            parse_svg_float(node.attribute("x2").value()).value_or(0.0f),
            parse_svg_float(node.attribute("y2").value()).value_or(0.0f),
        };
        append = true;
    } else if (node_name == "polyline" || node_name == "polygon") {
        shape.type = node_name == "polygon" ? SvgShapeType::Polygon : SvgShapeType::Polyline;
        shape.points = parse_svg_points(node.attribute("points").value());
        append = shape.points.size() >= 4U;
    }

    if (append) {
        shapes.push_back(std::move(shape));
    }

    for (auto child : node.children()) {
        if (child.type() == pugi::node_element) {
            collect_svg_shapes(child, paint, shapes);
        }
    }
}

RendererVectorImage parse_svg_document(std::string_view svg)
{
    RendererVectorImage result;
    pugi::xml_document doc;
    const auto status = doc.load_buffer(svg.data(), svg.size(), pugi::parse_default | pugi::parse_declaration);
    if (!status) {
        return result;
    }

    auto root = doc.child("svg");
    if (!root) {
        root = doc.first_child();
    }
    const float width = parse_svg_float(root.attribute("width").value()).value_or(0.0f);
    const float height = parse_svg_float(root.attribute("height").value()).value_or(0.0f);
    result.size = { width > 0.0f ? width : 100.0f, height > 0.0f ? height : 100.0f };
    result.view_box = { 0.0f, 0.0f, result.size.width, result.size.height };

    if (auto view_box_attr = root.attribute("viewBox")) {
        const auto values = parse_svg_points(view_box_attr.value());
        if (values.size() >= 4U && values[2] > 0.0f && values[3] > 0.0f) {
            result.view_box = { values[0], values[1], values[2], values[3] };
            if (width <= 0.0f) {
                result.size.width = values[2];
            }
            if (height <= 0.0f) {
                result.size.height = values[3];
            }
        }
    }

    SvgPaint paint;
    collect_svg_shapes(root, paint, result.shapes);
    return result;
}
#endif

void draw_svg_shape(VectorCanvas& canvas, const SvgShape& shape, Color tint)
{
    canvas.begin_path();
    switch (shape.type) {
    case SvgShapeType::Path:
        for (const auto& command : shape.path) {
            if (command.command == 'M') {
                canvas.move_to(command.values[0], command.values[1]);
            } else if (command.command == 'L') {
                canvas.line_to(command.values[0], command.values[1]);
            } else if (command.command == 'C') {
                canvas.cubic_to(command.values[0], command.values[1], command.values[2], command.values[3], command.values[4], command.values[5]);
            } else if (command.command == 'Q') {
                canvas.quadratic_to(command.values[0], command.values[1], command.values[2], command.values[3]);
            } else if (command.command == 'Z') {
                canvas.close_path();
            }
        }
        break;
    case SvgShapeType::Rect:
        if (shape.radius.top_left > 0.0f || shape.radius.top_right > 0.0f || shape.radius.bottom_right > 0.0f || shape.radius.bottom_left > 0.0f) {
            canvas.rounded_rect(shape.rect.x, shape.rect.y, shape.rect.width, shape.rect.height, shape.radius);
        } else {
            canvas.rect(shape.rect.x, shape.rect.y, shape.rect.width, shape.rect.height);
        }
        break;
    case SvgShapeType::Circle:
        canvas.circle(shape.rect.x, shape.rect.y, shape.rect.width);
        break;
    case SvgShapeType::Ellipse:
        canvas.ellipse(shape.rect.x, shape.rect.y, shape.rect.width, shape.rect.height);
        break;
    case SvgShapeType::Line:
        canvas.move_to(shape.rect.x, shape.rect.y);
        canvas.line_to(shape.rect.width, shape.rect.height);
        break;
    case SvgShapeType::Polyline:
    case SvgShapeType::Polygon:
        canvas.polyline(shape.points.data(), static_cast<std::uint32_t>(shape.points.size() / 2U));
        if (shape.type == SvgShapeType::Polygon) {
            canvas.close_path();
        }
        break;
    }

    Color fill = multiply_color(shape.paint.fill_color, tint);
    Color stroke = multiply_color(shape.paint.stroke_color, tint);
    fill.a *= shape.paint.opacity;
    stroke.a *= shape.paint.opacity;
    if (shape.paint.fill && fill.a > 0.0f) {
        canvas.fill(fill, VectorFillRule::NonZero, true);
    }
    if (shape.paint.stroke && shape.paint.stroke_width > 0.0f && stroke.a > 0.0f) {
        canvas.stroke(stroke, shape.paint.stroke_width, VectorLineCap::Butt, VectorLineJoin::Round, true);
    }
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

std::string font_key(std::string_view family)
{
    std::string key;
    key.reserve(family.size());
    for (const char ch : family) {
        key.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return key;
}

std::vector<std::filesystem::path> default_font_candidates()
{
    std::vector<std::filesystem::path> paths;
#if defined(_WIN32)
    std::filesystem::path windows_dir = "C:/Windows";
    if (const char* env = std::getenv("WINDIR"); env != nullptr && *env != '\0') {
        windows_dir = env;
    }
    paths.push_back(windows_dir / "Fonts" / "segoeui.ttf");
    paths.push_back(windows_dir / "Fonts" / "arial.ttf");
#elif defined(__APPLE__)
    paths.emplace_back("/System/Library/Fonts/SFNS.ttf");
    paths.emplace_back("/System/Library/Fonts/Supplemental/Arial.ttf");
    paths.emplace_back("/Library/Fonts/Arial.ttf");
#else
    paths.emplace_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    paths.emplace_back("/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf");
    paths.emplace_back("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
#endif
    return paths;
}

void destroy_font_atlases(FontFace& face) noexcept
{
    for (auto& [_, atlas] : face.atlases) {
        if (bgfx::isValid(atlas.texture)) {
            bgfx::destroy(atlas.texture);
            atlas.texture = BGFX_INVALID_HANDLE;
        }
    }
    face.atlases.clear();
}

void destroy_handle(bgfx::ProgramHandle& handle) noexcept
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        handle = BGFX_INVALID_HANDLE;
    }
}

void destroy_handle(bgfx::UniformHandle& handle) noexcept
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        handle = BGFX_INVALID_HANDLE;
    }
}

void destroy_handle(bgfx::FrameBufferHandle& handle) noexcept
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        handle = BGFX_INVALID_HANDLE;
    }
}

void destroy_handle(bgfx::TextureHandle& handle) noexcept
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        handle = BGFX_INVALID_HANDLE;
    }
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

VectorCanvas::VectorCanvas(void* native_context) noexcept
    : native_context_(native_context)
{
}

void VectorCanvas::begin_path()
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::beginPath(context);
    }
#endif
}

void VectorCanvas::move_to(float x, float y)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::moveTo(context, x, y);
    }
#endif
}

void VectorCanvas::line_to(float x, float y)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::lineTo(context, x, y);
    }
#endif
}

void VectorCanvas::cubic_to(float c1x, float c1y, float c2x, float c2y, float x, float y)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::cubicTo(context, c1x, c1y, c2x, c2y, x, y);
    }
#endif
}

void VectorCanvas::quadratic_to(float cx, float cy, float x, float y)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::quadraticTo(context, cx, cy, x, y);
    }
#endif
}

void VectorCanvas::arc(float cx, float cy, float radius, float start_radians, float end_radians, bool clockwise)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::arc(context, cx, cy, radius, start_radians, end_radians, clockwise ? vg::Winding::CW : vg::Winding::CCW);
    }
#endif
}

void VectorCanvas::rect(float x, float y, float width, float height)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::rect(context, x, y, width, height);
    }
#endif
}

void VectorCanvas::rounded_rect(float x, float y, float width, float height, float radius)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::roundedRect(context, x, y, width, height, radius);
    }
#endif
}

void VectorCanvas::rounded_rect(float x, float y, float width, float height, CornerRadius radius)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::roundedRectVarying(context, x, y, width, height, radius.top_left, radius.top_right, radius.bottom_right, radius.bottom_left);
    }
#endif
}

void VectorCanvas::circle(float cx, float cy, float radius)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::circle(context, cx, cy, radius);
    }
#endif
}

void VectorCanvas::ellipse(float cx, float cy, float rx, float ry)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::ellipse(context, cx, cy, rx, ry);
    }
#endif
}

void VectorCanvas::polyline(const float* points, std::uint32_t point_count)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_); context != nullptr && points != nullptr && point_count > 0U) {
        vg::polyline(context, points, point_count);
    }
#endif
}

void VectorCanvas::close_path()
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::closePath(context);
    }
#endif
}

void VectorCanvas::fill(Color color, VectorFillRule rule, bool anti_alias)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        const auto flags = VG_FILL_FLAGS(vg::PathType::Concave, to_vg_fill_rule(rule), anti_alias ? 1 : 0);
        vg::fillPath(context, to_vg_color(color), flags);
    }
#endif
}

void VectorCanvas::stroke(Color color, float width, VectorLineCap cap, VectorLineJoin join, bool anti_alias)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        const auto flags = VG_STROKE_FLAGS(to_vg_line_cap(cap), to_vg_line_join(join), anti_alias ? 1 : 0);
        vg::strokePath(context, to_vg_color(color), std::max(0.0f, width), flags);
    }
#endif
}

void VectorCanvas::push_state()
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::pushState(context);
    }
#endif
}

void VectorCanvas::pop_state()
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::popState(context);
    }
#endif
}

void VectorCanvas::set_alpha(float alpha)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::setGlobalAlpha(context, std::clamp(alpha, 0.0f, 1.0f));
    }
#endif
}

void VectorCanvas::translate(float x, float y)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::transformTranslate(context, x, y);
    }
#endif
}

void VectorCanvas::scale(float x, float y)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::transformScale(context, x, y);
    }
#endif
}

void VectorCanvas::rotate(float radians)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::transformRotate(context, radians);
    }
#endif
}

void VectorCanvas::scissor(Rect rect)
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::setScissor(context, rect.x, rect.y, rect.width, rect.height);
    }
#endif
}

void VectorCanvas::reset_scissor()
{
#if OUIF_WITH_VG_RENDERER
    if (auto* context = static_cast<vg::Context*>(native_context_)) {
        vg::resetScissor(context);
    }
#endif
}

void* VectorCanvas::native_handle() const noexcept
{
    return native_context_;
}

bool VectorCanvas::valid() const noexcept
{
    return native_context_ != nullptr;
}

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
    std::vector<Mat3> transform_stack;
#if OUIF_WITH_BGFX
    bgfx::ProgramHandle rect_program = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle text_program = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle texture_program = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle blur_program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle font_sampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle source_sampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle blur_uniform = BGFX_INVALID_HANDLE;
    bgfx::FrameBufferHandle scene_framebuffer = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle scene_texture = BGFX_INVALID_HANDLE;
    std::unordered_map<std::uint16_t, RendererImage> images;
    std::uint16_t next_image_id = 1U;
    std::unordered_map<std::uint16_t, RendererVectorImage> vector_images;
    std::uint16_t next_vector_image_id = 1U;
    std::unordered_map<std::string, FontFace> fonts;
#if OUIF_WITH_VG_RENDERER
    bx::DefaultAllocator vector_allocator;
    vg::Context* vector_context = nullptr;
#endif
#endif
    std::uint8_t draw_view = 0;
    bool scene_capture_enabled = false;
    std::string default_font_family = "OUIF Sans";
};

template <typename Impl>
Mat3 current_transform(const Impl& impl) noexcept
{
    return impl.transform_stack.empty() ? Mat3 {} : impl.transform_stack.back();
}

template <typename Impl>
PosColorVertex transformed_vertex_from_point(const Impl& impl, Point point, std::uint32_t abgr)
{
    return vertex_from_point(apply_matrix(current_transform(impl), point), impl.width, impl.height, abgr);
}

#if OUIF_WITH_BGFX
template <typename Impl>
PosColorTexVertex transformed_text_vertex_from_point(const Impl& impl, Point point, std::uint32_t abgr, float u, float v)
{
    return text_vertex_from_point(apply_matrix(current_transform(impl), point), impl.width, impl.height, abgr, u, v);
}

template <typename Impl>
PosColorTexVertex transformed_texture_vertex_from_point(const Impl& impl, Point point, std::uint32_t abgr)
{
    const Point transformed = apply_matrix(current_transform(impl), point);
    return text_vertex_from_point(
        transformed,
        impl.width,
        impl.height,
        abgr,
        std::clamp(point.x / static_cast<float>(impl.width), 0.0f, 1.0f),
        std::clamp(point.y / static_cast<float>(impl.height), 0.0f, 1.0f)
    );
}

template <typename Impl>
PosColorTexVertex transformed_texture_vertex_from_point(const Impl& impl, Point point, std::uint32_t abgr, float u, float v)
{
    return text_vertex_from_point(apply_matrix(current_transform(impl), point), impl.width, impl.height, abgr, u, v);
}

template <typename Impl>
void apply_scissor(const Impl& impl)
{
    if (impl.clip_stack.empty()) {
#if OUIF_WITH_BGFX
        bgfx::setScissor();
#endif
        return;
    }

    const auto rect = impl.clip_stack.back();
    const auto x = static_cast<std::uint16_t>(std::clamp(rect.x, 0.0f, static_cast<float>(impl.width)));
    const auto y = static_cast<std::uint16_t>(std::clamp(rect.y, 0.0f, static_cast<float>(impl.height)));
    const auto right = static_cast<std::uint16_t>(std::clamp(rect.x + rect.width, 0.0f, static_cast<float>(impl.width)));
    const auto bottom = static_cast<std::uint16_t>(std::clamp(rect.y + rect.height, 0.0f, static_cast<float>(impl.height)));
    bgfx::setScissor(x, y, static_cast<std::uint16_t>(right - x), static_cast<std::uint16_t>(bottom - y));
}

template <typename Impl>
FontFace* find_font(Impl& impl, std::string_view family)
{
    auto iterator = impl.fonts.find(font_key(family));
    if (iterator != impl.fonts.end()) {
        return &iterator->second;
    }

    iterator = impl.fonts.find(font_key(impl.default_font_family));
    if (iterator != impl.fonts.end()) {
        return &iterator->second;
    }

    return nullptr;
}

template <typename Impl>
const FontFace* find_font(const Impl& impl, std::string_view family)
{
    auto iterator = impl.fonts.find(font_key(family));
    if (iterator != impl.fonts.end()) {
        return &iterator->second;
    }

    iterator = impl.fonts.find(font_key(impl.default_font_family));
    if (iterator != impl.fonts.end()) {
        return &iterator->second;
    }

    return nullptr;
}

template <typename Impl>
FontAtlas* ensure_atlas(Impl& impl, FontFace& face, float font_size)
{
    const int pixel_height = std::clamp(static_cast<int>(std::round(font_size)), 8, 256);
    if (auto iterator = face.atlases.find(pixel_height); iterator != face.atlases.end()) {
        return &iterator->second;
    }

    if (!impl.initialized) {
        return nullptr;
    }

    FontAtlas atlas;
    atlas.pixel_height = pixel_height;
    atlas.width = pixel_height >= 64 ? 1024 : 512;
    atlas.height = atlas.width;
    atlas.scale = stbtt_ScaleForPixelHeight(&face.info, static_cast<float>(pixel_height));

    int ascent = 0;
    int descent = 0;
    int line_gap = 0;
    stbtt_GetFontVMetrics(&face.info, &ascent, &descent, &line_gap);
    atlas.ascent = static_cast<float>(ascent) * atlas.scale;
    atlas.descent = static_cast<float>(descent) * atlas.scale;
    atlas.line_gap = static_cast<float>(line_gap) * atlas.scale;

    std::vector<unsigned char> alpha(static_cast<std::size_t>(atlas.width * atlas.height), 0);
    stbtt_pack_context context {};
    if (!stbtt_PackBegin(&context, alpha.data(), atlas.width, atlas.height, 0, 1, nullptr)) {
        return nullptr;
    }

    const int oversample = impl.quality.preset == RendererQuality::Ultra ? 3 : (impl.quality.smoothing ? 2 : 1);
    stbtt_PackSetOversampling(&context, static_cast<unsigned int>(oversample), static_cast<unsigned int>(oversample));
    const int packed = stbtt_PackFontRange(
        &context,
        face.data.data(),
        0,
        static_cast<float>(pixel_height),
        32,
        static_cast<int>(atlas.chars.size()),
        atlas.chars.data()
    );
    stbtt_PackEnd(&context);
    if (packed == 0) {
        return nullptr;
    }

    std::vector<unsigned char> rgba(static_cast<std::size_t>(atlas.width * atlas.height * 4), 255);
    for (std::size_t index = 0; index < alpha.size(); ++index) {
        rgba[index * 4U + 3U] = alpha[index];
    }

    const std::uint64_t sampler_flags = impl.quality.smoothing
        ? 0
        : BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
    const bgfx::Memory* memory = bgfx::copy(rgba.data(), static_cast<std::uint32_t>(rgba.size()));
    atlas.texture = bgfx::createTexture2D(
        static_cast<std::uint16_t>(atlas.width),
        static_cast<std::uint16_t>(atlas.height),
        false,
        1,
        bgfx::TextureFormat::RGBA8,
        sampler_flags,
        memory
    );
    if (!bgfx::isValid(atlas.texture)) {
        return nullptr;
    }

    auto [iterator, inserted] = face.atlases.emplace(pixel_height, std::move(atlas));
    (void)inserted;
    return &iterator->second;
}

template <typename Impl>
void destroy_scene_targets(Impl& impl) noexcept
{
    impl.scene_texture = BGFX_INVALID_HANDLE;
    destroy_handle(impl.scene_framebuffer);
}

template <typename Impl>
void ensure_scene_targets(Impl& impl)
{
    if (bgfx::isValid(impl.scene_framebuffer)) {
        return;
    }

    const std::uint64_t flags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
    bgfx::TextureHandle texture = bgfx::createTexture2D(
        static_cast<std::uint16_t>(std::max<std::uint32_t>(1U, impl.width)),
        static_cast<std::uint16_t>(std::max<std::uint32_t>(1U, impl.height)),
        false,
        1,
        bgfx::TextureFormat::RGBA8,
        flags
    );
    if (!bgfx::isValid(texture)) {
        return;
    }

    impl.scene_framebuffer = bgfx::createFrameBuffer(1, &texture, true);
    if (!bgfx::isValid(impl.scene_framebuffer)) {
        if (bgfx::isValid(texture)) {
            bgfx::destroy(texture);
        }
        impl.scene_texture = BGFX_INVALID_HANDLE;
        return;
    }
    impl.scene_texture = bgfx::getTexture(impl.scene_framebuffer);
}

template <typename Impl>
void submit_textured_rect(Impl& impl, Rect rect, CornerRadius radius, Color color, bgfx::ProgramHandle program, std::uint8_t view_id)
{
    if (!bgfx::isValid(program) || !bgfx::isValid(impl.scene_texture) || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    const auto abgr = pack_abgr(color);
    const bool rounded = radius.top_left > 0.0f || radius.top_right > 0.0f || radius.bottom_right > 0.0f || radius.bottom_left > 0.0f;
    if (!rounded) {
        if (bgfx::getAvailTransientVertexBuffer(4, PosColorTexVertex::layout) < 4 || bgfx::getAvailTransientIndexBuffer(6) < 6) {
            return;
        }

        bgfx::TransientVertexBuffer vertices;
        bgfx::TransientIndexBuffer indices;
        bgfx::allocTransientVertexBuffer(&vertices, 4, PosColorTexVertex::layout);
        bgfx::allocTransientIndexBuffer(&indices, 6);

        auto* vertex_data = reinterpret_cast<PosColorTexVertex*>(vertices.data);
        vertex_data[0] = transformed_texture_vertex_from_point(impl, { rect.x, rect.y }, abgr);
        vertex_data[1] = transformed_texture_vertex_from_point(impl, { rect.x + rect.width, rect.y }, abgr);
        vertex_data[2] = transformed_texture_vertex_from_point(impl, { rect.x + rect.width, rect.y + rect.height }, abgr);
        vertex_data[3] = transformed_texture_vertex_from_point(impl, { rect.x, rect.y + rect.height }, abgr);

        auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
        const std::array<std::uint16_t, 6> quad_indices { 0, 1, 2, 0, 2, 3 };
        std::copy(quad_indices.begin(), quad_indices.end(), index_data);

        bgfx::setVertexBuffer(0, &vertices);
        bgfx::setIndexBuffer(&indices);
        bgfx::setTexture(0, impl.source_sampler, impl.scene_texture);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
        apply_scissor(impl);
        bgfx::submit(view_id, program);
        return;
    }

    const auto points = rounded_rect_points(rect, radius, impl.quality.curve_segments);
    const std::uint32_t vertex_count = static_cast<std::uint32_t>(points.size() + 1U);
    const std::uint32_t index_count = static_cast<std::uint32_t>(points.size() * 3U);
    if (vertex_count > bgfx::getAvailTransientVertexBuffer(vertex_count, PosColorTexVertex::layout)
        || index_count > bgfx::getAvailTransientIndexBuffer(index_count)) {
        return;
    }

    bgfx::TransientVertexBuffer vertices;
    bgfx::TransientIndexBuffer indices;
    bgfx::allocTransientVertexBuffer(&vertices, vertex_count, PosColorTexVertex::layout);
    bgfx::allocTransientIndexBuffer(&indices, index_count);

    auto* vertex_data = reinterpret_cast<PosColorTexVertex*>(vertices.data);
    vertex_data[0] = transformed_texture_vertex_from_point(impl, { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f }, abgr);
    for (std::size_t index = 0; index < points.size(); ++index) {
        vertex_data[index + 1U] = transformed_texture_vertex_from_point(impl, points[index], abgr);
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
    bgfx::setTexture(0, impl.source_sampler, impl.scene_texture);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(impl);
    bgfx::submit(view_id, program);
}

template <typename Impl>
void submit_image_rect(Impl& impl, bgfx::TextureHandle texture, Rect rect, Color color, float u0, float v0, float u1, float v1, ImageFilter filter)
{
    if (!bgfx::isValid(impl.texture_program) || !bgfx::isValid(impl.source_sampler) || !bgfx::isValid(texture)
        || rect.width <= 0.0f || rect.height <= 0.0f || color.a <= 0.0f) {
        return;
    }
    if (bgfx::getAvailTransientVertexBuffer(4, PosColorTexVertex::layout) < 4 || bgfx::getAvailTransientIndexBuffer(6) < 6) {
        return;
    }

    bgfx::TransientVertexBuffer vertices;
    bgfx::TransientIndexBuffer indices;
    bgfx::allocTransientVertexBuffer(&vertices, 4, PosColorTexVertex::layout);
    bgfx::allocTransientIndexBuffer(&indices, 6);

    const auto abgr = pack_abgr(color);
    auto* vertex_data = reinterpret_cast<PosColorTexVertex*>(vertices.data);
    vertex_data[0] = transformed_texture_vertex_from_point(impl, { rect.x, rect.y }, abgr, u0, v0);
    vertex_data[1] = transformed_texture_vertex_from_point(impl, { rect.x + rect.width, rect.y }, abgr, u1, v0);
    vertex_data[2] = transformed_texture_vertex_from_point(impl, { rect.x + rect.width, rect.y + rect.height }, abgr, u1, v1);
    vertex_data[3] = transformed_texture_vertex_from_point(impl, { rect.x, rect.y + rect.height }, abgr, u0, v1);

    auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
    const std::array<std::uint16_t, 6> quad_indices { 0, 1, 2, 0, 2, 3 };
    std::copy(quad_indices.begin(), quad_indices.end(), index_data);

    std::uint32_t sampler_flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
    if (filter == ImageFilter::Nearest) {
        sampler_flags |= BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
    }

    bgfx::setVertexBuffer(0, &vertices);
    bgfx::setIndexBuffer(&indices);
    bgfx::setTexture(0, impl.source_sampler, texture, sampler_flags);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(impl);
    bgfx::submit(impl.draw_view, impl.texture_program);
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
    PosColorTexVertex::layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
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

    impl_->width = width;
    impl_->height = height;

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

        const auto text_vertex = load_shader(shader_root / backend / "vs_ouif_text.bin");
        const auto text_fragment = load_shader(shader_root / backend / "fs_ouif_text.bin");
        if (bgfx::isValid(text_vertex) && bgfx::isValid(text_fragment)) {
            impl_->text_program = bgfx::createProgram(text_vertex, text_fragment, true);
            impl_->font_sampler = bgfx::createUniform("s_font", bgfx::UniformType::Sampler);
        } else {
            if (bgfx::isValid(text_vertex)) {
                bgfx::destroy(text_vertex);
            }
            if (bgfx::isValid(text_fragment)) {
                bgfx::destroy(text_fragment);
            }
        }

        const auto texture_vertex = load_shader(shader_root / backend / "vs_ouif_text.bin");
        const auto texture_fragment = load_shader(shader_root / backend / "fs_ouif_texture.bin");
        if (bgfx::isValid(texture_vertex) && bgfx::isValid(texture_fragment)) {
            impl_->texture_program = bgfx::createProgram(texture_vertex, texture_fragment, true);
        } else {
            if (bgfx::isValid(texture_vertex)) {
                bgfx::destroy(texture_vertex);
            }
            if (bgfx::isValid(texture_fragment)) {
                bgfx::destroy(texture_fragment);
            }
        }

        const auto blur_vertex = load_shader(shader_root / backend / "vs_ouif_text.bin");
        const auto blur_fragment = load_shader(shader_root / backend / "fs_ouif_blur.bin");
        if (bgfx::isValid(blur_vertex) && bgfx::isValid(blur_fragment)) {
            impl_->blur_program = bgfx::createProgram(blur_vertex, blur_fragment, true);
            impl_->blur_uniform = bgfx::createUniform("u_blur", bgfx::UniformType::Vec4);
        } else {
            if (bgfx::isValid(blur_vertex)) {
                bgfx::destroy(blur_vertex);
            }
            if (bgfx::isValid(blur_fragment)) {
                bgfx::destroy(blur_fragment);
            }
        }
        if ((bgfx::isValid(impl_->texture_program) || bgfx::isValid(impl_->blur_program)) && !bgfx::isValid(impl_->source_sampler)) {
            impl_->source_sampler = bgfx::createUniform("s_source", bgfx::UniformType::Sampler);
        }
    }
    ensure_scene_targets(*impl_);
#if OUIF_WITH_VG_RENDERER
    impl_->vector_context = vg::createContext(&impl_->vector_allocator);
#endif
#endif

    impl_->initialized = true;
    if (impl_->default_font_family == "OUIF Sans") {
        load_default_system_font();
    }
}

void Renderer::shutdown() noexcept
{
    if (!impl_ || !impl_->initialized) {
        return;
    }

#if OUIF_WITH_BGFX
    for (auto& [_, face] : impl_->fonts) {
        destroy_font_atlases(face);
    }
    for (auto& [_, image] : impl_->images) {
        destroy_handle(image.texture);
    }
    impl_->images.clear();
    impl_->vector_images.clear();
#if OUIF_WITH_VG_RENDERER
    if (impl_->vector_context != nullptr) {
        vg::destroyContext(impl_->vector_context);
        impl_->vector_context = nullptr;
    }
#endif
    destroy_scene_targets(*impl_);
    destroy_handle(impl_->blur_uniform);
    destroy_handle(impl_->source_sampler);
    destroy_handle(impl_->font_sampler);
    destroy_handle(impl_->blur_program);
    destroy_handle(impl_->texture_program);
    destroy_handle(impl_->text_program);
    destroy_handle(impl_->rect_program);
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
        destroy_scene_targets(*impl_);
        bgfx::reset(width, height, impl_->reset_flags);
        ensure_scene_targets(*impl_);
    }
#endif
}

void Renderer::begin_frame(Color clear_color)
{
    impl_->clip_stack.clear();
    impl_->transform_stack.clear();
#if OUIF_WITH_BGFX
    const auto r = static_cast<std::uint32_t>(clear_color.r * 255.0f) & 0xffU;
    const auto g = static_cast<std::uint32_t>(clear_color.g * 255.0f) & 0xffU;
    const auto b = static_cast<std::uint32_t>(clear_color.b * 255.0f) & 0xffU;
    const auto a = static_cast<std::uint32_t>(clear_color.a * 255.0f) & 0xffU;
    const auto rgba = (r << 24U) | (g << 16U) | (b << 8U) | a;

    ensure_scene_targets(*impl_);
    impl_->draw_view = 0;
    impl_->scene_capture_enabled = bgfx::isValid(impl_->scene_framebuffer)
        && bgfx::isValid(impl_->scene_texture)
        && bgfx::isValid(impl_->texture_program)
        && bgfx::isValid(impl_->source_sampler);

    if (impl_->scene_capture_enabled) {
        bgfx::setViewFrameBuffer(0, impl_->scene_framebuffer);
    } else {
        bgfx::setViewFrameBuffer(0, BGFX_INVALID_HANDLE);
    }
    bgfx::setViewFrameBuffer(1, BGFX_INVALID_HANDLE);
    bgfx::setViewFrameBuffer(2, BGFX_INVALID_HANDLE);
    bgfx::setViewRect(0, 0, 0, impl_->width, impl_->height);
    bgfx::setViewRect(1, 0, 0, impl_->width, impl_->height);
    bgfx::setViewRect(2, 0, 0, impl_->width, impl_->height);
    bgfx::setViewRect(3, 0, 0, impl_->width, impl_->height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, rgba, 1.0f, 0);
    bgfx::touch(0);
    if (impl_->scene_capture_enabled) {
        submit_textured_rect(*impl_, { 0.0f, 0.0f, static_cast<float>(impl_->width), static_cast<float>(impl_->height) }, {}, Color::rgba(255, 255, 255, 255), impl_->texture_program, 1);
    }
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
    if (matrix_is_identity(current_transform(*impl_))) {
        vertex_data[0] = { left, top, 0.0f, abgr };
        vertex_data[1] = { right, top, 0.0f, abgr };
        vertex_data[2] = { right, bottom, 0.0f, abgr };
        vertex_data[3] = { left, bottom, 0.0f, abgr };
    } else {
        vertex_data[0] = transformed_vertex_from_point(*impl_, { rect.x, rect.y }, abgr);
        vertex_data[1] = transformed_vertex_from_point(*impl_, { rect.x + rect.width, rect.y }, abgr);
        vertex_data[2] = transformed_vertex_from_point(*impl_, { rect.x + rect.width, rect.y + rect.height }, abgr);
        vertex_data[3] = transformed_vertex_from_point(*impl_, { rect.x, rect.y + rect.height }, abgr);
    }

    auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
    const std::array<std::uint16_t, 6> quad_indices { 0, 1, 2, 0, 2, 3 };
    std::copy(quad_indices.begin(), quad_indices.end(), index_data);

    bgfx::setVertexBuffer(0, &vertices);
    bgfx::setIndexBuffer(&indices);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(*impl_);
    bgfx::submit(impl_->draw_view, impl_->rect_program);
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
    vertex_data[0] = transformed_vertex_from_point(*impl_, { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f }, abgr);
    for (std::size_t index = 0; index < points.size(); ++index) {
        vertex_data[index + 1] = transformed_vertex_from_point(*impl_, points[index], abgr);
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
    bgfx::submit(impl_->draw_view, impl_->rect_program);
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

    if (top_left == 0.0f && top_right == 0.0f && bottom_right == 0.0f && bottom_left == 0.0f) {
        if (borders.top.width > 0.0f) {
            fill_rect({ rect.x, rect.y, rect.width, borders.top.width }, borders.top.color);
        }
        if (borders.right.width > 0.0f) {
            fill_rect({ rect.x + rect.width - borders.right.width, rect.y, borders.right.width, rect.height }, borders.right.color);
        }
        if (borders.bottom.width > 0.0f) {
            fill_rect({ rect.x, rect.y + rect.height - borders.bottom.width, rect.width, borders.bottom.width }, borders.bottom.color);
        }
        if (borders.left.width > 0.0f) {
            fill_rect({ rect.x, rect.y, borders.left.width, rect.height }, borders.left.color);
        }
        return;
    }

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
        vertex_data[index] = transformed_vertex_from_point(*impl_, points[index].outer, abgr);
        vertex_data[index + points.size()] = transformed_vertex_from_point(*impl_, points[index].inner, abgr);
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
    bgfx::submit(impl_->draw_view, impl_->rect_program);
#else
    (void)radius;
    if (borders.top.width > 0.0f) {
        stroke_rect({ rect.x, rect.y, rect.width, rect.height }, borders.top.color, borders.top.width);
    }
#endif
}

bool Renderer::load_font(std::string family, std::filesystem::path path)
{
    if (family.empty()) {
        return false;
    }

#if OUIF_WITH_BGFX
    auto data = read_binary_file(path);
    if (data.empty()) {
        return false;
    }

    FontFace face;
    face.family = std::move(family);
    face.data = std::move(data);
    const int offset = stbtt_GetFontOffsetForIndex(face.data.data(), 0);
    if (offset < 0 || stbtt_InitFont(&face.info, face.data.data(), offset) == 0) {
        return false;
    }
    face.valid = true;

    const auto key = font_key(face.family);
    if (auto existing = impl_->fonts.find(key); existing != impl_->fonts.end()) {
        destroy_font_atlases(existing->second);
    }
    impl_->fonts[key] = std::move(face);
    return true;
#else
    (void)path;
    impl_->default_font_family = std::move(family);
    return false;
#endif
}

ShaderProgram Renderer::load_shader_program(std::filesystem::path vertex_shader, std::filesystem::path fragment_shader)
{
#if OUIF_WITH_BGFX
    const auto vertex = load_shader(vertex_shader);
    const auto fragment = load_shader(fragment_shader);
    if (!bgfx::isValid(vertex) || !bgfx::isValid(fragment)) {
        if (bgfx::isValid(vertex)) {
            bgfx::destroy(vertex);
        }
        if (bgfx::isValid(fragment)) {
            bgfx::destroy(fragment);
        }
        return {};
    }

    const auto program = bgfx::createProgram(vertex, fragment, true);
    return bgfx::isValid(program) ? ShaderProgram { program.idx } : ShaderProgram {};
#else
    (void)vertex_shader;
    (void)fragment_shader;
    return {};
#endif
}

void Renderer::destroy_shader_program(ShaderProgram program) noexcept
{
#if OUIF_WITH_BGFX
    if (!program.valid()) {
        return;
    }
    bgfx::ProgramHandle handle { program.id };
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
#else
    (void)program;
#endif
}

void Renderer::fill_rect_with_program(Rect rect, Color color, ShaderProgram program)
{
#if OUIF_WITH_BGFX
    if (!program.valid() || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    bgfx::ProgramHandle handle { program.id };
    if (!bgfx::isValid(handle)) {
        return;
    }

    if (bgfx::getAvailTransientVertexBuffer(4, PosColorVertex::layout) < 4 || bgfx::getAvailTransientIndexBuffer(6) < 6) {
        return;
    }

    bgfx::TransientVertexBuffer vertices;
    bgfx::TransientIndexBuffer indices;
    bgfx::allocTransientVertexBuffer(&vertices, 4, PosColorVertex::layout);
    bgfx::allocTransientIndexBuffer(&indices, 6);

    const auto abgr = pack_abgr(color);
    auto* vertex_data = reinterpret_cast<PosColorVertex*>(vertices.data);
    vertex_data[0] = transformed_vertex_from_point(*impl_, { rect.x, rect.y }, abgr);
    vertex_data[1] = transformed_vertex_from_point(*impl_, { rect.x + rect.width, rect.y }, abgr);
    vertex_data[2] = transformed_vertex_from_point(*impl_, { rect.x + rect.width, rect.y + rect.height }, abgr);
    vertex_data[3] = transformed_vertex_from_point(*impl_, { rect.x, rect.y + rect.height }, abgr);

    auto* index_data = reinterpret_cast<std::uint16_t*>(indices.data);
    const std::array<std::uint16_t, 6> quad_indices { 0, 1, 2, 0, 2, 3 };
    std::copy(quad_indices.begin(), quad_indices.end(), index_data);

    bgfx::setVertexBuffer(0, &vertices);
    bgfx::setIndexBuffer(&indices);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
    apply_scissor(*impl_);
    bgfx::submit(impl_->draw_view, handle);
#else
    (void)rect;
    (void)color;
    (void)program;
#endif
}

void Renderer::draw_backdrop_blur(Rect rect, CornerRadius radius, float radius_px, Color tint)
{
#if OUIF_WITH_BGFX
    if (!bgfx::isValid(impl_->blur_program) || !bgfx::isValid(impl_->blur_uniform) || !bgfx::isValid(impl_->source_sampler)
        || !impl_->scene_capture_enabled || !bgfx::isValid(impl_->scene_texture) || rect.width <= 0.0f || rect.height <= 0.0f || radius_px <= 0.0f) {
        return;
    }

    const float clamped_radius = std::clamp(radius_px, 0.0f, 64.0f);
    const float uniform[4] {
        1.0f / static_cast<float>(std::max<std::uint32_t>(1U, impl_->width)),
        1.0f / static_cast<float>(std::max<std::uint32_t>(1U, impl_->height)),
        clamped_radius,
        0.0f,
    };
    bgfx::setUniform(impl_->blur_uniform, uniform);
    submit_textured_rect(*impl_, rect, radius, tint, impl_->blur_program, 2);
    impl_->draw_view = 3;
#else
    (void)rect;
    (void)radius;
    (void)radius_px;
    (void)tint;
#endif
}

ImageHandle Renderer::load_image(std::filesystem::path path)
{
#if OUIF_WITH_BGFX
    auto data = read_file(path);
    if (data.empty()) {
        return {};
    }
    return load_image(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
#else
    (void)path;
    return {};
#endif
}

ImageHandle Renderer::load_image(const std::uint8_t* data, std::size_t size)
{
#if OUIF_WITH_BGFX
    if (data == nullptr || size == 0U || !bgfx::isValid(impl_->texture_program) || !bgfx::isValid(impl_->source_sampler)) {
        return {};
    }

    bx::DefaultAllocator allocator;
    bimg::ImageContainer* parsed = bimg::imageParse(
        &allocator,
        data,
        static_cast<std::uint32_t>(std::min<std::size_t>(size, std::numeric_limits<std::uint32_t>::max())),
        bimg::TextureFormat::RGBA8
    );
    if (parsed == nullptr || parsed->m_width == 0U || parsed->m_height == 0U) {
        if (parsed != nullptr) {
            bimg::imageFree(parsed);
        }
        return {};
    }

    bimg::ImageMip mip {};
    if (!bimg::imageGetRawData(*parsed, 0, 0, parsed->m_data, parsed->m_size, mip) || mip.m_data == nullptr || mip.m_size == 0U) {
        bimg::imageFree(parsed);
        return {};
    }

    const auto width = static_cast<std::uint16_t>(std::min<std::uint32_t>(mip.m_width, std::numeric_limits<std::uint16_t>::max()));
    const auto height = static_cast<std::uint16_t>(std::min<std::uint32_t>(mip.m_height, std::numeric_limits<std::uint16_t>::max()));
    const bgfx::Memory* memory = bgfx::copy(mip.m_data, mip.m_size);
    auto texture = bgfx::createTexture2D(
        width,
        height,
        false,
        1,
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        memory
    );

    const Size natural_size { static_cast<float>(width), static_cast<float>(height) };
    bimg::imageFree(parsed);
    if (!bgfx::isValid(texture)) {
        return {};
    }

    std::uint16_t id = impl_->next_image_id;
    for (std::uint32_t attempts = 0; attempts < std::numeric_limits<std::uint16_t>::max(); ++attempts) {
        if (id == 0xffffU || id == 0U) {
            id = 1U;
        }
        if (!impl_->images.contains(id)) {
            impl_->next_image_id = static_cast<std::uint16_t>(id + 1U);
            impl_->images.emplace(id, RendererImage { texture, natural_size });
            return ImageHandle { id };
        }
        ++id;
    }

    bgfx::destroy(texture);
    return {};
#else
    (void)data;
    (void)size;
    return {};
#endif
}

void Renderer::destroy_image(ImageHandle image) noexcept
{
#if OUIF_WITH_BGFX
    if (!image.valid()) {
        return;
    }
    const auto found = impl_->images.find(image.id);
    if (found == impl_->images.end()) {
        return;
    }
    destroy_handle(found->second.texture);
    impl_->images.erase(found);
#else
    (void)image;
#endif
}

Size Renderer::image_size(ImageHandle image) const noexcept
{
#if OUIF_WITH_BGFX
    const auto found = impl_->images.find(image.id);
    return found != impl_->images.end() ? found->second.size : Size {};
#else
    (void)image;
    return {};
#endif
}

void Renderer::draw_image(ImageHandle image, Rect rect, ImageFit fit, ImageFilter filter, Color tint)
{
#if OUIF_WITH_BGFX
    const auto found = impl_->images.find(image.id);
    if (found == impl_->images.end() || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    const auto natural = found->second.size;
    if (natural.width <= 0.0f || natural.height <= 0.0f) {
        return;
    }

    Rect draw_rect = rect;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;

    const float image_aspect = natural.width / natural.height;
    const float rect_aspect = rect.width / rect.height;
    if (fit == ImageFit::Contain || fit == ImageFit::Center) {
        const float scale = fit == ImageFit::Center ? 1.0f : std::min(rect.width / natural.width, rect.height / natural.height);
        draw_rect.width = natural.width * scale;
        draw_rect.height = natural.height * scale;
        draw_rect.x = rect.x + (rect.width - draw_rect.width) * 0.5f;
        draw_rect.y = rect.y + (rect.height - draw_rect.height) * 0.5f;
    } else if (fit == ImageFit::Cover) {
        if (image_aspect > rect_aspect) {
            const float visible_u = rect_aspect / image_aspect;
            u0 = (1.0f - visible_u) * 0.5f;
            u1 = u0 + visible_u;
        } else if (image_aspect < rect_aspect) {
            const float visible_v = image_aspect / rect_aspect;
            v0 = (1.0f - visible_v) * 0.5f;
            v1 = v0 + visible_v;
        }
    }

    push_clip(rect);
    submit_image_rect(*impl_, found->second.texture, draw_rect, tint, u0, v0, u1, v1, filter);
    pop_clip();
#else
    (void)image;
    (void)rect;
    (void)fit;
    (void)filter;
    (void)tint;
#endif
}

VectorImageHandle Renderer::load_vector_image(std::filesystem::path path)
{
#if OUIF_WITH_BGFX && OUIF_WITH_PUGIXML
    auto data = read_file(path);
    if (data.empty()) {
        return {};
    }
    return load_vector_image(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
#else
    (void)path;
    return {};
#endif
}

VectorImageHandle Renderer::load_vector_image(std::string svg)
{
    return load_vector_image(reinterpret_cast<const std::uint8_t*>(svg.data()), svg.size());
}

VectorImageHandle Renderer::load_vector_image(const std::uint8_t* data, std::size_t size)
{
#if OUIF_WITH_BGFX && OUIF_WITH_PUGIXML
    if (data == nullptr || size == 0U) {
        return {};
    }

    auto image = parse_svg_document(std::string_view(reinterpret_cast<const char*>(data), size));
    if (image.shapes.empty() || image.size.width <= 0.0f || image.size.height <= 0.0f) {
        return {};
    }

    std::uint16_t id = impl_->next_vector_image_id;
    for (std::uint32_t attempts = 0; attempts < std::numeric_limits<std::uint16_t>::max(); ++attempts) {
        if (id == 0xffffU || id == 0U) {
            id = 1U;
        }
        if (!impl_->vector_images.contains(id)) {
            impl_->next_vector_image_id = static_cast<std::uint16_t>(id + 1U);
            impl_->vector_images.emplace(id, std::move(image));
            return VectorImageHandle { id };
        }
        ++id;
    }
    return {};
#else
    (void)data;
    (void)size;
    return {};
#endif
}

void Renderer::destroy_vector_image(VectorImageHandle image) noexcept
{
#if OUIF_WITH_BGFX
    if (!image.valid()) {
        return;
    }
    impl_->vector_images.erase(image.id);
#else
    (void)image;
#endif
}

Size Renderer::vector_image_size(VectorImageHandle image) const noexcept
{
#if OUIF_WITH_BGFX
    const auto found = impl_->vector_images.find(image.id);
    return found != impl_->vector_images.end() ? found->second.size : Size {};
#else
    (void)image;
    return {};
#endif
}

void Renderer::draw_vector_image(VectorImageHandle image, Rect rect, ImageFit fit, Color tint)
{
#if OUIF_WITH_BGFX && OUIF_WITH_VG_RENDERER
    const auto found = impl_->vector_images.find(image.id);
    if (found == impl_->vector_images.end() || impl_->vector_context == nullptr || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    const auto& vector_image = found->second;
    const auto natural = vector_image.size;
    Rect draw_rect = rect;
    if (natural.width > 0.0f && natural.height > 0.0f && fit != ImageFit::Stretch) {
        const float scale_x = rect.width / natural.width;
        const float scale_y = rect.height / natural.height;
        float scale = 1.0f;
        if (fit == ImageFit::Contain) {
            scale = std::min(scale_x, scale_y);
        } else if (fit == ImageFit::Cover) {
            scale = std::max(scale_x, scale_y);
        }
        if (fit == ImageFit::Center) {
            draw_rect.width = natural.width;
            draw_rect.height = natural.height;
        } else {
            draw_rect.width = natural.width * scale;
            draw_rect.height = natural.height * scale;
        }
        draw_rect.x = rect.x + (rect.width - draw_rect.width) * 0.5f;
        draw_rect.y = rect.y + (rect.height - draw_rect.height) * 0.5f;
    }

    push_clip(rect);
    vg::begin(impl_->vector_context, impl_->draw_view, static_cast<std::uint16_t>(impl_->width), static_cast<std::uint16_t>(impl_->height), 1.0f);
    vg::pushState(impl_->vector_context);
    if (!impl_->clip_stack.empty()) {
        const auto clip = impl_->clip_stack.back();
        vg::setScissor(impl_->vector_context, clip.x, clip.y, clip.width, clip.height);
    }
    const auto matrix = current_transform(*impl_);
    const float transform[6] { matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty };
    vg::transformMult(impl_->vector_context, transform, vg::TransformOrder::Post);
    vg::transformTranslate(impl_->vector_context, draw_rect.x, draw_rect.y);
    vg::transformScale(impl_->vector_context, draw_rect.width / vector_image.view_box.width, draw_rect.height / vector_image.view_box.height);
    vg::transformTranslate(impl_->vector_context, -vector_image.view_box.x, -vector_image.view_box.y);

    VectorCanvas canvas(impl_->vector_context);
    for (const auto& shape : vector_image.shapes) {
        draw_svg_shape(canvas, shape, tint);
    }

    vg::popState(impl_->vector_context);
    vg::end(impl_->vector_context);
    pop_clip();
#else
    (void)image;
    (void)rect;
    (void)fit;
    (void)tint;
#endif
}

void Renderer::draw_vector(Rect rect, const std::function<void(VectorCanvas&)>& draw_callback)
{
#if OUIF_WITH_BGFX && OUIF_WITH_VG_RENDERER
    if (impl_->vector_context == nullptr || !draw_callback || rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    vg::begin(impl_->vector_context, impl_->draw_view, static_cast<std::uint16_t>(impl_->width), static_cast<std::uint16_t>(impl_->height), 1.0f);
    vg::pushState(impl_->vector_context);
    if (!impl_->clip_stack.empty()) {
        const auto clip = impl_->clip_stack.back();
        vg::setScissor(impl_->vector_context, clip.x, clip.y, clip.width, clip.height);
    }
    const auto matrix = current_transform(*impl_);
    const float transform[6] { matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty };
    vg::transformMult(impl_->vector_context, transform, vg::TransformOrder::Post);
    vg::transformTranslate(impl_->vector_context, rect.x, rect.y);
    VectorCanvas canvas(impl_->vector_context);
    draw_callback(canvas);
    vg::popState(impl_->vector_context);
    vg::end(impl_->vector_context);
#else
    (void)rect;
    (void)draw_callback;
#endif
}

bool Renderer::load_default_system_font()
{
#if OUIF_WITH_BGFX
    for (const auto& path : default_font_candidates()) {
        if (std::filesystem::exists(path) && load_font("OUIF Sans", path)) {
            impl_->default_font_family = "OUIF Sans";
            return true;
        }
    }
#endif
    return false;
}

void Renderer::set_default_font_family(std::string family)
{
    if (!family.empty()) {
        impl_->default_font_family = std::move(family);
    }
}

std::string_view Renderer::default_font_family() const noexcept
{
    return impl_->default_font_family;
}

Size Renderer::measure_text(std::string_view text, const TextStyle& style) const noexcept
{
#if OUIF_WITH_BGFX
    if (const auto* face = find_font(*impl_, style.font_family); face != nullptr && face->valid) {
        const float scale = stbtt_ScaleForPixelHeight(&face->info, std::max(1.0f, style.font_size));
        float line_width = 0.0f;
        float max_width = 0.0f;
        std::uint32_t line_count = 1;
        for (const unsigned char ch : text) {
            if (ch == '\n') {
                max_width = std::max(max_width, line_width);
                line_width = 0.0f;
                ++line_count;
                continue;
            }
            if (ch < 32 || ch > 126) {
                line_width += glyph_advance(style);
                continue;
            }
            int advance_width = 0;
            int left_side_bearing = 0;
            stbtt_GetCodepointHMetrics(&face->info, ch, &advance_width, &left_side_bearing);
            line_width += static_cast<float>(advance_width) * scale + style.letter_spacing;
        }

        max_width = std::max(max_width, line_width);
        return {
            max_width,
            line_height_px(style) * static_cast<float>(line_count),
        };
    }
#endif

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

#if OUIF_WITH_BGFX
    if (bgfx::isValid(impl_->text_program) && bgfx::isValid(impl_->font_sampler)) {
        if (auto* face = find_font(*impl_, style.font_family); face != nullptr && face->valid) {
            if (auto* atlas = ensure_atlas(*impl_, *face, style.font_size); atlas != nullptr && bgfx::isValid(atlas->texture)) {
                float y = rect.y;
                const auto abgr = pack_abgr(style.color);

                for (const auto& line : lines) {
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

                    const float baseline = y + atlas->ascent;
                    std::vector<PosColorTexVertex> vertex_data;
                    std::vector<std::uint16_t> index_data;
                    vertex_data.reserve(line.size() * 4U);
                    index_data.reserve(line.size() * 6U);

                    float pen_x = x;
                    float pen_y = baseline;
                    for (const unsigned char ch : line) {
                        if (pen_x > rect.x + rect.width) {
                            break;
                        }
                        if (ch < 32 || ch > 126) {
                            pen_x += glyph_advance(style);
                            continue;
                        }

                        const float before = pen_x;
                        stbtt_aligned_quad quad {};
                        stbtt_GetPackedQuad(
                            atlas->chars.data(),
                            atlas->width,
                            atlas->height,
                            static_cast<int>(ch) - 32,
                            &pen_x,
                            &pen_y,
                            &quad,
                            1
                        );
                        pen_x += style.letter_spacing;

                        if (ch == ' ' || quad.x1 <= rect.x || quad.y1 <= rect.y || quad.x0 >= rect.x + rect.width || quad.y0 >= rect.y + rect.height) {
                            continue;
                        }

                        if (vertex_data.size() + 4U > std::numeric_limits<std::uint16_t>::max()) {
                            break;
                        }

                        const auto base = static_cast<std::uint16_t>(vertex_data.size());
                        vertex_data.push_back(transformed_text_vertex_from_point(*impl_, { quad.x0, quad.y0 }, abgr, quad.s0, quad.t0));
                        vertex_data.push_back(transformed_text_vertex_from_point(*impl_, { quad.x1, quad.y0 }, abgr, quad.s1, quad.t0));
                        vertex_data.push_back(transformed_text_vertex_from_point(*impl_, { quad.x1, quad.y1 }, abgr, quad.s1, quad.t1));
                        vertex_data.push_back(transformed_text_vertex_from_point(*impl_, { quad.x0, quad.y1 }, abgr, quad.s0, quad.t1));
                        index_data.insert(index_data.end(), {
                            base,
                            static_cast<std::uint16_t>(base + 1U),
                            static_cast<std::uint16_t>(base + 2U),
                            base,
                            static_cast<std::uint16_t>(base + 2U),
                            static_cast<std::uint16_t>(base + 3U),
                        });

                        if (pen_x <= before) {
                            pen_x = before + glyph_advance(style);
                        }
                    }

                    if (!vertex_data.empty()
                        && vertex_data.size() <= bgfx::getAvailTransientVertexBuffer(static_cast<std::uint32_t>(vertex_data.size()), PosColorTexVertex::layout)
                        && index_data.size() <= bgfx::getAvailTransientIndexBuffer(static_cast<std::uint32_t>(index_data.size()))) {
                        bgfx::TransientVertexBuffer vertices;
                        bgfx::TransientIndexBuffer indices;
                        bgfx::allocTransientVertexBuffer(&vertices, static_cast<std::uint32_t>(vertex_data.size()), PosColorTexVertex::layout);
                        bgfx::allocTransientIndexBuffer(&indices, static_cast<std::uint32_t>(index_data.size()));
                        std::copy(vertex_data.begin(), vertex_data.end(), reinterpret_cast<PosColorTexVertex*>(vertices.data));
                        std::copy(index_data.begin(), index_data.end(), reinterpret_cast<std::uint16_t*>(indices.data));

                        bgfx::setVertexBuffer(0, &vertices);
                        bgfx::setIndexBuffer(&indices);
                        bgfx::setTexture(0, impl_->font_sampler, atlas->texture);
                        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
                        apply_scissor(*impl_);
                        bgfx::submit(impl_->draw_view, impl_->text_program);
                    }

                    y += line_height;
                }

                pop_clip();
                return;
            }
        }
    }
#endif

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

void Renderer::push_transform(Rect bounds, Transform transform)
{
#if OUIF_WITH_BGFX
    const Mat3 local = matrix_from_transform(bounds, transform);
    const Mat3 parent = current_transform(*impl_);
    impl_->transform_stack.push_back(multiply(parent, local));
#else
    (void)bounds;
    (void)transform;
#endif
}

void Renderer::pop_transform()
{
#if OUIF_WITH_BGFX
    if (!impl_->transform_stack.empty()) {
        impl_->transform_stack.pop_back();
    }
#endif
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
