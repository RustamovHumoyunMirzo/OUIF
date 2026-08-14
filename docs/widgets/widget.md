# Widget

`ouif::Widget` is the base class for all custom UI. Users inherit it, configure style/layout state, add children, and override behavior.

## Include

```cpp
#include <OUIF/OUIF.h>
```

## Common Setup

```cpp
class Tile : public ouif::Widget {
public:
    Tile()
    {
        set_size({ 160.0f, 120.0f });
        set_keyboard_activation_enabled(true);
        set_accessibility_role(ouif::AccessibilityRole::Button);
        set_style(ouif::Style()
            .with_background("#2f6c9c")
            .with_background_hovered("#4692c4")
            .with_border("#e8edf3", 2.0f)
            .with_radius(12.0f));
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        toggle_state(ouif::WidgetState::Selected);
        return true;
    }
};
```

## Ownership

Let OUIF own a child:

```cpp
auto& tile = parent.add_child<Tile>();
```

Or keep a child as a normal C++ member:

```cpp
class Surface : public ouif::Widget {
public:
    Surface()
    {
        add_child(tile_);
    }

private:
    Tile tile_;
};
```

Owned children are destroyed by the parent. Member/external children auto-detach when destroyed.

## Layout

Important methods:

- `set_bounds(Rect)`
- `set_size(Size)`
- `set_width(Length)`
- `set_height(Length)`
- `set_layout_policy(SizePolicy, SizePolicy)`
- `set_margin(Insets)`
- `set_padding(Insets)`
- `set_flex(float)`

Override `on_layout(Rect content)` to position children manually.

## Rendering

If you do not override `draw(Renderer&)`, the widget draws its current style background and border.

Override `draw` for custom painting:

```cpp
void draw(ouif::Renderer& renderer) override
{
    renderer.fill_rounded_rect(bounds(), { 12.0f }, get_background());
}
```

## Clipping

`clip_content` controls child overflow:

```cpp
set_clip_content(true);  // default
set_clip_content(false); // children may draw outside the widget
```

When clipping is enabled, children outside parent bounds are clipped visually and do not receive mouse events outside the clipped area.

## Events

Useful hooks:

- `on_mouse_enter`
- `on_mouse_leave`
- `on_mouse_move`
- `on_mouse_down`
- `on_mouse_up`
- `on_click`
- `on_key_down`
- `on_key_up`
- `on_keyboard_activate`

Keyboard activation defaults to calling `on_click`, so button-like widgets can define behavior once.
