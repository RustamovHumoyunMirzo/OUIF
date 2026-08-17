# Styling

OUIF supports three styling paths:

- C++ `set_style(...)`
- runtime setters such as `set_background(...)`
- CSS strings or files through `set_stylesheet(...)`, `join_stylesheet(...)`, and XML `<Stylesheet>`

Motion uses the same style pipeline. See `docs/animations.md` for C++ transitions, CSS `@keyframes`, and XML animation attributes.

## C++ Style

```cpp
set_style(ouif::Style()
    .with_background("#2f6c9c")
    .with_background_hovered("#4692c4")
    .with_background_pressed("#161b23")
    .with_border("#e8edf3", 2.0f)
    .with_radius(12.0f));
```

`set_style()` is explicit C++ styling and wins over stylesheet styling for that widget.

## Runtime Setters

Use runtime setters for stateful or interactive changes:

```cpp
tile.set_background("#2f6c9c");
tile.set_border_left("#9fd7ff", 8.0f);
tile.set_radius({ 8.0f, 12.0f, 12.0f, 8.0f });
tile.set_opacity(0.85f);
```

Getters inspect the effective style:

```cpp
auto background = tile.get_background();
auto border = tile.get_border();
auto radius = tile.get_radius();
```

## CSS

```cpp
root.set_stylesheet(R"css(
    .tile {
        background: #2f6c9c;
        background-hovered: #4692c4;
        border: 2px solid #e8edf3;
        radius: 12px;
    }

    .tile:selected {
        background: #4692c4;
        border: 4px solid #e8edf3;
    }
)css");
```

Selectors currently support type names, classes, names, and state pseudo classes:

- `Widget`
- `.tile`
- `#primary`
- `:hover`
- `:active` / `:pressed`
- `:selected` / `:checked`
- `:focus`

## Supported Properties

- `background`
- `background-hovered`, `hover-background`
- `background-pressed`, `pressed-background`
- `background-selected`, `selected-background`
- `background-focused`, `focused-background`
- `foreground`, `color`
- `border`, `border-selected`, `border-focused`
- `border-left`, `border-top`, `border-right`, `border-bottom`
- `border-radius`, `radius`
- per-corner radius names
- `width`, `height`
- `flex`
- `margin`
- `padding`
- `clip-content`
- `transition`, `transition-duration`, `transition-timing-function`
- `animation`, `animation-name`, `animation-duration`, `animation-timing-function`, `animation-iteration-count`
- label text: `font-size`, `font-family`, `text-color`, `text-align`, `text-overflow`
- transforms: `transform`, `translate`, `translate-x`, `translate-y`, `scale`, `scale-x`, `scale-y`, `rotate`, `transform-origin`

`width` and `height` support `px`, `%`, `vw`, and `vh`. Spacing and radius values currently resolve as pixels.

## Directional Borders

```css
.tile {
    border-left: 8px solid #9fd7ff;
    border-top: 1px solid #53677d;
    border-right: 4px solid #1c2530;
    border-bottom: 6px solid #10151c;
}
```

Directional borders follow rounded corners and blend around arcs.

## Transforms

Transforms are applied at render time to a widget and its children:

```cpp
tile.set_translation(8.0f, 0.0f);
tile.set_scale(1.08f);
tile.set_rotation(4.0f);
tile.set_transform_origin(0.5f, 0.5f);
```

CSS supports both shorthand and longhand:

```css
.tile {
    transform: translate(8px, 0px) rotate(4deg) scale(1.08);
}
```

XML can use the same syntax:

```xml
<Widget transform="translate(8px, 0px) rotate(4deg) scale(1.08)" />
```
