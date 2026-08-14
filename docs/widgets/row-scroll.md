# RowScroll

`ouif::RowScroll` is a horizontal scroll container. It lays out children like `RowLayout`, but keeps overflowing content scrollable on the X axis.

## Basic Use

```cpp
class Palette : public ouif::RowScroll {
public:
    Palette()
    {
        set_gap(16.0f);
        set_padding(ouif::Insets(16.0f));
        set_scroll_step(72.0f);
        set_smooth_scroll_enabled(true);
        set_scroll_smoothing(0.18f);

        for (int index = 0; index < 12; ++index) {
            add_child<Tile>();
        }
    }
};
```

## Scrolling

Mouse wheel events scroll the container when the cursor is inside it.

Programmatic scrolling:

```cpp
palette.set_scroll_offset(palette.scroll_offset() + palette.scroll_step());
```

`set_scroll_offset(...)` targets a new offset. When smooth scrolling is enabled, the visible content eases toward that target each frame.

For an immediate jump:

```cpp
palette.jump_to_scroll_offset(0.0f);
```

Useful methods:

- `set_scroll_offset(float)`
- `scroll_offset()`
- `max_scroll_offset()`
- `content_size()`
- `set_scroll_step(float)`
- `scroll_step()`
- `set_smooth_scroll_enabled(bool)`
- `smooth_scroll_enabled()`
- `set_scroll_smoothing(float)`
- `scroll_smoothing()`
- `jump_to_scroll_offset(float)`
- `scroll_animating()`

`scroll_smoothing` is clamped from `0.01` to `1.0`. Lower values feel softer and heavier. `1.0` reaches the target immediately.

## Clipping

`RowScroll` uses `clip_content = true` by default, so overflowing children are clipped to the scroll bounds.

```cpp
palette.set_clip_content(false);
```

Turning clipping off is useful for debugging overflow or for intentional pop-out effects.

## Keyboard Example

```cpp
bool on_key_down(const ouif::KeyEvent& event) override
{
    if (event.key == 'A') {
        set_scroll_offset(scroll_offset() - scroll_step());
        return true;
    }
    if (event.key == 'D') {
        set_scroll_offset(scroll_offset() + scroll_step());
        return true;
    }
    return false;
}
```
