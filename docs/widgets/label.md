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

## CSS

```css
.title {
    color: #e8edf3;
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

The first renderer backend uses OUIF's built-in fallback glyphs. The public API is designed so a future font atlas or TTF backend can be added without changing user widget code.
