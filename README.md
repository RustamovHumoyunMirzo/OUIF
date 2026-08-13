# OUIF

An Open User Interface Framework for C++ developers who want to build custom UI without hauling a giant runtime or exposing rendering complexity to application code.

OUIF is designed around a small public API:

- Include `OUIF/OUIF.h`.
- Inherit `ouif::Widget`.
- Override layout, rendering, and event behavior.
- Let OUIF own the renderer, window backend, CMake wiring, and install/package details.

## Build

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

On Windows you can also use:

```powershell
.\scripts\build.ps1
.\scripts\test.ps1
```

By default CMake fetches bgfx and GLFW privately. bgfx is linked into OUIF, so users do not need to install or configure the renderer themselves. GLFW is used by `ouif::Application` for the convenience app/window path.

## Use From CMake

After installing OUIF:

```cmake
find_package(OUIF CONFIG REQUIRED)

ouif_add_app(my_app SOURCES main.cpp)
```

Or link directly:

```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE OUIF::ouif)
```

## Drawing Into Your Own Window

Apps with an existing platform window can skip OUIF window creation and drive frames themselves:

```cpp
ouif::Application app({
    .width = 1280,
    .height = 720,
    .native_window = my_native_window_handle,
    .create_window = false,
});

app.set_root(std::make_unique<MyWidget>());

while (running) {
    app.dispatch_event(next_ouif_event);
    app.frame();
}
```

For lower-level integration, create `ouif::Renderer` directly with `RendererConfig::native_window` and call `widget.layout(...)`, `widget.render(...)`, and `widget.event(...)` from your own loop.

## Minimal App

```cpp
#include <OUIF/OUIF.h>

class MyWidget : public ouif::Widget {
public:
    MyWidget()
    {
        ouif::Style style;
        style.background = ouif::Color::rgb(42, 92, 130);
        set_style(style);
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        return true;
    }
};

int main()
{
    ouif::Application app({ "My OUIF App", 960, 540 });
    app.set_root(std::make_unique<MyWidget>());
    return app.run();
}
```

## Project Shape

- `include/OUIF/OUIF.h`: main public umbrella header.
- `include/OUIF/Widget.h`: base class users inherit from.
- `include/OUIF/Event.h`: public event types.
- `include/OUIF/Style.h`: style properties exposed to widgets.
- `src/Renderer.cpp`: private bgfx-backed renderer boundary.
- `cmake/OUIFFunctions.cmake`: helper functions exported to users.
