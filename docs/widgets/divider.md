# Divider

`ouif::Divider` draws a thin line for separating layout regions.

## Basic Use

Horizontal divider:

```cpp
add_child<ouif::Divider>(ouif::Orientation::Horizontal, 1.0f);
```

Vertical divider:

```cpp
add_child<ouif::Divider>(ouif::Orientation::Vertical, 1.0f);
```

## Orientation

```cpp
divider.set_orientation(ouif::Orientation::Vertical);
auto orientation = divider.orientation();
```

`Horizontal` dividers fill width and use fixed height. `Vertical` dividers use fixed width and fill height.

## Thickness

```cpp
divider.set_thickness(2.0f);
auto thickness = divider.thickness();
```

Changing thickness updates the divider's layout size on its fixed axis.

## Color

```cpp
divider.set_color("#e8edf3");
auto color = divider.color();
```

Divider color uses the normal widget background style, so CSS can style it too:

```css
.divider {
    background: #344052;
}
```

## Behavior

- Draws a styled line.
- Handles no events.
- Participates in row/column layout like any other widget.

## XML

```xml
<Divider orientation="horizontal" thickness="1" color="#344052" />
<Divider orientation="vertical" thickness="2" class="divider" />
```
