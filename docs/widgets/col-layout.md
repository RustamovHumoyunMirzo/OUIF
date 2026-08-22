# ColLayout

`ouif::ColLayout` arranges children from top to bottom.

## Basic Use

```cpp
class Sidebar : public ouif::ColLayout {
public:
    Sidebar()
    {
        set_gap(10.0f);
        set_alignment(ouif::Align::Start);
        set_cross_alignment(ouif::Align::Center);
        set_gravity(ouif::Gravity::TopCenter());
        set_padding(ouif::Insets(16.0f));

        add_child<Tile>();
        add_child<Tile>();
    }
};
```

## Main Axis

The main axis is vertical. `set_alignment` controls placement when fixed-size children do not use all vertical space.

## Cross Axis

The cross axis is horizontal. `set_cross_alignment` controls child horizontal placement when a child does not fill width.

`set_gravity(...)` is a shorthand over both axes:

```cpp
set_gravity(ouif::Gravity::BottomRight());
```

For `ColLayout`, top/center/bottom controls vertical placement and left/center/right controls horizontal placement.

## Fill And Flex

Children with `SizePolicy::Fill` or `set_flex(...)` take remaining vertical space.

```cpp
header.set_size({ 0.0f, 56.0f });
content.set_flex(1.0f);
footer.set_size({ 0.0f, 40.0f });
```

This pattern creates a fixed header, flexible content area, and fixed footer.
