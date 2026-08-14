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
    </RowLayout>
</Window>
```

## Built-In Tags

- `Widget`
- `RowLayout`
- `ColLayout`
- `RowScroll`
- `ColScroll`

Aliases:

- `Row`
- `Col`
- `Column`
- `HorizontalScroll`
- `VerticalScroll`

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
- `focusable`
- `keyboard-activation`
- `role`
- `aria-label`
- `aria-description`
- `style`

## CSS Order

Styles are loaded in this order:

1. External `<Stylesheet src="..."/>`
2. Embedded `<Style>...</Style>`
3. Generated inline `style="..."`

C++ runtime setters can still manage widgets after XML loading.
