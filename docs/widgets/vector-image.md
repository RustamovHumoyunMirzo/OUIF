# VectorImage

`ouif::VectorImage` is a leaf widget for SVG/vector artwork. It parses SVG into OUIF vector commands and renders them through the internal vg-renderer/bgfx path.

## C++

```cpp
ouif::VectorImage logo;
logo.set_source("assets/logo.svg");
logo.set_fit(ouif::ImageFit::Contain);
logo.set_tint("#ffffff");
```

Inline and embedded SVG are also supported:

```cpp
logo.set_svg(R"svg(<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/></svg>)svg");

ouif::Resources::register_bytes(42, bytes, size);
logo.set_resource(42);
```

`VectorImage` does not accept children.

## Fit And Tint

`VectorImage` uses the same `ouif::ImageFit` values as `Image`: `Stretch`, `Contain`, `Cover`, and `Center`.

Tint multiplies SVG colors and alpha. Use white tint for original SVG colors.

## CSS

```css
.logo {
    svg-source: path(def(logo-path));
    svg-fit: contain;
    svg-tint: #ffffff;
}

.embedded-logo {
    vector-source: res(def(logo-resource-id));
}
```

Supported properties:

- `vector-source`, `svg-source`, `source`, `src`
- `vector-resource`, `svg-resource`, `resource`
- `vector-svg`, `svg`
- `vector-fit`, `svg-fit`
- `vector-tint`, `svg-tint`

`vector-fit`, `svg-fit`, `vector-tint`, and `svg-tint` support `inherit`.

## XML

```xml
<VectorImage src="assets/logo.svg" fit="contain" tint="#ffffff" />
<Svg resource="42" />
```

XML paths resolve relative to the XML file.

## Low-Level Vector Drawing

Custom widgets can draw vector primitives directly:

```cpp
void draw(ouif::Renderer& renderer) override
{
    renderer.draw_vector(bounds(), [](ouif::VectorCanvas& canvas) {
        canvas.begin_path();
        canvas.rounded_rect(8, 8, 80, 80, 16);
        canvas.fill("#4692c4");
    });
}
```

`VectorCanvas` exposes path, stroke, fill, transform, scissor, and state methods while keeping vg-renderer private to OUIF.

## SVG Features

The SVG loader supports static shapes, path commands, nested groups, transforms, local `symbol`/`use` references, `linearGradient`, `radialGradient`, `clipPath`, mask-style clipping, and `feGaussianBlur` filter metadata. Gradient paints use `fill="url(#id)"` or `stroke="url(#id)"`.
