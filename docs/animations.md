# Animations And Transitions

OUIF has one motion model shared by C++, CSS, and XML:

- transitions interpolate from one computed style to another
- animations sample keyframes over time
- easing is shared through `ouif::Easing`
- keyframes use `ouif::Style`, so new style sources do not need a separate animation system

## C++ Transitions

```cpp
tile.set_transition(0.18f, ouif::Easing::EaseOut);
tile.set_background("#4692c4");
```

The first computed style applies immediately. Later calls to `set_style(...)`, runtime setters, or stylesheet updates can animate toward the new target style.

Use the object form when storing settings:

```cpp
tile.set_transition({
    .duration = 0.25f,
    .easing = ouif::Easing::EaseInOut,
    .enabled = true,
});
```

## C++ Keyframes

```cpp
tile.set_animation({
    .name = "pulse",
    .duration = 1.2f,
    .easing = ouif::Easing::EaseInOut,
    .loop = true,
    .keyframes = {
        { 0.0f, ouif::Style().with_opacity(0.75f), ouif::style_property_mask(ouif::StyleProperty::Opacity) },
        { 1.0f, ouif::Style().with_opacity(1.0f), ouif::style_property_mask(ouif::StyleProperty::Opacity) },
    },
});
```

`StyleKeyframe::properties` tells OUIF which style fields the keyframe owns. This lets an opacity animation leave background, border, radius, and other fields alone.

## CSS Transitions

```css
.tile {
    transition: 180ms ease-out;
}
```

Supported easing names:

- `linear`
- `ease-in`
- `ease-out`
- `ease`
- `ease-in-out`

You can also use longhand:

```css
.tile {
    transition-duration: 250ms;
    transition-timing-function: ease-in-out;
}
```

## CSS Animations

```css
@keyframes pulse {
    from { opacity: 0.75; }
    50% { opacity: 1.0; }
    to { opacity: 0.75; }
}

.tile {
    animation: pulse 1.2s ease-in-out infinite;
}
```

Supported animation properties:

- `animation`
- `animation-name`
- `animation-duration`
- `animation-timing-function`
- `animation-iteration-count`

CSS keyframes currently animate visual style fields such as background, foreground, borders, radius, and opacity. Layout animation is intentionally not enabled yet because it affects parent measurement and child flow.

## XML

XML uses the same CSS path. Put keyframes in `<Style>` or linked `<Stylesheet>`, then attach motion through `style`, `transition`, or `animation` attributes:

```xml
<Window>
    <Style>
        @keyframes fadeIn {
            from { opacity: 0.4; }
            to { opacity: 1.0; }
        }
    </Style>

    <Widget class="tile" transition="200ms ease-out" animation="fadeIn 1s linear infinite" />
</Window>
```

## Runtime Inspection

```cpp
auto transition = tile.get_transition();
auto animation = tile.get_animation();

tile.clear_transition();
tile.clear_animation();
```

`get_style()` returns the effective rendered style for the current frame, including active transition or animation sampling.
