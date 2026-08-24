#pragma once

#include <OUIF/Renderer.h>
#include <OUIF/Widget.h>

#include <string>
#include <string_view>

namespace ouif {

enum class Orientation : std::uint8_t {
    Horizontal,
    Vertical,
};

class OUIF_API Spacer : public Widget {
public:
    Spacer() noexcept;
    explicit Spacer(float flex) noexcept;
    explicit Spacer(Size size) noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;
};

class OUIF_API Divider : public Widget {
public:
    explicit Divider(Orientation orientation = Orientation::Horizontal, float thickness = 1.0f);

    void set_orientation(Orientation orientation) noexcept;
    [[nodiscard]] Orientation orientation() const noexcept;
    void set_thickness(float thickness) noexcept;
    [[nodiscard]] float thickness() const noexcept;
    void set_color(Color color) noexcept;
    [[nodiscard]] Color color() const noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;

private:
    void apply_axis_size() noexcept;

    Orientation orientation_ = Orientation::Horizontal;
    float thickness_ = 1.0f;
};

class OUIF_API Label : public Widget {
public:
    Label();
    explicit Label(std::string text);

    void set_text(std::string text);
    [[nodiscard]] std::string_view text() const noexcept;
    [[nodiscard]] std::string_view get_text() const noexcept;

    void set_text_style(TextStyle style) noexcept;
    void set_text_style(InheritTag) noexcept;
    [[nodiscard]] const TextStyle& text_style() const noexcept;
    [[nodiscard]] const TextStyle& get_text_style() const noexcept;
    void set_font_family(std::string family);
    void set_font_family(InheritTag);
    [[nodiscard]] std::string_view font_family() const noexcept;
    void set_font_size(float size) noexcept;
    void set_font_size(InheritTag) noexcept;
    [[nodiscard]] float font_size() const noexcept;
    void set_text_color(Color color) noexcept;
    void set_text_color(InheritTag) noexcept;
    [[nodiscard]] Color text_color() const noexcept;
    void set_text_align(TextAlign align) noexcept;
    void set_text_align(InheritTag) noexcept;
    [[nodiscard]] TextAlign text_align() const noexcept;
    void set_text_overflow(TextOverflow overflow) noexcept;
    void set_text_overflow(InheritTag) noexcept;
    [[nodiscard]] TextOverflow text_overflow() const noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;

private:
    std::string text_;
    TextStyle text_style_ {};
    bool has_text_color_ = false;
};

class OUIF_API Overlay : public Widget {
public:
    Overlay() noexcept;

protected:
    void on_layout(Rect content) override;
};

} // namespace ouif
