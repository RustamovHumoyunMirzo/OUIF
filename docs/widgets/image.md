# Image

`ouif::Image` is a leaf widget that draws a renderer-owned texture. It uses bimg for decoding and bgfx for upload/drawing, so users do not deal with renderer handles.

## C++

```cpp
ouif::Image preview("assets/photo.png");
preview.set_fit(ouif::ImageFit::Cover);
preview.set_filter(ouif::ImageFilter::Linear);
preview.set_tint(ouif::Color::rgba(255, 255, 255, 255));
```

Images can also load embedded resources:

```cpp
ouif::Resources::register_bytes(42, bytes, size);
preview.set_resource(42);
```

`Image` does not accept children.

## Fit

- `ImageFit::Stretch`: fill the widget bounds.
- `ImageFit::Contain`: preserve aspect ratio and fit inside the widget.
- `ImageFit::Cover`: preserve aspect ratio and crop to cover the widget.
- `ImageFit::Center`: draw at natural size centered in the widget.

## Filtering And Tint

```cpp
preview.set_filter(ouif::ImageFilter::Nearest);
preview.set_tint("#9fd7ff");
```

Use `Linear` for normal UI images and `Nearest` for pixel art. Tint multiplies the image color and alpha, so transparent PNG/TGA/etc images keep their transparency.

## CSS

```css
.preview {
    image-source: path(def(photo-path));
    image-fit: cover;
    image-filter: linear;
    image-tint: #ffffff;
}

.embedded {
    image-source: res(def(photo-resource-id));
}
```

Supported image properties:

- `image-source`, `source`, `src`
- `image-resource`, `resource`
- `image-fit`, `object-fit`
- `image-filter`, `image-rendering`
- `image-tint`, `tint`

`image-fit`, `image-filter`, and `image-tint` support `inherit`.

`image-source` accepts regular strings, `path(...)`, or `res(...)`. `path(...)` is for file paths; `res(...)` is for IDs registered through `ouif::Resources`.

## XML

```xml
<Image src="assets/photo.png" fit="cover" filter="linear" tint="#ffffff" />
```

`<Img>` is also accepted as an alias. XML image paths are resolved relative to the XML file.
