#pragma once

#include <OUIF/Event.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <memory>
#include <optional>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ouif {

class Renderer;

enum class SizePolicy : std::uint8_t {
    Fixed,
    Fill,
    Content,
};

enum class WidgetState : std::uint8_t {
    Selected,
    Focused,
};

enum class AccessibilityRole : std::uint8_t {
    None,
    Widget,
    Button,
    Checkbox,
    Radio,
    Slider,
    TextInput,
    Label,
    Container,
};

struct AccessibilityInfo {
    AccessibilityRole role = AccessibilityRole::Widget;
    std::string label;
    std::string description;
};

struct Layout {
    Size preferred_size { 0.0f, 0.0f };
    Size min_size { 0.0f, 0.0f };
    Size max_size { 100000.0f, 100000.0f };
    Insets margin {};
    Insets padding {};
    Length width_value {};
    Length height_value {};
    float flex = 0.0f;
    SizePolicy width = SizePolicy::Fill;
    SizePolicy height = SizePolicy::Fill;
};

class OUIF_API Widget {
public:
    Widget() = default;
    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) noexcept = delete;
    Widget& operator=(Widget&&) noexcept = delete;

    void set_bounds(Rect bounds) noexcept;
    [[nodiscard]] Rect bounds() const noexcept;
    void set_size(Size size) noexcept;
    void set_width(Length width) noexcept;
    void set_height(Length height) noexcept;
    void set_size(Length width, Length height) noexcept;

    void set_style(Style style) noexcept;
    [[nodiscard]] const Style& style() const noexcept;
    [[nodiscard]] const Style& get_style() const noexcept;
    void set_background(Color color) noexcept;
    [[nodiscard]] Color get_background() const noexcept;
    void set_background_hovered(Color color) noexcept;
    [[nodiscard]] Color get_background_hovered() const noexcept;
    void set_background_pressed(Color color) noexcept;
    [[nodiscard]] Color get_background_pressed() const noexcept;
    void set_background_selected(Color color) noexcept;
    [[nodiscard]] Color get_background_selected() const noexcept;
    void set_background_focused(Color color) noexcept;
    [[nodiscard]] Color get_background_focused() const noexcept;
    void set_foreground(Color color) noexcept;
    [[nodiscard]] Color get_foreground() const noexcept;
    void set_border(Color color, float width) noexcept;
    [[nodiscard]] Border get_border() const noexcept;
    void set_border_left(Color color, float width) noexcept;
    [[nodiscard]] Border get_border_left() const noexcept;
    void set_border_top(Color color, float width) noexcept;
    [[nodiscard]] Border get_border_top() const noexcept;
    void set_border_right(Color color, float width) noexcept;
    [[nodiscard]] Border get_border_right() const noexcept;
    void set_border_bottom(Color color, float width) noexcept;
    [[nodiscard]] Border get_border_bottom() const noexcept;
    void set_border_selected(Color color, float width) noexcept;
    [[nodiscard]] Border get_border_selected() const noexcept;
    void set_border_focused(Color color, float width) noexcept;
    [[nodiscard]] Border get_border_focused() const noexcept;
    void set_radius(float radius) noexcept;
    void set_radius(CornerRadius radius) noexcept;
    [[nodiscard]] CornerRadius get_radius() const noexcept;
    void set_opacity(float opacity) noexcept;
    [[nodiscard]] float get_opacity() const noexcept;

    void set_stylesheet(std::string stylesheet);
    void join_stylesheet(std::string_view stylesheet);
    [[nodiscard]] std::string_view get_stylesheet() const noexcept;

    void set_name(std::string name);
    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::string_view get_name() const noexcept;
    void set_type_name(std::string type_name);
    [[nodiscard]] std::string_view type_name() const noexcept;
    Widget& add_class(std::string class_name);
    bool remove_class(std::string_view class_name);
    [[nodiscard]] bool has_class(std::string_view class_name) const noexcept;
    [[nodiscard]] const std::vector<std::string>& classes() const noexcept;

    void set_layout(Layout layout) noexcept;
    [[nodiscard]] const Layout& layout_rules() const noexcept;
    void set_layout_policy(SizePolicy width, SizePolicy height) noexcept;
    void set_margin(Insets margin) noexcept;
    [[nodiscard]] Insets get_margin() const noexcept;
    void set_padding(Insets padding) noexcept;
    [[nodiscard]] Insets get_padding() const noexcept;
    void set_flex(float flex) noexcept;
    [[nodiscard]] float get_flex() const noexcept;

    void set_visible(bool visible) noexcept;
    [[nodiscard]] bool visible() const noexcept;

    void set_enabled(bool enabled) noexcept;
    [[nodiscard]] bool enabled() const noexcept;

    void set_focusable(bool focusable) noexcept;
    [[nodiscard]] bool focusable() const noexcept;
    [[nodiscard]] bool can_focus() const noexcept;
    void set_keyboard_activation_enabled(bool enabled) noexcept;
    [[nodiscard]] bool keyboard_activation_enabled() const noexcept;

    void set_accessibility_role(AccessibilityRole role) noexcept;
    [[nodiscard]] AccessibilityRole accessibility_role() const noexcept;
    void set_accessibility_label(std::string label);
    [[nodiscard]] std::string_view accessibility_label() const noexcept;
    void set_accessibility_description(std::string description);
    [[nodiscard]] std::string_view accessibility_description() const noexcept;
    void set_accessibility(AccessibilityInfo info);
    [[nodiscard]] const AccessibilityInfo& accessibility() const noexcept;

    bool focus_next(bool reverse = false) noexcept;

    Widget& add_child(Widget& child);
    Widget& add_child(std::unique_ptr<Widget> child);
    bool remove_child(Widget& child) noexcept;
    void clear_children() noexcept;

    template <typename... Widgets>
        requires(sizeof...(Widgets) > 0)
    Widget& children(Widgets&... widgets)
    {
        (add_child(widgets), ...);
        return *this;
    }

    template <typename T, typename... Args>
    T& add_child(Args&&... args)
    {
        static_assert(std::is_base_of_v<Widget, T>, "add_child<T> requires T to inherit ouif::Widget");
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        return static_cast<T&>(add_child(std::move(child)));
    }

    [[nodiscard]] const std::vector<Widget*>& children() const noexcept;
    [[nodiscard]] Widget* parent() noexcept;
    [[nodiscard]] const Widget* parent() const noexcept;

    void set_state(WidgetState state, bool enabled) noexcept;
    void toggle_state(WidgetState state) noexcept;
    [[nodiscard]] bool has_state(WidgetState state) const noexcept;

    [[nodiscard]] bool hovered() const noexcept;
    [[nodiscard]] bool pressed() const noexcept;
    [[nodiscard]] bool focused() const noexcept;
    void focus() noexcept;
    void blur() noexcept;
    [[nodiscard]] bool hit_test(Point point) const noexcept;

    virtual void layout(Size available);
    virtual void render(Renderer& renderer);
    virtual bool event(const Event& event);

protected:
    [[nodiscard]] std::vector<Widget*>& mutable_children() noexcept;
    virtual void draw(Renderer& renderer);
    virtual void on_layout(Rect content);
    virtual bool on_event(const Event& event);
    virtual void on_mouse_enter(const MouseEvent& event);
    virtual void on_mouse_leave(const MouseEvent& event);
    virtual void on_focus();
    virtual void on_blur();
    virtual bool on_mouse_move(const MouseEvent& event);
    virtual bool on_mouse_down(const MouseEvent& event);
    virtual bool on_mouse_up(const MouseEvent& event);
    virtual bool on_click(const MouseEvent& event);
    virtual bool on_key_down(const KeyEvent& event);
    virtual bool on_key_up(const KeyEvent& event);
    virtual bool on_keyboard_activate(const KeyEvent& event);

private:
    void detach_from_parent() noexcept;
    bool detach_child(Widget& child, bool destroy_owned) noexcept;
    [[nodiscard]] bool owns_child(const Widget& child) const noexcept;
    void recompute_style() noexcept;
    void apply_stylesheet_to_tree();
    bool handle_key_event(const KeyEvent& event);
    bool handle_focused_key_event(const KeyEvent& event);
    void collect_focusable_widgets(std::vector<Widget*>& widgets) noexcept;
    [[nodiscard]] bool contains_widget(const Widget& widget) const noexcept;

    [[nodiscard]] Point to_local(Point point) const noexcept;
    [[nodiscard]] std::optional<MouseEvent> mouse_event_from(const Event& event) const noexcept;

    Widget* parent_ = nullptr;
    Rect bounds_;
    Style style_;
    Style inline_style_;
    Style stylesheet_style_;
    Layout layout_;
    std::string stylesheet_;
    std::string name_;
    std::string type_name_ = "Widget";
    std::vector<std::string> classes_;
    AccessibilityInfo accessibility_ {};
    std::vector<std::unique_ptr<Widget>> owned_children_;
    std::vector<Widget*> children_;
    bool has_inline_style_ = false;
    bool has_stylesheet_style_ = false;
    bool visible_ = true;
    bool enabled_ = true;
    bool focusable_ = false;
    bool keyboard_activation_enabled_ = false;
    bool hovered_ = false;
    bool pressed_ = false;
    bool selected_ = false;
    bool focused_ = false;

    static Widget* focused_widget_;
};

} // namespace ouif
