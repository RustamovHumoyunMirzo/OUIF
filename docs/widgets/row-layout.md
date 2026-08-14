# RowLayout

`ouif::RowLayout` arranges children from left to right.

## Basic Use

```cpp
class Toolbar : public ouif::RowLayout {
public:
    Toolbar()
    {
        set_gap(12.0f);
        set_alignment(ouif::Align::Center);
        set_cross_alignment(ouif::Align::Center);
        set_padding(ouif::Insets(16.0f));

        add_child<Tile>();
        add_child<Tile>();
        add_child<Tile>();
    }
};
```

## Main Axis

The main axis is horizontal. `set_alignment` controls how fixed-size children are placed when there is extra horizontal space:

- `Align::Start`
- `Align::Center`
- `Align::End`

## Cross Axis

The cross axis is vertical. `set_cross_alignment` controls child vertical placement when a child does not fill height.

## Gap, Margin, And Padding

- `set_gap(float)` sets space between children.
- Parent `padding` shrinks the content area children are laid out inside.
- Child `margin` contributes to spacing around each child.

## Fill And Flex

Children with `SizePolicy::Fill` or `set_flex(...)` take remaining horizontal space.

```cpp
left.set_flex(1.0f);
right.set_flex(2.0f);
```

The second child receives twice the remaining width of the first.
