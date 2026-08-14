# Raw Widget API

`ouif::Widget` is the central extension point. OUIF users inherit it and build their own UI concepts on top.

The framework intentionally does not ship built-in controls yet. The goal for this stage is a strong raw widget foundation.

## Core State

Every widget has:

- bounds: `set_bounds(Rect)` and `bounds()`
- style: `set_style(Style)` and `style()`
- layout rules: `set_layout(Layout)` and `layout_rules()`
- child widgets: `add_child(widget)` or `add_child<T>(args...)`
- visibility: `set_visible(bool)` and `visible()`
- enabled state: `set_enabled(bool)` and `enabled()`
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

The current bgfx renderer draws filled rectangles and borders. `radius` is stored for API stability, but rounded rendering is not implemented yet.

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

## Layout

`ouif::Layout` includes:

- `preferred_size`
- `min_size`
- `max_size`
- `margin`
- `padding`
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

Use `set_layout_policy(width, height)` for fill/content behavior:

```cpp
set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
```

Spacing helpers:

```cpp
set_margin(ouif::Insets(12.0f));
set_padding(24.0f);
```

Flex/weight works like Android weight or CSS flex grow:

```cpp
left.set_flex(1.0f);
right.set_flex(2.0f);
```

The second widget receives twice as much remaining main-axis space as the first.

## Built-In Layout Containers

OUIF includes simple linear layout containers:

- `ouif::RowLayout`
- `ouif::ColLayout`

They support:

- `set_alignment(ouif::Align::Start)`
- `set_alignment(ouif::Align::Center)`
- `set_alignment(ouif::Align::End)`
- `set_gap(float)`
- `set_cross_alignment(ouif::Align::Center)`

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

## Hit Testing

The default `hit_test(Point)` checks:

- widget is visible
- widget is enabled
- point is inside bounds

Override event hooks for behavior, and use `set_enabled(false)` or `set_visible(false)` to remove a widget from interaction.
