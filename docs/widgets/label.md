# Label

`ouif::Label` is a leaf widget that draws text. Text itself is a renderer feature, not a separate widget model: custom widgets can call `renderer.draw_text(...)` from their own `draw()` method.

## C++

```cpp
ouif::Label title("Hello OUIF");
title.set_font_size(22.0f);
title.set_text_color("#e8edf3");
title.set_text_align(ouif::TextAlign::Center);
title.set_text_overflow(ouif::TextOverflow::Wrap);
```

`Label` does not accept children.

## Text Styling

Use `ouif::TextStyle` when you want to configure text as a single object:

```cpp
title.set_text_style(ouif::TextStyle()
    .with_font_family("OUIF Sans")
    .with_font_size(20.0f)
    .with_color("#e8edf3")
    .with_align(ouif::TextAlign::Center)
    .with_overflow(ouif::TextOverflow::Wrap));
```

If no explicit text color is set, `Label` uses the widget style foreground color. That means CSS, state changes, and animations can drive label color through the normal style pipeline.

Text color can also be a gradient:

```cpp
title.set_text_color(ouif::Gradient::Linear(90.0f, {
    { 0.0f, "#ffffff" },
    { 1.0f, "#8dc7ff" },
}));
```

## Fonts

OUIF loads a platform native default font automatically as `OUIF Sans` when rendering starts. Applications can also load TTF files and use them from any `Label` or custom `draw()` method:

```cpp
ouif::Application app;
app.load_font("Brand", "assets/brand.ttf");

ouif::Label title("Hello");
title.set_font_family("Brand");
```

Custom widgets use the same font system:

```cpp
renderer.draw_text("READY", bounds(), ouif::TextStyle()
    .with_font_family("Brand")
    .with_font_size(18.0f));
```

Use `app.set_default_font_family("Brand")` when you want labels that still say `OUIF Sans` to resolve to your loaded face.

## CSS

```css
.title {
    color: gradient(linear 90deg (0% #ffffff) (100% #8dc7ff));
    font-size: 22px;
    font-family: OUIF;
    text-align: center;
    text-overflow: wrap;
}
```

Supported text properties:

- `font-size`
- `font-family`
- `text-color`
- `text-align`
- `text-overflow`
- `color` / `foreground` through regular style foreground

## XML

```xml
<Label text="Hello OUIF" font-size="22" text-color="#e8edf3" />
```

`<Text>` is also accepted as an alias.

## Custom Drawing

```cpp
void Badge::draw(ouif::Renderer& renderer)
{
    Widget::draw(renderer);
    renderer.draw_text("READY", bounds(), ouif::TextStyle()
        .with_font_size(14.0f)
        .with_color("#ffffff")
        .with_align(ouif::TextAlign::Center));
}
```

The bgfx renderer uses TTF font atlases and falls back to OUIF's built-in glyphs if a font or text shader is unavailable.
