# XML UI

XML UI is an external authoring layer for OUIF. It can define the window metadata, linked CSS files, embedded CSS, inline style attributes, and the widget tree.

## Loading

```cpp
ouif::Application app;
app.load_xml("ui/main.xml");
return app.run();
```

To use app-specific tags, register a factory:

```cpp
app.register_xml_widget("ColorTile", [](const ouif::XmlElement& element) {
    auto tile = std::make_unique<ColorTile>();
    tile->set_accessibility_label(std::string(element.attribute("id", "Color tile")));
    return tile;
});
```

## XML Shape

```xml
<Window title="Hello OUIF" width="960" height="540" clear_color="#101218">
    <Stylesheet src="styles/base.css" />
    <Style>
        .tile:selected { border: 4px solid #e8edf3dc; }
    </Style>

    <RowLayout id="surface" class="panel" gap="32" alignment="center" policy="fill,fill">
        <ColorTile id="blue" class="tile" size="160,120" style="background: #2f6c9c;" />
        <Input id="name" placeholder="Name" style="text-color: gradient(linear 90deg (0% #ffffff) (100% #8dc7ff));" />
    </RowLayout>
</Window>
```

## Built-In Tags

- `Widget`
- `RowLayout`
- `ColLayout`
- `RowScroll`
- `ColScroll`
- `Spacer`
- `Divider`
- `Label`
- `Input`
- `Image`
- `VectorImage`
- `Overlay`

Aliases:

- `Row`
- `Col`
- `Column`
- `HorizontalScroll`
- `VerticalScroll`
- `Text`
- `InputField`
- `TextInput`
- `Img`
- `Svg`
- `SVG`

## Common Attributes

- `id`
- `name`
- `class`
- `size`
- `width`
- `height`
- `margin`
- `padding`
- `flex`
- `weight`
- `policy`
- `alignment`
- `cross-alignment`
- `gap`
- `clip-content`
- `scroll-offset`
- `scroll-step`
- `smooth-scroll`
- `scroll-smoothing`
- `orientation`
- `thickness`
- `color`
- `focusable`
- `keyboard-activation`
- `role`
- `aria-label`
- `aria-description`
- `style`
- `transition`
- `animation`

Built-in widgets also accept their own attributes. `Label` and `Input` accept `text`, `value`, `placeholder`, `font-size`, `font-family`, `text-color`, and `placeholder-color` where applicable. `Image` accepts `src`, `source`, `image-source`, `resource`, `fit`, `filter`, and `tint`. `VectorImage` accepts the same source/resource/fit/tint pattern plus inline `svg`.

## CSS Order

Styles are loaded in this order:

1. External `<Stylesheet src="..."/>`
2. Embedded `<Style>...</Style>`
3. Generated inline `style="..."`

C++ runtime setters can still manage widgets after XML loading.

Motion attributes are converted into inline CSS and share the same runtime as C++:

```xml
<Style>
    @keyframes pulse { from { opacity: 0.6; } to { opacity: 1.0; } }
</Style>

<Widget transition="180ms ease-out" animation="pulse 1s linear infinite" />
```
