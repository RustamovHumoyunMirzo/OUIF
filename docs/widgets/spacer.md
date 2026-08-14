# Spacer

`ouif::Spacer` reserves layout space without drawing anything and without handling input.

Use it when a row or column needs flexible empty space between real widgets.

## Basic Use

```cpp
class Toolbar : public ouif::RowLayout {
public:
    Toolbar()
    {
        add_child<Button>();
        add_child<ouif::Spacer>(1.0f);
        add_child<Button>();
    }
};
```

The constructor value is flex/weight. In a `RowLayout`, the spacer expands horizontally. In a `ColLayout`, it expands vertically.

## Fixed Size

```cpp
add_child<ouif::Spacer>(ouif::Size { 24.0f, 1.0f });
```

Use fixed-size spacers when you need an exact gap that is easier to read as a child than as layout margin.

## Behavior

- Draws nothing.
- Handles no events.
- Can still use normal layout APIs such as `set_size`, `set_margin`, and `set_flex`.

## XML

```xml
<Spacer flex="1" />
<Spacer size="24,1" />
```
