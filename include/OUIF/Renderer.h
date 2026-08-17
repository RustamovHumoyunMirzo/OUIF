#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <cstdint>
#include <memory>
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
    void fill_rounded_rect(Rect rect, CornerRadius radius, Color color);
    void stroke_rect(Rect rect, Color color, float width);
    void stroke_rounded_rect(Rect rect, CornerRadius radius, Color color, float width);
    void stroke_rounded_rect(Rect rect, CornerRadius radius, BorderEdges borders);
    [[nodiscard]] Size measure_text(std::string_view text, const TextStyle& style) const noexcept;
    void draw_text(std::string_view text, Rect rect, const TextStyle& style);
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
