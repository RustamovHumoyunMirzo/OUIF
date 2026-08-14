# OUIF

An Open User Interface Framework for C++ developers who want to build custom UI without hauling a giant runtime or exposing rendering complexity to application code.

OUIF is designed around a small public API:

- Include `OUIF/OUIF.h`.
- Inherit `ouif::Widget`.
- Override layout, rendering, and event behavior.
- Let OUIF own the renderer, window backend, CMake wiring, and install/package details.

## Status

OUIF is early and intentionally raw. The current API exposes the foundation developers need to build their own widgets:

- a base `ouif::Widget`
- event dispatch for hover, press, click, key, and resize events
- style properties
- layout constraints and padding
- safe parent/child tracking for owned and member widgets
- bgfx-backed rectangle rendering
- GLFW-backed convenience windows
- support for drawing into an existing native window

Built-in widgets such as buttons, sliders, lists, text inputs, and layout containers will come later.

## Quick Start

Fetch the external renderer/window dependencies once:

```powershell
.\scripts\fetch-deps.ps1
```

Configure, build, and test:

```sh
cmake -S . -B build/full-example -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/full-example --target ouif_hello
ctest --test-dir build/full-example --output-on-failure
```

The example executable is written to:

`build/full-example/bin/ouif_hello.exe`

## Documentation

- [Getting Started](docs/getting-started.md)
- [Raw Widget API](docs/widget-api.md)
- [Build And Dependencies](docs/build-and-dependencies.md)

## Memory Model

OUIF supports both framework-owned widgets and Qt-like member widgets:

```cpp
row.add_child<ColorTile>("#2f6c9c", "#4692c4", ouif::Size { 160.0f, 120.0f });
children(blue_, violet_, green_);
```

External/member children auto-detach when destroyed. OUIF-owned children are deleted by their parent exactly once. Widgets are non-movable so registered addresses stay stable.

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

## Minimal App

```cpp
#include <OUIF/OUIF.h>

class MyWidget : public ouif::Widget {
public:
    MyWidget()
    {
        set_size({ 160.0f, 120.0f });
        set_style(ouif::Style()
            .with_background(ouif::Color::hex(0x2a5c82))
            .with_background_hovered(ouif::Color::hex(0x3a76a0))
            .with_background_selected(ouif::Color::hex(0x3a76a0)));
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        toggle_state(ouif::WidgetState::Selected);
        return true;
    }
};

int main()
{
    ouif::ApplicationConfig config;
    config.title = "My OUIF App";
    config.width = 960;
    config.height = 540;

    ouif::Application app(config);
    MyWidget widget;
    app.set_root(widget);
    return app.run();
}
```

## License

```
MIT License

Copyright (c) 2026 Rustamov Humoyun Mirzo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
