#pragma once

#include <OUIF/Animation.h>
#include <OUIF/Effect.h>
#include <OUIF/Event.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <functional>
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
class Widget;

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

struct CssValue {
    std::string text;
    std::optional<float> number;
    std::optional<Color> color;
    Length length {};
    bool inherit = false;
};

struct CssDeclaration {
    std::string property;
    std::vector<CssValue> values;
    std::string raw;

    [[nodiscard]] bool inherited() const noexcept
    {
        return !values.empty() && values.front().inherit;
    }
};

using CssPropertyHandler = std::function<bool(Widget&, const CssDeclaration&)>;

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
    void set_size(InheritTag) noexcept;
    void set_width(Length width) noexcept;
    void set_width(InheritTag) noexcept;
    void set_height(Length height) noexcept;
    void set_height(InheritTag) noexcept;
    void set_size(Length width, Length height) noexcept;

    void set_style(Style style) noexcept;
    void set_style(InheritTag) noexcept;
    [[nodiscard]] const Style& style() const noexcept;
    [[nodiscard]] const Style& get_style() const noexcept;
    void set_background(Color color) noexcept;
    void set_background(InheritTag) noexcept;
    [[nodiscard]] Color get_background() const noexcept;
    void set_background_hovered(Color color) noexcept;
    void set_background_hovered(InheritTag) noexcept;
    [[nodiscard]] Color get_background_hovered() const noexcept;
    void set_background_pressed(Color color) noexcept;
    void set_background_pressed(InheritTag) noexcept;
    [[nodiscard]] Color get_background_pressed() const noexcept;
    void set_background_selected(Color color) noexcept;
    void set_background_selected(InheritTag) noexcept;
    [[nodiscard]] Color get_background_selected() const noexcept;
    void set_background_focused(Color color) noexcept;
    void set_background_focused(InheritTag) noexcept;
    [[nodiscard]] Color get_background_focused() const noexcept;
    void set_foreground(Color color) noexcept;
    void set_foreground(InheritTag) noexcept;
    [[nodiscard]] Color get_foreground() const noexcept;
    void set_border(Color color, float width) noexcept;
    void set_border(InheritTag) noexcept;
    [[nodiscard]] Border get_border() const noexcept;
    void set_border_left(Color color, float width) noexcept;
    void set_border_left(InheritTag) noexcept;
    [[nodiscard]] Border get_border_left() const noexcept;
    void set_border_top(Color color, float width) noexcept;
    void set_border_top(InheritTag) noexcept;
    [[nodiscard]] Border get_border_top() const noexcept;
    void set_border_right(Color color, float width) noexcept;
    void set_border_right(InheritTag) noexcept;
    [[nodiscard]] Border get_border_right() const noexcept;
    void set_border_bottom(Color color, float width) noexcept;
    void set_border_bottom(InheritTag) noexcept;
    [[nodiscard]] Border get_border_bottom() const noexcept;
    void set_border_selected(Color color, float width) noexcept;
    void set_border_selected(InheritTag) noexcept;
    [[nodiscard]] Border get_border_selected() const noexcept;
    void set_border_focused(Color color, float width) noexcept;
    void set_border_focused(InheritTag) noexcept;
    [[nodiscard]] Border get_border_focused() const noexcept;
    void set_radius(float radius) noexcept;
    void set_radius(CornerRadius radius) noexcept;
    void set_radius(InheritTag) noexcept;
    [[nodiscard]] CornerRadius get_radius() const noexcept;
    void set_opacity(float opacity) noexcept;
    void set_opacity(InheritTag) noexcept;
    [[nodiscard]] float get_opacity() const noexcept;

    void set_transition(StyleTransition transition) noexcept;
    void set_transition(float duration, Easing easing = Easing::EaseOut) noexcept;
    void clear_transition() noexcept;
    [[nodiscard]] const StyleTransition& transition() const noexcept;
    [[nodiscard]] const StyleTransition& get_transition() const noexcept;

    void set_animation(StyleAnimation animation);
    void clear_animation() noexcept;
    [[nodiscard]] const std::optional<StyleAnimation>& animation() const noexcept;
    [[nodiscard]] const std::optional<StyleAnimation>& get_animation() const noexcept;
    [[nodiscard]] bool animation_running() const noexcept;

    void set_stylesheet(std::string stylesheet);
    void join_stylesheet(std::string_view stylesheet);
    [[nodiscard]] std::string_view get_stylesheet() const noexcept;

    void add_layer_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {});
    void add_layer_effect(std::string name, std::vector<float> numbers = {});
    void clear_layer_effects() noexcept;
    [[nodiscard]] const std::vector<std::shared_ptr<Effect>>& layer_effects() const noexcept;
    void add_backdrop_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {});
    void add_backdrop_effect(std::string name, std::vector<float> numbers = {});
    void clear_backdrop_effects() noexcept;
    [[nodiscard]] const std::vector<std::shared_ptr<Effect>>& backdrop_effects() const noexcept;
    void add_stylesheet_layer_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {});
    void add_stylesheet_layer_effect(std::string name, std::vector<float> numbers = {});
    void add_stylesheet_backdrop_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {});
    void add_stylesheet_backdrop_effect(std::string name, std::vector<float> numbers = {});
    void clear_stylesheet_effects() noexcept;
    [[nodiscard]] const std::vector<std::shared_ptr<Effect>>& stylesheet_layer_effects() const noexcept;
    [[nodiscard]] const std::vector<std::shared_ptr<Effect>>& stylesheet_backdrop_effects() const noexcept;

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
    void set_layout(InheritTag) noexcept;
    [[nodiscard]] const Layout& layout_rules() const noexcept;
    void set_layout_policy(SizePolicy width, SizePolicy height) noexcept;
    void set_layout_policy(InheritTag) noexcept;
    void set_margin(Insets margin) noexcept;
    void set_margin(InheritTag) noexcept;
    [[nodiscard]] Insets get_margin() const noexcept;
    void set_padding(Insets padding) noexcept;
    void set_padding(InheritTag) noexcept;
    [[nodiscard]] Insets get_padding() const noexcept;
    void set_flex(float flex) noexcept;
    void set_flex(InheritTag) noexcept;
    [[nodiscard]] float get_flex() const noexcept;
    void set_child_gravity(Gravity gravity) noexcept;
    void set_child_gravity(HorizontalGravity horizontal, VerticalGravity vertical) noexcept;
    void set_child_gravity(InheritTag) noexcept;
    [[nodiscard]] Gravity child_gravity() const noexcept;
    [[nodiscard]] Gravity get_child_gravity() const noexcept;
    void set_transform(Transform transform) noexcept;
    [[nodiscard]] const Transform& transform() const noexcept;
    [[nodiscard]] const Transform& get_transform() const noexcept;
    void set_translation(float x, float y) noexcept;
    void set_scale(float scale) noexcept;
    void set_scale(float x, float y) noexcept;
    void set_rotation(float degrees) noexcept;
    void set_transform_origin(float x, float y) noexcept;

    void set_visible(bool visible) noexcept;
    [[nodiscard]] bool visible() const noexcept;

    void set_enabled(bool enabled) noexcept;
    [[nodiscard]] bool enabled() const noexcept;

    void set_clip_content(bool clip) noexcept;
    void set_clip_content(InheritTag) noexcept;
    [[nodiscard]] bool clip_content() const noexcept;

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
    Widget& add_child(Widget* child);
    Widget& add_child(std::unique_ptr<Widget> child);
    bool remove_child(Widget& child) noexcept;
    void clear_children() noexcept;
    void set_accepts_children(bool accepts) noexcept;
    [[nodiscard]] bool accepts_children() const noexcept;

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
    [[nodiscard]] virtual bool hit_test(Point point) const noexcept;

    virtual void layout(Size available);
    virtual void render(Renderer& renderer);
    virtual bool event(const Event& event);

    static void register_css_property(std::string property, CssPropertyHandler handler);
    static bool unregister_css_property(std::string_view property);
    static void clear_css_properties();
    static void register_effect(std::string name, EffectFactory factory);
    static bool unregister_effect(std::string_view name);
    static void clear_effects();

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
    void clear_mouse_state(Point position) noexcept;
    void advance_style_motion(float dt) noexcept;
    [[nodiscard]] Style sample_animation_style(const Style& base, float progress) const noexcept;

    [[nodiscard]] Point to_local(Point point) const noexcept;
    [[nodiscard]] std::optional<MouseEvent> mouse_event_from(const Event& event) const noexcept;

    Widget* parent_ = nullptr;
    Rect bounds_;
    Style style_;
    Style target_style_;
    Style transition_from_style_;
    Style transition_to_style_;
    Style inline_style_;
    Style stylesheet_style_;
    StyleTransition transition_;
    std::optional<StyleAnimation> animation_;
    Layout layout_;
    Gravity child_gravity_ = Gravity::TopLeft();
    Transform transform_;
    std::string stylesheet_;
    std::vector<std::shared_ptr<Effect>> layer_effects_;
    std::vector<EffectParameters> layer_effect_parameters_;
    std::vector<std::shared_ptr<Effect>> backdrop_effects_;
    std::vector<EffectParameters> backdrop_effect_parameters_;
    std::vector<std::shared_ptr<Effect>> stylesheet_layer_effects_;
    std::vector<EffectParameters> stylesheet_layer_effect_parameters_;
    std::vector<std::shared_ptr<Effect>> stylesheet_backdrop_effects_;
    std::vector<EffectParameters> stylesheet_backdrop_effect_parameters_;
    std::string name_;
    std::string type_name_ = "Widget";
    std::vector<std::string> classes_;
    AccessibilityInfo accessibility_ {};
    std::vector<std::unique_ptr<Widget>> owned_children_;
    std::vector<Widget*> children_;
    bool has_inline_style_ = false;
    bool has_stylesheet_style_ = false;
    bool has_computed_style_ = false;
    bool transition_active_ = false;
    float transition_elapsed_ = 0.0f;
    float animation_elapsed_ = 0.0f;
    bool visible_ = true;
    bool enabled_ = true;
    bool clip_content_ = true;
    bool accepts_children_ = true;
    bool focusable_ = false;
    bool keyboard_activation_enabled_ = false;
    bool hovered_ = false;
    bool pressed_ = false;
    bool selected_ = false;
    bool focused_ = false;

    static Widget* focused_widget_;
};

} // namespace ouif
