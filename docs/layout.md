# Layout

OUIF layout is intentionally small. Widgets own their own layout rules, and containers decide how to place children.

## Size

Fixed size:

```cpp
set_size({ 160.0f, 120.0f });
```

Length values:

```cpp
set_width(ouif::Length::percent(50.0f));
set_height(ouif::Length::vh(30.0f));
```

## Size Policy

```cpp
set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
```

Policies:

- `Fixed`: use preferred size or explicit size.
- `Fill`: take available space from the parent/container.
- `Content`: reserved for content-driven sizing as OUIF grows.

## Spacing

```cpp
set_margin(ouif::Insets(8.0f));
set_padding(ouif::Insets(16.0f));
```

`Insets` can be constructed as:

```cpp
ouif::Insets all(8.0f);
ouif::Insets horizontal_vertical(12.0f, 8.0f);
ouif::Insets edges(4.0f, 8.0f, 12.0f, 16.0f);
```

## Flex

Flex works like Android weight or CSS flex grow:

```cpp
left.set_flex(1.0f);
right.set_flex(2.0f);
```

The second widget gets twice the remaining main-axis space.

## Built-In Containers

- `RowLayout`: left-to-right.
- `ColLayout`: top-to-bottom.
- `RowScroll`: horizontal scrolling.
- `ColScroll`: vertical scrolling.

See [widgets](widgets/index.md) for per-container pages.

## Custom Containers

Override `on_layout(Rect content)`:

```cpp
class Stack : public ouif::Widget {
protected:
    void on_layout(ouif::Rect content) override
    {
        for (auto* child : children()) {
            child->set_bounds(content);
        }
    }
};
```

The base `Widget::layout(...)` calls `on_layout(...)`, then layouts children using their assigned bounds.
