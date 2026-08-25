# Rendering

OUIF uses bgfx internally for the default renderer. Application code usually does not need to touch bgfx.

## Quality

```cpp
ouif::Application app(ouif::ApplicationConfig()
    .with_render_quality(ouif::RendererQuality::Ultra));
```

Available presets:

- `Low`
- `Balanced`
- `High`
- `Ultra`

Explicit config:

```cpp
ouif::RendererQualityConfig quality;
quality.curve_segments = 24;
quality.border_curve_segments = 48;
quality.msaa_samples = 8;
quality.smoothing = true;
quality.hardware_acceleration = true;
```

## Drawing API

Custom widgets can use:

- `fill_rect`
- `fill_rounded_rect`
- `stroke_rect`
- `stroke_rounded_rect`
- `draw_text`
- `load_image`
- `draw_image`
- `destroy_image`
- `push_clip`
- `pop_clip`

Most widgets should rely on the base styled drawing unless they need a custom visual.

## Images

The bgfx renderer uses bimg to decode image bytes and stores the resulting texture internally. `ouif::ImageHandle` is an opaque handle; callers destroy it with `destroy_image(...)` when they load images manually.

```cpp
auto image = renderer.load_image("assets/photo.png");
renderer.draw_image(image, bounds(), ouif::ImageFit::Cover, ouif::ImageFilter::Linear);
renderer.destroy_image(image);
```

For ordinary UI use, prefer `ouif::Image`; it loads from a file path or `ouif::Resources` ID and draws through this same renderer path.

## Clipping

`Widget::render(...)` automatically pushes a clip rect before rendering children when `clip_content` is true. Custom renderer users can also use `push_clip` and `pop_clip` directly.
