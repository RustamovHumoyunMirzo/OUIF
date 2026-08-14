# Raw Widget API

`ouif::Widget` is the central extension point. OUIF users inherit it and build their own UI concepts on top.

The framework intentionally does not ship built-in controls yet. The goal for this stage is a strong raw widget foundation.

## Core State

Every widget has:

- bounds: `set_bounds(Rect)` and `bounds()`
- style: `set_style(Style)` and `style()`
- stylesheet: `set_stylesheet(css)`, `join_stylesheet(css)`, and `get_stylesheet()`
- layout rules: `set_layout(Layout)` and `layout_rules()`
- child widgets: `add_child(widget)` or `add_child<T>(args...)`
- visibility: `set_visible(bool)` and `visible()`
- enabled state: `set_enabled(bool)` and `enabled()`
- content clipping: `set_clip_content(bool)` and `clip_content()`
- interaction state: `hovered()` and `pressed()`
- custom state: `set_state(...)`, `toggle_state(...)`, and `has_state(...)`
- focus state: `focus()`, `blur()`, and `focused()`

## Memory And Ownership

OUIF supports two safe child patterns.

Let OUIF construct and own the child:

```cpp
auto& tile = add_child<ColorTile>("#2f6c9c", "#4692c4", ouif::Size { 160.0f, 120.0f });
```

Or keep widgets as normal variables or members and register them with the parent:

```cpp
class DemoSurface : public ouif::RowLayout {
public:
    DemoSurface()
    {
        children(blue_, violet_, green_);
    }

private:
    ColorTile blue_;
    ColorTile violet_;
    ColorTile green_;
};
```

The framework tracks parent pointers internally:

- external/member children automatically detach from their parent when destroyed
- owned children are destroyed by their parent exactly once
- duplicate `add_child(child)` calls do not register the same child twice
- external/member children can move from one parent to another
- internally owned children cannot be reparented by reference, because the original parent owns their memory

`Widget` and `Application` are intentionally non-copyable and non-movable. This keeps registered widget addresses stable.

Use `remove_child(child)` to detach or destroy a child. Use `clear_children()` to detach external children and destroy owned children.

## Style

`ouif::Style` currently includes:

- `background`
- `hovered`
- `pressed`
- `selected`
- `focused`
- `background_hovered`
- `background_pressed`
- `background_selected`
- `foreground`
- `border`
- `border_selected`
- `border_focused`
- `border_width`
- `border_width_selected`
- `radius`
- `opacity`

The current bgfx renderer draws filled rectangles, rounded rectangles, borders, and rounded borders.

Styles support a fluent builder API:

```cpp
set_style(ouif::Style()
    .with_background(base)
    .with_background_hovered(active)
    .with_background_pressed(ouif::Color::rgba(22, 27, 35, 255))
    .with_background_selected(active)
    .with_border(ouif::Color::rgba(232, 237, 243, 220), 2.0f)
    .with_border_selected(ouif::Color::rgba(232, 237, 243, 220), 4.0f));
```

For compact widget setup, `Style` also supports aggregate initialization:

```cpp
set_style(ouif::Style {
    .background = "#1c1f26",
    .hovered = "#20252e",
    .pressed = "#181c23",
    .focused = "#223148",
    .border = { "#647084", 1.0f },
    .border_focused = { "#83b7ff", 2.0f },
    .radius = ouif::CornerRadius(8.0f),
});
```

Per-corner radius is supported:

```cpp
.radius = ouif::CornerRadius(
    8.0f,  // top-left
    12.0f, // top-right
    12.0f, // bottom-right
    8.0f   // bottom-left
)
```

Hex colors are supported:

```cpp
ouif::Color color = "#2f6c9c";
auto blue = ouif::Color::hex(0x2f6c9c);
auto translucent = ouif::Color::hexa(0xe8edf3dc);
auto parsed = ouif::Color::from_hex("#2f6c9c");
```

## Stylesheets

OUIF builds Katana internally and exposes CSS styling through `Widget`; users still only include `OUIF/OUIF.h`.

```cpp
tile.add_class("tile");
tile.set_name("primary");

root.set_stylesheet(R"css(
    .tile {
        background: #2f6c9c;
        background-hovered: #4692c4;
        width: 50%;
        height: 120px;
        border-radius: 8px;
        border: #e8edf3 2px;
    }

    #primary:selected {
        background: #4692c4;
        border: #e8edf3 4px;
    }
)css");
```

Selectors currently support:

- `*`
- `Widget` or a value set with `set_type_name(...)`
- `.class` from `add_class(...)`
- `#name` from `set_name(...)`
- state pseudo classes: `:hover`, `:active`, `:pressed`, `:selected`, `:checked`, `:focus`

CSS property names intentionally mirror `Style` naming:

- `background`, `background-hovered`, `background-pressed`, `background-selected`, `background-focused`
- `foreground` or `color`
- `border`, `border-selected`, `border-focused`
- `border-left`, `border-top`, `border-right`, `border-bottom`
- `border-left-width`, `border-top-width`, `border-right-width`, `border-bottom-width`
- `border-radius`, `radius`, and per-corner radius names
- `width`, `height`, `flex`, `margin`, `padding`, `clip-content`

`width` and `height` support `px`, `%`, `vw`, and `vh`. `margin`, `padding`, and radius currently resolve as pixel values.

`set_style()` is treated as explicit C++ styling and wins over stylesheet styling for that widget. This keeps runtime behavior deterministic when both APIs are used. Use `get_style()` to inspect the effective style and `get_stylesheet()` to inspect the CSS string. `join_stylesheet()` appends more CSS and reapplies it to the tree.

For runtime changes, widgets also expose direct style setters and getters:

```cpp
tile.set_background("#2f6c9c");
tile.set_background_hovered("#4692c4");
tile.set_border("#e8edf3", 2.0f);
tile.set_border_left("#9fd7ff", 8.0f);
tile.set_border_bottom("#102838", 6.0f);
tile.set_radius(8.0f);
tile.set_opacity(0.85f);

auto background = tile.get_background();
auto border = tile.get_border();
auto left_border = tile.get_border_left();
auto radius = tile.get_radius();
```

These convenience setters are C++ runtime style changes, so they also override stylesheet values for that widget.

Directional borders follow web-style naming and can be mixed:

```cpp
set_style(ouif::Style()
    .with_background("#263241")
    .with_border_top("#53677d", 1.0f)
    .with_border_right("#1c2530", 4.0f)
    .with_border_bottom("#10151c", 6.0f)
    .with_border_left("#9fd7ff", 8.0f));
```

```css
.tile {
    border-left: #9fd7ff 8px;
    border-top: #53677d 1px;
    border-right: #1c2530 4px;
    border-bottom: #10151c 6px;
}

.tile:selected {
    border-left: #ffffff 4px;
    border-right: #ffffff 4px;
}
```

Uniform and mixed directional borders use rounded border rendering. Corner arcs smoothly blend adjacent side colors and widths while staying inside the rounded border shape.

## Rendering Quality

Rounded borders and corners are tessellated, so low segment counts can look pixelated. OUIF defaults to high quality, and apps can raise or lower the cost:

```cpp
ouif::Application app(ouif::ApplicationConfig()
    .with_render_quality(ouif::RendererQuality::Ultra));
```

For explicit control:

```cpp
ouif::RendererQualityConfig quality;
quality.preset = ouif::RendererQuality::High;
quality.curve_segments = 24;
quality.border_curve_segments = 48;
quality.msaa_samples = 8;
quality.smoothing = true;
quality.hardware_acceleration = true;

ouif::Application app(ouif::ApplicationConfig()
    .with_render_quality(quality));
```

Higher segment counts make rounded fills and mixed directional border curves smoother. MSAA asks bgfx for hardware multisampling where the selected backend supports it.

## Layout

`ouif::Layout` includes:

- `preferred_size`
- `min_size`
- `max_size`
- `margin`
- `padding`
- `width_value`
- `height_value`
- `flex`
- `width`
- `height`

`width` and `height` use `ouif::SizePolicy`:

- `Fixed`
- `Fill`
- `Content`

The base implementation computes its own bounds and gives children the padded content area. Custom containers can override `on_layout(Rect content)` to position children.

Use `set_size(Size)` for fixed-size widgets:

```cpp
set_size({ 160.0f, 120.0f });
```

Use web-like lengths when a value depends on the available space:

```cpp
set_width(ouif::Length::percent(50.0f));
set_height(ouif::Length::vh(25.0f));
```

Use `set_layout_policy(width, height)` for fill/content behavior:

```cpp
set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
```

Spacing helpers:

```cpp
set_margin(ouif::Insets(12.0f));
set_padding(24.0f);
auto margin = get_margin();
auto padding = get_padding();
```

Flex/weight works like Android weight or CSS flex grow:

```cpp
left.set_flex(1.0f);
right.set_flex(2.0f);
auto weight = right.get_flex();
```

The second widget receives twice as much remaining main-axis space as the first.

## Built-In Layout Containers

OUIF includes simple linear layout containers:

- `ouif::RowLayout`
- `ouif::ColLayout`
- `ouif::RowScroll`
- `ouif::ColScroll`

They support:

- `set_alignment(ouif::Align::Start)`
- `set_alignment(ouif::Align::Center)`
- `set_alignment(ouif::Align::End)`
- `set_gap(float)`
- `set_cross_alignment(ouif::Align::Center)`

`RowScroll` and `ColScroll` use the same alignment, gap, margin, and padding ideas, but measure overflowing content on the main axis and respond to mouse wheel events. Children are clipped to the scroll container by default through `clip_content`.

```cpp
class Palette : public ouif::ColScroll {
public:
    Palette()
    {
        set_gap(12.0f);
        set_scroll_step(56.0f);
        set_smooth_scroll_enabled(true);
        set_scroll_smoothing(0.18f);
        set_clip_content(true);

        for (int index = 0; index < 12; ++index) {
            add_child<ColorTile>(ouif::Size { 160.0f, 48.0f });
        }
    }
};
```

Useful scroll APIs:

- `set_scroll_offset(float)` and `scroll_offset()`
- `max_scroll_offset()`
- `content_size()`
- `set_scroll_step(float)` and `scroll_step()`
- `set_smooth_scroll_enabled(bool)` and `smooth_scroll_enabled()`
- `set_scroll_smoothing(float)` and `scroll_smoothing()`
- `jump_to_scroll_offset(float)`
- `scroll_animating()`

Use `set_clip_content(false)` when a container should allow children to draw and receive mouse events outside its bounds.

```cpp
class DemoSurface : public ouif::RowLayout {
public:
    DemoSurface()
    {
        set_alignment(ouif::Align::Center);
        set_cross_alignment(ouif::Align::Center);
        set_gap(32.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);

        add_child<ColorTile>(base, active, ouif::Size { 160.0f, 120.0f });
    }
};
```

You can also define children as normal member variables and register them with the parent:

```cpp
class DemoSurface : public ouif::RowLayout {
public:
    DemoSurface()
        : tile_(ouif::Color::hex(0x2f6c9c), ouif::Color::hex(0x4692c4), { 160.0f, 120.0f })
    {
        children(tile_);
    }

private:
    ColorTile tile_;
};
```

Register multiple member widgets in one call:

```cpp
children(blue_, violet_, green_);
```

```cpp
class Row : public ouif::Widget {
protected:
    void on_layout(ouif::Rect content) override
    {
        float x = content.x;
        for (const auto& child : children()) {
            child->set_bounds({ x, content.y, 120.0f, content.height });
            x += 132.0f;
        }
    }
};
```

## Drawing

Override `draw(Renderer&)` for custom visuals.

```cpp
class Tile : public ouif::Widget {
protected:
    void draw(ouif::Renderer& renderer) override
    {
        renderer.fill_rect(bounds(), hovered() ? hover_color : base_color);
        renderer.stroke_rect(bounds(), border_color, 2.0f);
    }

private:
    ouif::Color base_color = ouif::Color::rgb(42, 92, 130);
    ouif::Color hover_color = ouif::Color::rgb(58, 118, 160);
    ouif::Color border_color = ouif::Color::rgb(180, 218, 255);
};
```

If you do not override `draw`, the base widget draws its styled background and border.

## Events

Widgets can either handle the generic variant:

```cpp
bool on_event(const ouif::Event& event) override;
```

Or override mouse-specific hooks:

```cpp
void on_mouse_enter(const ouif::MouseEvent& event) override;
void on_mouse_leave(const ouif::MouseEvent& event) override;
bool on_mouse_move(const ouif::MouseEvent& event) override;
bool on_mouse_down(const ouif::MouseEvent& event) override;
bool on_mouse_up(const ouif::MouseEvent& event) override;
bool on_click(const ouif::MouseEvent& event) override;
```

The base widget tracks hover and pressed state automatically. Events are sent to children in reverse child order so later children behave as visually front-most.

Selected state can be toggled without manually rewriting the style:

```cpp
bool on_click(const ouif::MouseEvent&) override
{
    toggle_state(ouif::WidgetState::Selected);
    return true;
}
```

## Accessibility And Keyboard

Widgets opt into keyboard navigation:

```cpp
class Tile : public ouif::Widget {
public:
    Tile()
    {
        set_keyboard_activation_enabled(true);
        set_accessibility_role(ouif::AccessibilityRole::Button);
        set_accessibility_label("Color tile");
        set_accessibility_description("Toggles the selected state");
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        toggle_state(ouif::WidgetState::Selected);
        return true;
    }
};
```

Keyboard behavior:

- `Tab` moves to the next focusable widget.
- `Shift+Tab` moves to the previous focusable widget.
- `Enter` or `Space` triggers `on_keyboard_activate()` for the focused widget when keyboard activation is enabled.
- The default `on_keyboard_activate()` calls `on_click()`, so custom widgets usually only need to override click behavior once.

Use `set_focusable(true)` for focus-only widgets, or `set_keyboard_activation_enabled(true)` for controls that should respond to keyboard clicks. Disabled, hidden, or non-focusable widgets are skipped by keyboard navigation.

Available accessible metadata:

```cpp
set_accessibility({
    ouif::AccessibilityRole::Button,
    "Save",
    "Saves the current document",
});
```

## Hit Testing

The default `hit_test(Point)` checks:

- widget is visible
- widget is enabled
- point is inside bounds

Override event hooks for behavior, and use `set_enabled(false)` or `set_visible(false)` to remove a widget from interaction.
