#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace ouif {

enum class RendererQuality {
    Low,
    Balanced,
    High,
    Ultra,
};

struct RendererQualityConfig {
    RendererQuality preset = RendererQuality::High;
    std::uint16_t curve_segments = 12;
    std::uint16_t border_curve_segments = 24;
    std::uint8_t msaa_samples = 4;
    bool hardware_acceleration = true;
    bool smoothing = true;
};

struct RendererConfig {
    void* native_window = nullptr;
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    const char* shader_directory = nullptr;
    bool vsync = true;
    RendererQualityConfig quality {};
};

struct ShaderProgram {
    std::uint16_t id = 0xffffU;

    [[nodiscard]] constexpr bool valid() const noexcept { return id != 0xffffU; }
};

struct ImageHandle {
    std::uint16_t id = 0xffffU;

    [[nodiscard]] constexpr bool valid() const noexcept { return id != 0xffffU; }
};

struct VectorImageHandle {
    std::uint16_t id = 0xffffU;

    [[nodiscard]] constexpr bool valid() const noexcept { return id != 0xffffU; }
};

enum class ImageFit : std::uint8_t {
    Stretch,
    Contain,
    Cover,
    Center,
};

enum class ImageFilter : std::uint8_t {
    Linear,
    Nearest,
};

enum class BlurType : std::uint8_t {
    Gaussian = 0,
    DualKawase = 1,
};

enum class VectorLineCap : std::uint8_t {
    Butt,
    Round,
    Square,
};

enum class VectorLineJoin : std::uint8_t {
    Miter,
    Round,
    Bevel,
};

enum class VectorFillRule : std::uint8_t {
    NonZero,
    EvenOdd,
};

class OUIF_API VectorCanvas {
public:
    explicit VectorCanvas(void* native_context = nullptr) noexcept;

    void begin_path();
    void move_to(float x, float y);
    void line_to(float x, float y);
    void cubic_to(float c1x, float c1y, float c2x, float c2y, float x, float y);
    void quadratic_to(float cx, float cy, float x, float y);
    void arc(float cx, float cy, float radius, float start_radians, float end_radians, bool clockwise = true);
    void rect(float x, float y, float width, float height);
    void rounded_rect(float x, float y, float width, float height, float radius);
    void rounded_rect(float x, float y, float width, float height, CornerRadius radius);
    void circle(float cx, float cy, float radius);
    void ellipse(float cx, float cy, float rx, float ry);
    void polyline(const float* points, std::uint32_t point_count);
    void close_path();
    void fill(Color color, VectorFillRule rule = VectorFillRule::NonZero, bool anti_alias = true);
    void fill_linear_gradient(float start_x, float start_y, float end_x, float end_y, Color inner, Color outer, VectorFillRule rule = VectorFillRule::NonZero, bool anti_alias = true);
    void fill_radial_gradient(float center_x, float center_y, float inner_radius, float outer_radius, Color inner, Color outer, VectorFillRule rule = VectorFillRule::NonZero, bool anti_alias = true);
    void stroke(
        Color color,
        float width = 1.0f,
        VectorLineCap cap = VectorLineCap::Butt,
        VectorLineJoin join = VectorLineJoin::Miter,
        bool anti_alias = true
    );
    void stroke_linear_gradient(float start_x, float start_y, float end_x, float end_y, Color inner, Color outer, float width = 1.0f, bool anti_alias = true);
    void stroke_radial_gradient(float center_x, float center_y, float inner_radius, float outer_radius, Color inner, Color outer, float width = 1.0f, bool anti_alias = true);

    void push_state();
    void pop_state();
    void set_alpha(float alpha);
    void translate(float x, float y);
    void scale(float x, float y);
    void rotate(float radians);
    void transform(const float matrix[6]);
    void begin_clip(VectorFillRule rule = VectorFillRule::NonZero);
    void end_clip();
    void scissor(Rect rect);
    void reset_scissor();

    [[nodiscard]] void* native_handle() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    void* native_context_ = nullptr;
};

enum class TextAlign : std::uint8_t {
    Start,
    Center,
    End,
};

enum class TextOverflow : std::uint8_t {
    Clip,
    Wrap,
};

struct TextStyle {
    std::string font_family = "OUIF Sans";
    float font_size = 16.0f;
    float line_height = 1.25f;
    float letter_spacing = 0.0f;
    Color color = Color::rgba(242, 244, 248, 255);
    std::optional<Gradient> color_gradient {};
    TextAlign align = TextAlign::Start;
    TextOverflow overflow = TextOverflow::Clip;

    TextStyle& with_font_family(std::string value)
    {
        font_family = std::move(value);
        return *this;
    }

    TextStyle& with_font_size(float value) noexcept
    {
        font_size = value;
        return *this;
    }

    TextStyle& with_line_height(float value) noexcept
    {
        line_height = value;
        return *this;
    }

    TextStyle& with_letter_spacing(float value) noexcept
    {
        letter_spacing = value;
        return *this;
    }

    TextStyle& with_color(Color value) noexcept
    {
        color = value;
        color_gradient.reset();
        return *this;
    }

    TextStyle& with_color(Gradient value)
    {
        color_gradient = std::move(value);
        return *this;
    }

    TextStyle& with_align(TextAlign value) noexcept
    {
        align = value;
        return *this;
    }

    TextStyle& with_overflow(TextOverflow value) noexcept
    {
        overflow = value;
        return *this;
    }
};

class OUIF_API Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept;
    Renderer& operator=(Renderer&&) noexcept;

    void initialize(const RendererConfig& config);
    void shutdown() noexcept;
    void resize(std::uint32_t width, std::uint32_t height);
    void begin_frame(Color clear_color);
    void fill_rect(Rect rect, Color color);
    void fill_rect(Rect rect, const Gradient& gradient);
    void fill_rounded_rect(Rect rect, CornerRadius radius, Color color);
    void fill_rounded_rect(Rect rect, CornerRadius radius, const Gradient& gradient);
    void stroke_rect(Rect rect, Color color, float width);
    void stroke_rounded_rect(Rect rect, CornerRadius radius, Color color, float width);
    void stroke_rounded_rect(Rect rect, CornerRadius radius, BorderEdges borders);
    bool load_font(std::string family, std::filesystem::path path);
    [[nodiscard]] ShaderProgram load_shader_program(std::filesystem::path vertex_shader, std::filesystem::path fragment_shader);
    void destroy_shader_program(ShaderProgram program) noexcept;
    void fill_rect_with_program(Rect rect, Color color, ShaderProgram program);
    void draw_backdrop_blur(Rect rect, CornerRadius radius, float radius_px, Color tint, BlurType type = BlurType::Gaussian);
    void begin_layer_capture(Rect bounds);
    void end_layer_blur(Rect bounds, CornerRadius radius, float radius_px, Color tint, BlurType type = BlurType::Gaussian);
    [[nodiscard]] ImageHandle load_image(std::filesystem::path path);
    [[nodiscard]] ImageHandle load_image(const std::uint8_t* data, std::size_t size);
    void destroy_image(ImageHandle image) noexcept;
    [[nodiscard]] Size image_size(ImageHandle image) const noexcept;
    void draw_image(ImageHandle image, Rect rect, ImageFit fit = ImageFit::Contain, ImageFilter filter = ImageFilter::Linear, Color tint = Color::rgba(255, 255, 255, 255));
    [[nodiscard]] VectorImageHandle load_vector_image(std::filesystem::path path);
    [[nodiscard]] VectorImageHandle load_vector_image(std::string svg);
    [[nodiscard]] VectorImageHandle load_vector_image(const std::uint8_t* data, std::size_t size);
    void destroy_vector_image(VectorImageHandle image) noexcept;
    [[nodiscard]] Size vector_image_size(VectorImageHandle image) const noexcept;
    void draw_vector_image(VectorImageHandle image, Rect rect, ImageFit fit = ImageFit::Contain, Color tint = Color::rgba(255, 255, 255, 255));
    void draw_vector(Rect rect, const std::function<void(VectorCanvas&)>& draw_callback);
    bool load_default_system_font();
    void set_default_font_family(std::string family);
    [[nodiscard]] std::string_view default_font_family() const noexcept;
    [[nodiscard]] Size measure_text(std::string_view text, const TextStyle& style) const noexcept;
    void draw_text(std::string_view text, Rect rect, const TextStyle& style);
    void draw_text(std::string_view text, Rect rect, const TextStyle& style, const Gradient& gradient);
    void push_transform(Rect bounds, Transform transform);
    void pop_transform();
    void push_clip(Rect rect);
    void pop_clip();
    void end_frame();

    [[nodiscard]] bool initialized() const noexcept;
    [[nodiscard]] Size size() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ouif
