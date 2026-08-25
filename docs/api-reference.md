# API Reference

All user-facing APIs are available through:

```cpp
#include <OUIF/OUIF.h>
```

This page lists public types and methods. See the topic docs for examples and behavior notes.

## Geometry

### `ouif::Point`

Fields:

- `float x`
- `float y`

### `ouif::Size`

Fields:

- `float width`
- `float height`

### `ouif::Length`

Fields:

- `float value`
- `LengthUnit unit`

Static constructors:

- `static constexpr Length auto_value() noexcept -> Length`
- `static constexpr Length px(float value) noexcept -> Length`
- `static constexpr Length percent(float value) noexcept -> Length`
- `static constexpr Length vw(float value) noexcept -> Length`
- `static constexpr Length vh(float value) noexcept -> Length`

Methods:

- `automatic() const noexcept -> bool`
- `resolve(Size available, bool horizontal) const noexcept -> float`

### `ouif::Insets`

Constructors:

- `Insets() noexcept`
- `Insets(float all) noexcept`
- `Insets(float horizontal, float vertical) noexcept`
- `Insets(float left, float top, float right, float bottom) noexcept`

Fields:

- `float left`
- `float top`
- `float right`
- `float bottom`

### `ouif::Rect`

Fields:

- `float x`
- `float y`
- `float width`
- `float height`

Methods:

- `contains(Point point) const noexcept -> bool`
- `inset(Insets insets) const noexcept -> Rect`

### `ouif::Gravity`

Fields:

- `HorizontalGravity horizontal`
- `VerticalGravity vertical`

Static presets:

- `TopLeft() noexcept -> Gravity`
- `TopCenter() noexcept -> Gravity`
- `TopRight() noexcept -> Gravity`
- `CenterLeft() noexcept -> Gravity`
- `Center() noexcept -> Gravity`
- `CenterRight() noexcept -> Gravity`
- `BottomLeft() noexcept -> Gravity`
- `BottomCenter() noexcept -> Gravity`
- `BottomRight() noexcept -> Gravity`

### `ouif::inherit`

`ouif::inherit` is an `InheritTag` value accepted by style, text, layout, spacing, clipping, and gravity setters. It copies the corresponding value from the widget's direct parent.

## Color

### `ouif::Color`

Constructors:

- `Color() noexcept`
- `Color(float red, float green, float blue, float alpha = 1.0f) noexcept`
- `Color(std::string_view hex_value) noexcept`
- `Color(const char* hex_value) noexcept`

Static constructors:

- `rgba(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept -> Color`
- `rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept -> Color`
- `hex(std::uint32_t rgb) noexcept -> Color`
- `hexa(std::uint32_t rgba) noexcept -> Color`
- `from_hex(std::string_view value) noexcept -> std::optional<Color>`

Fields:

- `float r`
- `float g`
- `float b`
- `float a`

## Style

### `ouif::Border`

Constructors:

- `Border() noexcept`
- `Border(Color color, float width) noexcept`

Fields:

- `Color color`
- `float width`

### `ouif::BorderEdges`

Constructors:

- `BorderEdges() noexcept`
- `BorderEdges(Border all) noexcept`

Methods:

- `empty() const noexcept -> bool`
- `uniform() const noexcept -> bool`

Fields:

- `Border left`
- `Border top`
- `Border right`
- `Border bottom`

### `ouif::Style`

Fluent methods return `Style&`:

- `with_background(Color color) noexcept`
- `with_background_hovered(Color color) noexcept`
- `with_background_pressed(Color color) noexcept`
- `with_background_selected(Color color) noexcept`
- `with_background_focused(Color color) noexcept`
- `with_foreground(Color color) noexcept`
- `with_border(Color color, float width) noexcept`
- `with_border_selected(Color color, float width) noexcept`
- `with_border_focused(Color color, float width) noexcept`
- `with_border_left(Color color, float width) noexcept`
- `with_border_top(Color color, float width) noexcept`
- `with_border_right(Color color, float width) noexcept`
- `with_border_bottom(Color color, float width) noexcept`
- `with_border_left_selected(Color color, float width) noexcept`
- `with_border_top_selected(Color color, float width) noexcept`
- `with_border_right_selected(Color color, float width) noexcept`
- `with_border_bottom_selected(Color color, float width) noexcept`
- `with_border_left_focused(Color color, float width) noexcept`
- `with_border_top_focused(Color color, float width) noexcept`
- `with_border_right_focused(Color color, float width) noexcept`
- `with_border_bottom_focused(Color color, float width) noexcept`
- `with_radius(float value) noexcept`
- `with_radius(CornerRadius value) noexcept`
- `with_opacity(float value) noexcept`

Public fields include base/state colors, foreground, uniform borders, directional borders, radius, and opacity.

## Widget

### `ouif::Layout`

Fields:

- `Size preferred_size`
- `Size min_size`
- `Size max_size`
- `Insets margin`
- `Insets padding`
- `Length width_value`
- `Length height_value`
- `float flex`
- `SizePolicy width`
- `SizePolicy height`

### CSS Extension Types

`ouif::CssValue` fields:

- `std::string text`
- `std::optional<float> number`
- `std::optional<Color> color`
- `Length length`
- `bool inherit`

`ouif::CssDeclaration` fields and methods:

- `std::string property`
- `std::vector<CssValue> values`
- `std::string raw`
- `inherited() const noexcept -> bool`

`ouif::CssPropertyHandler`:

- `std::function<bool(Widget&, const CssDeclaration&)>`

### Effect Types

`ouif::EffectLayer` values:

- `Layer`
- `Backdrop`

`ouif::EffectParameters` fields:

- `std::string name`
- `std::vector<float> numbers`
- `std::vector<std::string> args`

`ouif::EffectContext` fields:

- `Renderer& renderer`
- `Widget& widget`
- `Rect bounds`
- `EffectLayer layer`
- `const EffectParameters& parameters`

`ouif::Effect` methods:

- `virtual ~Effect()`
- `expand_bounds(const EffectContext& context) const -> Rect`
- `pre_draw(const EffectContext& context) -> void`
- `post_draw(const EffectContext& context) -> void`

`ouif::EffectFactory`:

- `std::function<std::shared_ptr<Effect>(const EffectParameters&)>`

`ouif::BlurEffect`:

- `BlurEffect(float radius = 8.0f) noexcept`
- `set_radius(float radius) noexcept -> void`
- `radius() const noexcept -> float`
- overrides `expand_bounds(...)`, `pre_draw(...)`, and `post_draw(...)`

### `ouif::Widget`

Construction:

- `Widget()`
- `virtual ~Widget()`
- non-copyable and non-movable

Bounds and size:

- `set_bounds(Rect bounds) noexcept -> void`
- `bounds() const noexcept -> Rect`
- `set_size(Size size) noexcept -> void`
- `set_size(InheritTag) noexcept -> void`
- `set_width(Length width) noexcept -> void`
- `set_width(InheritTag) noexcept -> void`
- `set_height(Length height) noexcept -> void`
- `set_height(InheritTag) noexcept -> void`
- `set_size(Length width, Length height) noexcept -> void`

Style:

- `set_style(Style style) noexcept -> void`
- `set_style(InheritTag) noexcept -> void`
- `style() const noexcept -> const Style&`
- `get_style() const noexcept -> const Style&`
- `set_background(Color color) noexcept -> void`
- `set_background(InheritTag) noexcept -> void`
- `get_background() const noexcept -> Color`
- `set_background_hovered(Color color) noexcept -> void`
- `set_background_hovered(InheritTag) noexcept -> void`
- `get_background_hovered() const noexcept -> Color`
- `set_background_pressed(Color color) noexcept -> void`
- `set_background_pressed(InheritTag) noexcept -> void`
- `get_background_pressed() const noexcept -> Color`
- `set_background_selected(Color color) noexcept -> void`
- `set_background_selected(InheritTag) noexcept -> void`
- `get_background_selected() const noexcept -> Color`
- `set_background_focused(Color color) noexcept -> void`
- `set_background_focused(InheritTag) noexcept -> void`
- `get_background_focused() const noexcept -> Color`
- `set_foreground(Color color) noexcept -> void`
- `set_foreground(InheritTag) noexcept -> void`
- `get_foreground() const noexcept -> Color`
- `set_border(Color color, float width) noexcept -> void`
- `set_border(InheritTag) noexcept -> void`
- `get_border() const noexcept -> Border`
- `set_border_left(Color color, float width) noexcept -> void`
- `set_border_left(InheritTag) noexcept -> void`
- `get_border_left() const noexcept -> Border`
- `set_border_top(Color color, float width) noexcept -> void`
- `set_border_top(InheritTag) noexcept -> void`
- `get_border_top() const noexcept -> Border`
- `set_border_right(Color color, float width) noexcept -> void`
- `set_border_right(InheritTag) noexcept -> void`
- `get_border_right() const noexcept -> Border`
- `set_border_bottom(Color color, float width) noexcept -> void`
- `set_border_bottom(InheritTag) noexcept -> void`
- `get_border_bottom() const noexcept -> Border`
- `set_border_selected(Color color, float width) noexcept -> void`
- `set_border_selected(InheritTag) noexcept -> void`
- `get_border_selected() const noexcept -> Border`
- `set_border_focused(Color color, float width) noexcept -> void`
- `set_border_focused(InheritTag) noexcept -> void`
- `get_border_focused() const noexcept -> Border`
- `set_radius(float radius) noexcept -> void`
- `set_radius(CornerRadius radius) noexcept -> void`
- `set_radius(InheritTag) noexcept -> void`
- `get_radius() const noexcept -> CornerRadius`
- `set_opacity(float opacity) noexcept -> void`
- `set_opacity(InheritTag) noexcept -> void`
- `get_opacity() const noexcept -> float`

Motion:

- `set_transition(StyleTransition transition) noexcept -> void`
- `set_transition(float duration, Easing easing = Easing::EaseOut) noexcept -> void`
- `clear_transition() noexcept -> void`
- `transition() const noexcept -> const StyleTransition&`
- `get_transition() const noexcept -> const StyleTransition&`
- `set_animation(StyleAnimation animation) -> void`
- `clear_animation() noexcept -> void`
- `animation() const noexcept -> const std::optional<StyleAnimation>&`
- `get_animation() const noexcept -> const std::optional<StyleAnimation>&`
- `animation_running() const noexcept -> bool`

Stylesheets and identity:

- `set_stylesheet(std::string stylesheet) -> void`
- `join_stylesheet(std::string_view stylesheet) -> void`
- `get_stylesheet() const noexcept -> std::string_view`
- `add_layer_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {}) -> void`
- `add_layer_effect(std::string name, std::vector<float> numbers = {}) -> void`
- `clear_layer_effects() noexcept -> void`
- `layer_effects() const noexcept -> const std::vector<std::shared_ptr<Effect>>&`
- `add_backdrop_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {}) -> void`
- `add_backdrop_effect(std::string name, std::vector<float> numbers = {}) -> void`
- `clear_backdrop_effects() noexcept -> void`
- `backdrop_effects() const noexcept -> const std::vector<std::shared_ptr<Effect>>&`
- `add_stylesheet_layer_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {}) -> void`
- `add_stylesheet_layer_effect(std::string name, std::vector<float> numbers = {}) -> void`
- `add_stylesheet_backdrop_effect(std::shared_ptr<Effect> effect, EffectParameters parameters = {}) -> void`
- `add_stylesheet_backdrop_effect(std::string name, std::vector<float> numbers = {}) -> void`
- `clear_stylesheet_effects() noexcept -> void`
- `stylesheet_layer_effects() const noexcept -> const std::vector<std::shared_ptr<Effect>>&`
- `stylesheet_backdrop_effects() const noexcept -> const std::vector<std::shared_ptr<Effect>>&`
- `static register_css_property(std::string property, CssPropertyHandler handler) -> void`
- `static unregister_css_property(std::string_view property) -> bool`
- `static clear_css_properties() -> void`
- `static register_effect(std::string name, EffectFactory factory) -> void`
- `static unregister_effect(std::string_view name) -> bool`
- `static clear_effects() -> void`
- `set_name(std::string name) -> void`
- `name() const noexcept -> std::string_view`
- `get_name() const noexcept -> std::string_view`
- `set_type_name(std::string type_name) -> void`
- `type_name() const noexcept -> std::string_view`
- `add_class(std::string class_name) -> Widget&`
- `remove_class(std::string_view class_name) -> bool`
- `has_class(std::string_view class_name) const noexcept -> bool`
- `classes() const noexcept -> const std::vector<std::string>&`

Layout:

- `set_layout(Layout layout) noexcept -> void`
- `set_layout(InheritTag) noexcept -> void`
- `layout_rules() const noexcept -> const Layout&`
- `set_layout_policy(SizePolicy width, SizePolicy height) noexcept -> void`
- `set_layout_policy(InheritTag) noexcept -> void`
- `set_margin(Insets margin) noexcept -> void`
- `set_margin(InheritTag) noexcept -> void`
- `get_margin() const noexcept -> Insets`
- `set_padding(Insets padding) noexcept -> void`
- `set_padding(InheritTag) noexcept -> void`
- `get_padding() const noexcept -> Insets`
- `set_flex(float flex) noexcept -> void`
- `set_flex(InheritTag) noexcept -> void`
- `get_flex() const noexcept -> float`
- `set_child_gravity(Gravity gravity) noexcept -> void`
- `set_child_gravity(HorizontalGravity horizontal, VerticalGravity vertical) noexcept -> void`
- `set_child_gravity(InheritTag) noexcept -> void`
- `child_gravity() const noexcept -> Gravity`
- `get_child_gravity() const noexcept -> Gravity`

Transform:

- `set_transform(Transform transform) noexcept -> void`
- `transform() const noexcept -> const Transform&`
- `get_transform() const noexcept -> const Transform&`
- `set_translation(float x, float y) noexcept -> void`
- `set_scale(float scale) noexcept -> void`
- `set_scale(float x, float y) noexcept -> void`
- `set_rotation(float degrees) noexcept -> void`
- `set_transform_origin(float x, float y) noexcept -> void`

Visibility, input, focus, accessibility:

- `set_visible(bool visible) noexcept -> void`
- `visible() const noexcept -> bool`
- `set_visibility(bool visible) noexcept -> void`
- `visibility() const noexcept -> bool`
- `get_visibility() const noexcept -> bool`
- `set_enabled(bool enabled) noexcept -> void`
- `enabled() const noexcept -> bool`
- `get_enabled() const noexcept -> bool`
- `set_ghost(bool ghost) noexcept -> void`
- `ghost() const noexcept -> bool`
- `get_ghost() const noexcept -> bool`
- `set_z_index(int z_index) noexcept -> void`
- `z_index() const noexcept -> int`
- `get_z_index() const noexcept -> int`
- `set_overlay(bool overlay) noexcept -> void`
- `overlay() const noexcept -> bool`
- `get_overlay() const noexcept -> bool`
- `set_clip_content(bool clip) noexcept -> void`
- `set_clip_content(InheritTag) noexcept -> void`
- `clip_content() const noexcept -> bool`
- `set_focusable(bool focusable) noexcept -> void`
- `focusable() const noexcept -> bool`
- `can_focus() const noexcept -> bool`
- `set_keyboard_activation_enabled(bool enabled) noexcept -> void`
- `keyboard_activation_enabled() const noexcept -> bool`
- `set_draggable(bool draggable) noexcept -> void`
- `draggable() const noexcept -> bool`
- `set_accepts_drop(bool accepts) noexcept -> void`
- `accepts_drop() const noexcept -> bool`
- `dragging() const noexcept -> bool`
- `set_accessibility_role(AccessibilityRole role) noexcept -> void`
- `accessibility_role() const noexcept -> AccessibilityRole`
- `set_accessibility_label(std::string label) -> void`
- `accessibility_label() const noexcept -> std::string_view`
- `set_accessibility_description(std::string description) -> void`
- `accessibility_description() const noexcept -> std::string_view`
- `set_accessibility(AccessibilityInfo info) -> void`
- `accessibility() const noexcept -> const AccessibilityInfo&`
- `focus_next(bool reverse = false) noexcept -> bool`

Children:

- `add_child(Widget& child) -> Widget&`
- `add_child(Widget* child) -> Widget&`
- `add_child(std::unique_ptr<Widget> child) -> Widget&`
- `template <typename T, typename... Args> add_child(Args&&... args) -> T&`
- `template <typename... Widgets> children(Widgets&... widgets) -> Widget&`
- `remove_child(Widget& child) noexcept -> bool`
- `remove_from_parent() noexcept -> bool`
- `clear_children() noexcept -> void`
- `set_accepts_children(bool accepts) noexcept -> void`
- `accepts_children() const noexcept -> bool`
- `children() const noexcept -> const std::vector<Widget*>&`
- `parent() noexcept -> Widget*`
- `parent() const noexcept -> const Widget*`

State and lifecycle:

- `set_state(WidgetState state, bool enabled) noexcept -> void`
- `toggle_state(WidgetState state) noexcept -> void`
- `has_state(WidgetState state) const noexcept -> bool`
- `hovered() const noexcept -> bool`
- `pressed() const noexcept -> bool`
- `focused() const noexcept -> bool`
- `focus() noexcept -> void`
- `blur() noexcept -> void`
- `hit_test(Point point) const noexcept -> bool`
- `layout(Size available) -> void`
- `render(Renderer& renderer) -> void`
- `event(const Event& event) -> bool`

Protected extension points:

- `mutable_children() noexcept -> std::vector<Widget*>&`
- `draw(Renderer& renderer) -> void`
- `on_layout(Rect content) -> void`
- `on_event(const Event& event) -> bool`
- `on_mouse_enter(const MouseEvent& event) -> void`
- `on_mouse_leave(const MouseEvent& event) -> void`
- `on_focus() -> void`
- `on_blur() -> void`
- `on_mouse_move(const MouseEvent& event) -> bool`
- `on_mouse_down(const MouseEvent& event) -> bool`
- `on_mouse_up(const MouseEvent& event) -> bool`
- `on_click(const MouseEvent& event) -> bool`
- `on_key_down(const KeyEvent& event) -> bool`
- `on_key_up(const KeyEvent& event) -> bool`
- `on_keyboard_activate(const KeyEvent& event) -> bool`
- `on_drag_start(const DragEvent& event) -> bool`
- `on_drag_move(const DragEvent& event) -> bool`
- `on_drag_end(const DragEvent& event) -> bool`
- `on_drop(const DragEvent& event) -> bool`

## Resources

### `ouif::ResourceData`

- `const std::uint8_t* data`
- `std::size_t size`
- `empty() const noexcept -> bool`
- `as_string() const -> std::string`

### `ouif::Resources`

- `static register_bytes(int id, const std::uint8_t* data, std::size_t size) -> void`
- `static load(int id) -> std::optional<ResourceData>`
- `static contains(int id) -> bool`

CMake:

- `ouif_add_file(target id file_path)`

## Layout Containers

### `ouif::LinearLayout`

- `set_alignment(Align alignment) noexcept -> void`
- `alignment() const noexcept -> Align`
- `set_cross_alignment(Align alignment) noexcept -> void`
- `cross_alignment() const noexcept -> Align`
- `set_gap(float gap) noexcept -> void`
- `gap() const noexcept -> float`
- `set_gravity(Gravity gravity) noexcept -> void`
- `set_gravity(HorizontalGravity horizontal, VerticalGravity vertical) noexcept -> void`
- `gravity() const noexcept -> Gravity`

### `ouif::RowLayout`

- `RowLayout()`

### `ouif::ColLayout`

- `ColLayout()`

### `ouif::ScrollLayout`

- `set_scroll_offset(float offset) noexcept -> void`
- `scroll_offset() const noexcept -> float`
- `max_scroll_offset() const noexcept -> float`
- `content_size() const noexcept -> Size`
- `set_scroll_step(float step) noexcept -> void`
- `scroll_step() const noexcept -> float`
- `set_smooth_scroll_enabled(bool enabled) noexcept -> void`
- `smooth_scroll_enabled() const noexcept -> bool`
- `set_scroll_smoothing(float smoothing) noexcept -> void`
- `scroll_smoothing() const noexcept -> float`
- `jump_to_scroll_offset(float offset) noexcept -> void`
- `scroll_animating() const noexcept -> bool`
- `event(const Event& event) -> bool`

### `ouif::Overlay`

- `Overlay() noexcept`

Overlay widgets do not consume Row/Col/Scroll layout space. Use `z-index` or `set_z_index(...)` to layer multiple overlays.

### `ouif::RowScroll` / `ouif::ColScroll`

- `RowScroll()`
- `ColScroll()`

## Built-In Widgets

### `ouif::Spacer`

- `Spacer() noexcept`
- `Spacer(float flex) noexcept`
- `Spacer(Size size) noexcept`
- `event(const Event& event) -> bool`

### `ouif::Divider`

- `Divider(Orientation orientation = Orientation::Horizontal, float thickness = 1.0f)`
- `set_orientation(Orientation orientation) noexcept -> void`
- `orientation() const noexcept -> Orientation`
- `set_thickness(float thickness) noexcept -> void`
- `thickness() const noexcept -> float`
- `set_color(Color color) noexcept -> void`
- `color() const noexcept -> Color`
- `event(const Event& event) -> bool`

### `ouif::Label`

- `Label()`
- `Label(std::string text)`
- `set_text(std::string text) -> void`
- `text() const noexcept -> std::string_view`
- `get_text() const noexcept -> std::string_view`
- `set_text_style(TextStyle style) noexcept -> void`
- `set_text_style(InheritTag) noexcept -> void`
- `text_style() const noexcept -> const TextStyle&`
- `get_text_style() const noexcept -> const TextStyle&`
- `set_font_family(std::string family) -> void`
- `set_font_family(InheritTag) -> void`
- `font_family() const noexcept -> std::string_view`
- `set_font_size(float size) noexcept -> void`
- `set_font_size(InheritTag) noexcept -> void`
- `font_size() const noexcept -> float`
- `set_text_color(Color color) noexcept -> void`
- `set_text_color(InheritTag) noexcept -> void`
- `text_color() const noexcept -> Color`
- `set_text_align(TextAlign align) noexcept -> void`
- `set_text_align(InheritTag) noexcept -> void`
- `text_align() const noexcept -> TextAlign`
- `set_text_overflow(TextOverflow overflow) noexcept -> void`
- `set_text_overflow(InheritTag) noexcept -> void`
- `text_overflow() const noexcept -> TextOverflow`
- `event(const Event& event) -> bool`

### `ouif::Image`

- `Image()`
- `Image(std::filesystem::path source)`
- `set_source(std::filesystem::path source) -> void`
- `set_source(std::string source) -> void`
- `source() const noexcept -> const std::filesystem::path&`
- `get_source() const noexcept -> const std::filesystem::path&`
- `set_resource(int id) noexcept -> void`
- `clear_resource() noexcept -> void`
- `resource() const noexcept -> std::optional<int>`
- `get_resource() const noexcept -> std::optional<int>`
- `set_fit(ImageFit fit) noexcept -> void`
- `set_fit(InheritTag) noexcept -> void`
- `fit() const noexcept -> ImageFit`
- `get_fit() const noexcept -> ImageFit`
- `set_filter(ImageFilter filter) noexcept -> void`
- `set_filter(InheritTag) noexcept -> void`
- `filter() const noexcept -> ImageFilter`
- `get_filter() const noexcept -> ImageFilter`
- `set_tint(Color tint) noexcept -> void`
- `set_tint(InheritTag) noexcept -> void`
- `tint() const noexcept -> Color`
- `get_tint() const noexcept -> Color`
- `loaded() const noexcept -> bool`
- `natural_size() const noexcept -> Size`
- `reload(Renderer& renderer) -> void`
- `unload(Renderer& renderer) noexcept -> void`
- `event(const Event& event) -> bool`

## Rendering

### `ouif::Renderer`

- `Renderer()`
- `~Renderer()`
- movable, non-copyable
- `initialize(const RendererConfig& config) -> void`
- `shutdown() noexcept -> void`
- `resize(std::uint32_t width, std::uint32_t height) -> void`
- `begin_frame(Color clear_color) -> void`
- `fill_rect(Rect rect, Color color) -> void`
- `fill_rounded_rect(Rect rect, CornerRadius radius, Color color) -> void`
- `stroke_rect(Rect rect, Color color, float width) -> void`
- `stroke_rounded_rect(Rect rect, CornerRadius radius, Color color, float width) -> void`
- `stroke_rounded_rect(Rect rect, CornerRadius radius, BorderEdges borders) -> void`
- `load_font(std::string family, std::filesystem::path path) -> bool`
- `load_shader_program(std::filesystem::path vertex_shader, std::filesystem::path fragment_shader) -> ShaderProgram`
- `destroy_shader_program(ShaderProgram program) noexcept -> void`
- `fill_rect_with_program(Rect rect, Color color, ShaderProgram program) -> void`
- `draw_backdrop_blur(Rect rect, CornerRadius radius, float radius_px, Color tint) -> void`
- `load_image(std::filesystem::path path) -> ImageHandle`
- `load_image(const std::uint8_t* data, std::size_t size) -> ImageHandle`
- `destroy_image(ImageHandle image) noexcept -> void`
- `image_size(ImageHandle image) const noexcept -> Size`
- `draw_image(ImageHandle image, Rect rect, ImageFit fit = ImageFit::Contain, ImageFilter filter = ImageFilter::Linear, Color tint = Color::rgba(255, 255, 255, 255)) -> void`
- `load_default_system_font() -> bool`
- `set_default_font_family(std::string family) -> void`
- `default_font_family() const noexcept -> std::string_view`
- `measure_text(std::string_view text, const TextStyle& style) const noexcept -> Size`
- `draw_text(std::string_view text, Rect rect, const TextStyle& style) -> void`
- `push_transform(Rect bounds, Transform transform) -> void`
- `pop_transform() -> void`
- `push_clip(Rect rect) -> void`
- `pop_clip() -> void`
- `end_frame() -> void`
- `initialized() const noexcept -> bool`
- `size() const noexcept -> Size`

## Application And Windows

### `ouif::ApplicationConfig`

Builder methods return `ApplicationConfig&`:

- `with_title(std::string value)`
- `with_size(std::uint32_t width, std::uint32_t height) noexcept`
- `with_native_window(void* window) noexcept`
- `with_clear_color(Color color) noexcept`
- `with_render_quality(RendererQuality quality) noexcept`
- `with_render_quality(RendererQualityConfig quality) noexcept`
- `with_window(WindowConfig value)`

### `ouif::Application`

- `Application(ApplicationConfig config = {})`
- `~Application()`
- non-copyable and non-movable
- `set_root(Widget& root) -> Widget&`
- `set_root(std::unique_ptr<Widget> root) -> Widget&`
- `template <typename T, typename... Args> set_root(Args&&... args) -> T&`
- `root() noexcept -> Widget*`
- `root() const noexcept -> const Widget*`
- `window() -> Window&`
- `window() const -> const Window&`
- `create_window(WindowConfig config = {}) -> Window&`
- `set_root(Window& window, Widget& root) -> Widget&`
- `set_root(Window& window, std::unique_ptr<Widget> root) -> Widget&`
- `template <typename T, typename... Args> set_root(Window& window, Args&&... args) -> T&`
- `root(Window& window) noexcept -> Widget*`
- `root(const Window& window) const noexcept -> const Widget*`
- `show_dialog(DialogConfig config, std::unique_ptr<Widget> root = {}) -> Window&`
- `show_dialog(const DialogBuilder& builder, std::unique_ptr<Widget> root = {}) -> Window&`
- `register_xml_widget(std::string tag_name, XmlWidgetFactory factory) -> Application&`
- `template <typename T> register_xml_widget(std::string tag_name) -> Application&`
- `load_xml(std::string_view path) -> Widget&`
- `load_xml_string(std::string_view xml, std::string_view base_path = {}) -> Widget&`
- `load_stylesheet_file(std::string_view path) -> void`
- `join_stylesheet_file(std::string_view path) -> void`
- `load_stylesheet_resource(int id) -> void`
- `join_stylesheet_resource(int id) -> void`
- `load_font(std::string family, std::filesystem::path path) -> bool`
- `load_default_system_font() -> bool`
- `set_default_font_family(std::string family) -> void`
- `default_font_family() const noexcept -> std::string_view`
- `run() -> int`
- `start() -> void`
- `poll_events() -> void`
- `should_close() const noexcept -> bool`
- `running() const noexcept -> bool`
- `frame() -> void`
- `dispatch_event(const Event& event) -> bool`
- `request_exit() noexcept -> void`

### `ouif::WindowConfig`

Builder methods return `WindowConfig&`:

- `with_title(std::string value)`
- `with_size(std::uint32_t width, std::uint32_t height) noexcept`
- `with_position(float x, float y) noexcept`
- `with_visible(bool value) noexcept`
- `with_decorated(bool value) noexcept`
- `with_resizable(bool value) noexcept`
- `with_always_on_top(bool value) noexcept`
- `with_transparent_framebuffer(bool value) noexcept`
- `with_mode(WindowMode value) noexcept`
- `with_theme(WindowTheme value) noexcept`
- `with_material(WindowMaterial value) noexcept`
- `with_background(Color value) noexcept`
- `with_owner(Window& value) noexcept`

### `ouif::Window`

- `Window() noexcept`
- `~Window()`
- movable, non-copyable
- `valid() const noexcept -> bool`
- `native_handle() const noexcept -> void*`
- `config() const noexcept -> const WindowConfig&`
- `title() const noexcept -> std::string_view`
- `size() const noexcept -> Size`
- `position() const noexcept -> Point`
- `visible() const noexcept -> bool`
- `should_close() const noexcept -> bool`
- `decorated() const noexcept -> bool`
- `resizable() const noexcept -> bool`
- `always_on_top() const noexcept -> bool`
- `theme() const noexcept -> WindowTheme`
- `material() const noexcept -> WindowMaterial`
- `set_title(std::string title) -> void`
- `set_size(std::uint32_t width, std::uint32_t height) -> void`
- `set_position(float x, float y) -> void`
- `show() -> void`
- `hide() -> void`
- `focus() -> void`
- `request_close() -> void`
- `set_decorated(bool decorated) -> void`
- `set_resizable(bool resizable) -> void`
- `set_always_on_top(bool always_on_top) -> void`
- `set_opacity(float opacity) -> void`
- `set_theme(WindowTheme theme) noexcept -> void`
- `set_material(WindowMaterial material) noexcept -> void`

### `ouif::DialogBuilder`

Builder methods return `DialogBuilder&`:

- `with_title(std::string title)`
- `with_size(std::uint32_t width, std::uint32_t height) noexcept`
- `with_modal(bool modal) noexcept`
- `with_theme(WindowTheme theme) noexcept`
- `with_material(WindowMaterial material) noexcept`
- `with_background(Color background) noexcept`
- `with_owner(Window& owner) noexcept`
- `config() const noexcept -> const DialogConfig&`

## XML

### `ouif::XmlElement`

- `XmlElement()`
- `XmlElement(std::string name, std::vector<XmlAttribute> attributes)`
- `name() const noexcept -> std::string_view`
- `has_attribute(std::string_view name) const noexcept -> bool`
- `attribute(std::string_view name, std::string_view fallback = {}) const noexcept -> std::string_view`
- `attribute_float(std::string_view name) const noexcept -> std::optional<float>`
- `attribute_bool(std::string_view name) const noexcept -> std::optional<bool>`
- `attribute_color(std::string_view name) const noexcept -> std::optional<Color>`
- `attribute_size(std::string_view name) const noexcept -> std::optional<Size>`
- `attributes() const noexcept -> const std::vector<XmlAttribute>&`

## Animation Helpers

- `style_property_mask(StyleProperty property) noexcept -> StyleProperties`
- `apply_easing(Easing easing, float progress) noexcept -> float`
- `interpolate_style(const Style& from, const Style& to, float progress) noexcept -> Style`
- `apply_animated_style(const Style& base, const Style& animated, StyleProperties properties) noexcept -> Style`
- `style_equals(const Style& left, const Style& right) noexcept -> bool`
