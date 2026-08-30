#pragma once

#include <OUIF/Renderer.h>
#include <OUIF/Widget.h>

#include <filesystem>
#include <optional>
#include <utility>
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
    void set_text_color(Gradient gradient);
    void set_text_color(InheritTag) noexcept;
    [[nodiscard]] Color text_color() const noexcept;
    [[nodiscard]] const std::optional<Gradient>& text_gradient() const noexcept;
    [[nodiscard]] const std::optional<Gradient>& get_text_gradient() const noexcept;
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

class OUIF_API Input : public Widget {
public:
    Input();
    explicit Input(std::string text);

    void set_text(std::string text);
    [[nodiscard]] std::string_view text() const noexcept;
    [[nodiscard]] std::string_view get_text() const noexcept;
    void clear_text() noexcept;

    void set_placeholder(std::string placeholder);
    [[nodiscard]] std::string_view placeholder() const noexcept;
    [[nodiscard]] std::string_view get_placeholder() const noexcept;
    void set_composition_text(std::string text);
    [[nodiscard]] std::string_view composition_text() const noexcept;
    [[nodiscard]] std::string_view get_composition_text() const noexcept;
    void clear_composition() noexcept;

    void set_text_style(TextStyle style) noexcept;
    void set_text_style(InheritTag) noexcept;
    [[nodiscard]] const TextStyle& text_style() const noexcept;
    [[nodiscard]] const TextStyle& get_text_style() const noexcept;
    void set_font_family(std::string family);
    [[nodiscard]] std::string_view font_family() const noexcept;
    void set_font_size(float size) noexcept;
    [[nodiscard]] float font_size() const noexcept;
    void set_text_color(Color color) noexcept;
    void set_text_color(Gradient gradient);
    void set_placeholder_color(Color color) noexcept;
    [[nodiscard]] Color text_color() const noexcept;
    [[nodiscard]] const std::optional<Gradient>& text_gradient() const noexcept;
    [[nodiscard]] const std::optional<Gradient>& get_text_gradient() const noexcept;
    [[nodiscard]] Color placeholder_color() const noexcept;

    void set_caret(std::size_t index) noexcept;
    [[nodiscard]] std::size_t caret() const noexcept;
    [[nodiscard]] std::size_t get_caret() const noexcept;
    void select(std::size_t anchor, std::size_t caret) noexcept;
    void select_all() noexcept;
    void clear_selection() noexcept;
    [[nodiscard]] bool has_selection() const noexcept;
    [[nodiscard]] std::pair<std::size_t, std::size_t> selection() const noexcept;
    void insert_text(std::string_view text);
    void erase_selection();
    void erase_previous();
    void erase_next();
    void copy_selection();
    void cut_selection();
    void paste_text(std::string_view text);
    void paste_clipboard();
    static void set_clipboard_text(std::string text);
    [[nodiscard]] static std::string_view clipboard_text() noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;
    bool on_event(const Event& event) override;
    bool on_key_down(const KeyEvent& event) override;
    bool on_mouse_down(const MouseEvent& event) override;

private:
    [[nodiscard]] std::size_t caret_from_point(Point point) const noexcept;
    [[nodiscard]] std::pair<std::size_t, std::size_t> ordered_selection() const noexcept;

    std::string text_;
    std::string placeholder_;
    std::string composition_text_;
    TextStyle text_style_ {};
    Color placeholder_color_ = Color::rgba(160, 172, 190, 255);
    std::size_t caret_ = 0;
    std::size_t selection_anchor_ = 0;
};

class OUIF_API Image : public Widget {
public:
    Image();
    explicit Image(std::filesystem::path source);

    void set_source(std::filesystem::path source);
    void set_source(std::string source);
    [[nodiscard]] const std::filesystem::path& source() const noexcept;
    [[nodiscard]] const std::filesystem::path& get_source() const noexcept;

    void set_resource(int id) noexcept;
    void clear_resource() noexcept;
    [[nodiscard]] std::optional<int> resource() const noexcept;
    [[nodiscard]] std::optional<int> get_resource() const noexcept;

    void set_fit(ImageFit fit) noexcept;
    void set_fit(InheritTag) noexcept;
    [[nodiscard]] ImageFit fit() const noexcept;
    [[nodiscard]] ImageFit get_fit() const noexcept;

    void set_filter(ImageFilter filter) noexcept;
    void set_filter(InheritTag) noexcept;
    [[nodiscard]] ImageFilter filter() const noexcept;
    [[nodiscard]] ImageFilter get_filter() const noexcept;

    void set_tint(Color tint) noexcept;
    void set_tint(InheritTag) noexcept;
    [[nodiscard]] Color tint() const noexcept;
    [[nodiscard]] Color get_tint() const noexcept;

    [[nodiscard]] bool loaded() const noexcept;
    [[nodiscard]] Size natural_size() const noexcept;
    void reload(Renderer& renderer);
    void unload(Renderer& renderer) noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;

private:
    void reset_loaded_state() noexcept;

    std::filesystem::path source_;
    std::optional<int> resource_id_;
    ImageFit fit_ = ImageFit::Contain;
    ImageFilter filter_ = ImageFilter::Linear;
    Color tint_ = Color::rgba(255, 255, 255, 255);
    ImageHandle image_ {};
    Size natural_size_ {};
    bool image_dirty_ = true;
};

class OUIF_API VectorImage : public Widget {
public:
    VectorImage();
    explicit VectorImage(std::filesystem::path source);

    void set_source(std::filesystem::path source);
    void set_source(std::string source);
    [[nodiscard]] const std::filesystem::path& source() const noexcept;
    [[nodiscard]] const std::filesystem::path& get_source() const noexcept;

    void set_svg(std::string svg);
    [[nodiscard]] std::string_view svg() const noexcept;
    [[nodiscard]] std::string_view get_svg() const noexcept;

    void set_resource(int id) noexcept;
    void clear_resource() noexcept;
    [[nodiscard]] std::optional<int> resource() const noexcept;
    [[nodiscard]] std::optional<int> get_resource() const noexcept;

    void set_fit(ImageFit fit) noexcept;
    void set_fit(InheritTag) noexcept;
    [[nodiscard]] ImageFit fit() const noexcept;
    [[nodiscard]] ImageFit get_fit() const noexcept;

    void set_tint(Color tint) noexcept;
    void set_tint(InheritTag) noexcept;
    [[nodiscard]] Color tint() const noexcept;
    [[nodiscard]] Color get_tint() const noexcept;

    [[nodiscard]] bool loaded() const noexcept;
    [[nodiscard]] Size natural_size() const noexcept;
    void reload(Renderer& renderer);
    void unload(Renderer& renderer) noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;

private:
    void reset_loaded_state() noexcept;

    std::filesystem::path source_;
    std::string inline_svg_;
    std::optional<int> resource_id_;
    ImageFit fit_ = ImageFit::Contain;
    Color tint_ = Color::rgba(255, 255, 255, 255);
    VectorImageHandle image_ {};
    Size natural_size_ {};
    bool image_dirty_ = true;
};

class OUIF_API Overlay : public Widget {
public:
    Overlay() noexcept;

protected:
    void on_layout(Rect content) override;
};

} // namespace ouif
