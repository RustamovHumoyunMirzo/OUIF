# ColScroll

`ouif::ColScroll` is a vertical scroll container. It lays out children like `ColLayout`, but keeps overflowing content scrollable on the Y axis.

## Basic Use

```cpp
class Feed : public ouif::ColScroll {
public:
    Feed()
    {
        set_gap(12.0f);
        set_padding(ouif::Insets(16.0f));
        set_scroll_step(64.0f);
        set_smooth_scroll_enabled(true);
        set_scroll_smoothing(0.18f);

        for (int index = 0; index < 20; ++index) {
            add_child<RowItem>();
        }
    }
};
```

## Scrolling

Mouse wheel events scroll the container when the cursor is inside it.

Programmatic scrolling:

```cpp
feed.set_scroll_offset(feed.scroll_offset() + feed.scroll_step());
```

`set_scroll_offset(...)` targets a new offset. When smooth scrolling is enabled, the visible content eases toward that target each frame.

For an immediate jump:

```cpp
feed.jump_to_scroll_offset(0.0f);
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

`ColScroll` clips children by default:

```cpp
feed.set_clip_content(true);
```

Set `clip_content` to false only when overflow should remain visible and interactive outside the container.

## Keyboard Example

```cpp
bool on_key_down(const ouif::KeyEvent& event) override
{
    if (event.key == 'Z') {
        set_scroll_offset(scroll_offset() - scroll_step());
        return true;
    }
    if (event.key == 'X') {
        set_scroll_offset(scroll_offset() + scroll_step());
        return true;
    }
    return false;
}
```
