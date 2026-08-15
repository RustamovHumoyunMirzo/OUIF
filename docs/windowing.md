# Windowing

OUIF exposes windowing through `ouif::Window`, `ouif::WindowConfig`, and dialog helpers. GLFW remains an internal backend detail; users do not include or touch GLFW types.

## Application Window

Configure the main window through `ApplicationConfig`:

```cpp
ouif::Application app(ouif::ApplicationConfig()
    .with_window(ouif::WindowConfig()
        .with_title("OUIF Tools")
        .with_size(1280, 720)
        .with_resizable(true)
        .with_theme(ouif::WindowTheme::Dark)
        .with_material(ouif::WindowMaterial::Solid)
        .with_background("#101218")));
```

After `run()` creates the native window, inspect and control it through `app.window()`:

```cpp
auto& window = app.window();
window.set_title("New Title");
window.set_size(960, 540);
window.set_position(80.0f, 80.0f);
window.set_opacity(0.95f);
window.focus();
```

`native_handle()` returns the platform native handle as `void*` for advanced embedding or platform integration. It does not expose GLFW.

## Config

`WindowConfig` supports title, size, position, visibility, decorations, resizing, focus, always-on-top, transparent framebuffers, window mode, theme, material, background color, and an owner window for dialog/tool-window relationships.

Runtime controls include:

- `show()`
- `hide()`
- `focus()`
- `request_close()`
- `set_title(...)`
- `set_size(...)`
- `set_position(...)`
- `set_decorated(...)`
- `set_resizable(...)`
- `set_always_on_top(...)`
- `set_opacity(...)`
- `set_theme(...)`
- `set_material(...)`

Getters mirror the same state: `title()`, `size()`, `position()`, `visible()`, `decorated()`, `resizable()`, `always_on_top()`, `theme()`, and `material()`.

## Additional Windows

Create extra native windows from the app:

```cpp
auto& tools = app.create_window(ouif::WindowConfig()
    .with_title("Tools")
    .with_size(480, 640)
    .with_owner(app.window())
    .with_always_on_top(true));

app.set_root<InspectorPanel>(tools);
```

The public API supports per-window roots now. The current renderer still draws the primary application root; per-window renderer swapchain support is the next backend step.

## Dialogs

Dialogs are regular OUIF windows configured for dialog behavior:

```cpp
auto& dialog = app.show_dialog(
    ouif::DialogBuilder()
        .with_title("Confirm")
        .with_size(420, 220)
        .with_modal(true),
    std::make_unique<ConfirmSurface>());
```

OUIF dialogs use the same widget system as normal windows, so custom confirmation, preferences, picker, and tool dialogs can be authored in C++, CSS, or XML-backed widget trees.
