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

Most style and layout setters also accept `ouif::inherit`, which copies the current value from the nearest parent:

```cpp
child.set_background(ouif::inherit);
child.set_padding(ouif::inherit);
child.set_child_gravity(ouif::inherit);
```

Inheritance always uses the widget's direct parent at the time the setter or stylesheet is applied. OUIF widgets form a tree, so there is no ambiguous multiple-parent lookup.

## CSS

```cpp
ouif::define_var("tile-bg", "#2f6c9c");

root.set_stylesheet(R"css(
    .tile {
        background: var("tile-bg");
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
- `z-index`
- `visible`, `visibility`
- `enabled`
- `ghost`
- `overlay`
- `draggable`
- `accepts-drop`
- `layer-effect`
- `backdrop-effect`
- `transition`, `transition-duration`, `transition-timing-function`
- `animation`, `animation-name`, `animation-duration`, `animation-timing-function`, `animation-iteration-count`
- label text: `font-size`, `font-family`, `text-color`, `text-align`, `text-overflow`
- image drawing: `image-source`, `image-resource`, `image-fit`, `image-filter`, `image-tint`
- vector image drawing: `svg-source`, `svg-resource`, `svg-fit`, `svg-tint`, `vector-svg`
- transforms: `transform`, `translate`, `translate-x`, `translate-y`, `scale`, `scale-x`, `scale-y`, `rotate`, `transform-origin`

`width` and `height` support `px`, `%`, `vw`, and `vh`. Spacing and radius values currently resolve as pixels.

Any supported property can use `inherit`:

```css
.tile {
    background: inherit;
    color: inherit;
    padding: inherit;
    gravity: inherit;
    image-fit: inherit;
}
```

CSS inheritance is resolved while stylesheets walk the tree from parent to child, so children see the parent's latest effective stylesheet result.

## CSS Aliases

OUIF expands a few simple CSS helper functions before Katana parses the stylesheet:

```cpp
ouif::define_var("bg-color", "#000000");
ouif::define_var("cat-path", OUIF_EXAMPLE_CAT_PATH);
ouif::define_var("cat-id", "4201");
```

```css
.panel {
    background: var("bg-color");
}

.file-image {
    image-source: path(def(cat-path));
}

.resource-image {
    image-source: res(def(cat-id));
}

.logo {
    svg-source: path(def(logo-path));
    svg-tint: var("icon-color");
}
```

- `var(name)` reads a global OUIF CSS variable.
- `def(name)` reads the same registry, and falls back to the name text when no value is registered.
- `path(value)` returns a CSS string suitable for file paths.
- `res(value)` returns a numeric resource ID suitable for `image-source` or `svg-source`.

Use `define_var(...)`, `edit_var(...)`, `delete_var(...)`, `get_var(...)`, and `clear_vars()` from C++. `Widget` also exposes static aliases with the same names.

SVG files loaded by `VectorImage` can contain their own SVG attributes and defs, including gradients and local symbol/use references. OUIF CSS chooses the SVG source, fit, tint, layout, and widget states around that vector content.

## Custom CSS Properties

Framework extensions and custom widgets can register new CSS properties:

```cpp
ouif::Widget::register_css_property("debug-number",
    [](ouif::Widget& widget, const ouif::CssDeclaration& declaration) {
        auto* custom = dynamic_cast<MyWidget*>(&widget);
        if (custom == nullptr || declaration.values.empty()) {
            return false;
        }

        if (declaration.inherited()) {
            custom->set_debug_from_parent();
            return true;
        }

        if (declaration.values.front().number) {
            custom->set_debug_value(*declaration.values.front().number);
            return true;
        }

        return false;
    });
```

`CssDeclaration` exposes the normalized property name, raw declaration text, and parsed values. Each `CssValue` can expose text, number, color, length, and whether the value is `inherit`.

Use `unregister_css_property(...)` or `clear_css_properties()` when a test, plugin, or module should stop handling a custom property.

## Effects

Effects are open C++ objects with render hooks. CSS just names and configures them:

```css
.panel {
    backdrop-effect: blur(12px);
    layer-effect: glow(8px, soft);
}
```

Built-in `blur(...)` is registered through the same effect registry developers use:

```cpp
panel.add_backdrop_effect("blur", { 12.0f });

ouif::Widget::register_effect("glow",
    [](const ouif::EffectParameters& parameters) {
        return std::make_shared<MyGlowEffect>(parameters);
    });
```

The bgfx renderer implements `blur(...)` as a shader-backed backdrop blur. OUIF captures the current scene to an internal render target, samples it through the blur shader inside the widget's rounded shape, then draws the widget content sharply above it. If the required shader assets are not available, the blur effect becomes a no-op instead of crashing.

Custom effects inherit `ouif::Effect` and can override `expand_bounds(...)`, `pre_draw(...)`, and `post_draw(...)`. Use `Renderer` primitives, `draw_backdrop_blur(...)`, or load shader binaries with `Renderer::load_shader_program(...)` when an effect needs low-level drawing.

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
