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

## Child Gravity

Plain `Widget` containers can align children inside their padded content area:

```cpp
panel.set_child_gravity(ouif::Gravity::BottomRight());
panel.set_child_gravity(ouif::HorizontalGravity::Right, ouif::VerticalGravity::Center);
```

Built-in linear layouts support the same idea through `set_gravity(...)`. In a `RowLayout`, horizontal gravity maps to main-axis alignment and vertical gravity maps to cross-axis alignment. In a `ColLayout`, vertical gravity maps to main-axis alignment and horizontal gravity maps to cross-axis alignment.

```cpp
row.set_gravity(ouif::Gravity::Center());
column.set_gravity(ouif::Gravity::TopRight());
```

Available preset helpers:

- `Gravity::TopLeft()`
- `Gravity::TopCenter()`
- `Gravity::TopRight()`
- `Gravity::CenterLeft()`
- `Gravity::Center()`
- `Gravity::CenterRight()`
- `Gravity::BottomLeft()`
- `Gravity::BottomCenter()`
- `Gravity::BottomRight()`

CSS and XML can use the same property:

```css
.panel {
    gravity: right bottom;
}
```

```xml
<RowLayout gravity="center" />
<Widget child-gravity="right bottom" />
```

## Built-In Containers

- `RowLayout`: left-to-right.
- `ColLayout`: top-to-bottom.
- `RowScroll`: horizontal scrolling.
- `ColScroll`: vertical scrolling.
- `Spacer`: empty layout space.
- `Divider`: visual separator line.

See [widgets](widgets/index.md) for per-container pages.

## Spacers And Dividers

Use `Spacer` for flexible empty space:

```cpp
row.add_child<ouif::Spacer>(1.0f);
```

Use `Divider` for a separator that participates in layout:

```cpp
column.add_child<ouif::Divider>(ouif::Orientation::Horizontal, 1.0f);
row.add_child<ouif::Divider>(ouif::Orientation::Vertical, 1.0f);
```

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
