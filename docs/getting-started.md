# Getting Started

OUIF is a small C++ UI foundation. It gives you a window loop, renderer boundary, raw widget class, styles, layout state, and events. It does not try to be a complete widget toolkit yet.

## Requirements

- CMake 3.24 or newer
- A C++20 compiler
- Git
- A build tool supported by CMake

On the current Windows setup, `MinGW Makefiles` is known to work.

## Fetch Dependencies

OUIF keeps third-party source checkouts outside the framework source in `external/`. They are ignored by Git.

```powershell
.\scripts\fetch-deps.ps1
```

Unix-like shells:

```sh
./scripts/fetch-deps.sh
```

The script fetches:

- `bgfx.cmake`
- `bgfx`
- `bx`
- `bimg`
- `glfw`

This explicit layout avoids slow or fragile recursive submodule fetching during CMake configure.

## Build The Example

```powershell
cmake -S . -B build/full-example -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/full-example --target ouif_hello
```

Run:

```powershell
.\build\full-example\bin\ouif_hello.exe
```

The executable needs these files next to it:

- `libouif.dll`
- `ouif-shaders/dx11/vs_ouif_rect.bin`
- `ouif-shaders/dx11/fs_ouif_rect.bin`

The normal build places them there automatically.

## Test

```powershell
cmake --build build/full-example --target ouif_core_tests
ctest --test-dir build/full-example --output-on-failure
```

## Basic Application

```cpp
#include <OUIF/OUIF.h>

#include <memory>

class Panel : public ouif::Widget {
public:
    Panel()
    {
        ouif::Style style;
        style.background = ouif::Color::rgb(42, 92, 130);
        style.background_hovered = ouif::Color::rgb(58, 118, 160);
        style.border = ouif::Color::rgb(180, 218, 255);
        style.border_width = 2.0f;
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
    ouif::ApplicationConfig config;
    config.title = "Hello OUIF";
    config.width = 960;
    config.height = 540;

    ouif::Application app(config);
    app.set_root(std::make_unique<Panel>());
    return app.run();
}
```

## Existing Native Window

If your application already owns a native window, pass its platform handle and drive frames yourself:

```cpp
ouif::ApplicationConfig config;
config.width = 1280;
config.height = 720;
config.native_window = my_native_window_handle;
config.create_window = false;

ouif::Application app(config);
app.set_root(std::make_unique<MyWidget>());

while (running) {
    app.dispatch_event(next_event);
    app.frame();
}
```

You can also use `ouif::Renderer` directly if you want complete control over event routing and frame scheduling.
