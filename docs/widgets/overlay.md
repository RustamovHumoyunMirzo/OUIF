# Overlay

`ouif::Overlay` is a layered container. It can be added as a child without consuming RowLayout, ColLayout, RowScroll, or ColScroll flow space.

```cpp
ouif::RowLayout row;
auto& left = row.add_child<ouif::Widget>();
auto& overlay = row.add_child<ouif::Overlay>();
auto& right = row.add_child<ouif::Widget>();

left.set_size({ 100.0f, 40.0f });
right.set_size({ 100.0f, 40.0f });
overlay.set_bounds({ 12.0f, 12.0f, 240.0f, 160.0f });
overlay.set_z_index(100);
```

The overlay keeps its own bounds. If no bounds are set, it fills the parent content area.

## CSS

Any widget can opt into overlay behavior:

```css
.floating {
    overlay: true;
    z-index: 100;
}
```

XML can use the built-in tag:

```xml
<Overlay id="menu" z-index="100" ghost="false" />
```

## API

- `Overlay() noexcept`
- inherited `set_overlay(bool)`
- inherited `overlay()`
- inherited `set_z_index(int)`
- inherited `z_index()`

Use `ghost: true` or `set_ghost(true)` when an overlay should be visible but let pointer hits pass through to widgets below.
