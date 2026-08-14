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

Useful methods:

- `set_scroll_offset(float)`
- `scroll_offset()`
- `max_scroll_offset()`
- `content_size()`
- `set_scroll_step(float)`
- `scroll_step()`

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
