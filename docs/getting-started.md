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
- `katana-parser`

This explicit layout avoids slow or fragile recursive submodule fetching during CMake configure.

## Build The Example

```powershell
cmake -S . -B build/full-example -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/full-example --target ouif_hello
cmake --build build/full-example --target ouif_complex_layout
cmake --build build/full-example --target ouif_css_style
```

Run:

```powershell
.\build\full-example\bin\ouif_hello.exe
```

```powershell
.\build\full-example\bin\ouif_complex_layout.exe
```

```powershell
.\build\full-example\bin\ouif_css_style.exe
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

class Panel : public ouif::Widget {
public:
    Panel()
    {
        set_size({ 160.0f, 120.0f });
        set_style(ouif::Style {
            .background = "#2a5c82",
            .hovered = "#3a76a0",
            .selected = "#3a76a0",
            .focused = "#223148",
            .border = { "#b4daff", 2.0f },
            .border_selected = { "#b4daff", 4.0f },
            .border_focused = { "#83b7ff", 2.0f },
            .radius = 8.0f,
        });
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
    ouif::Application app(ouif::ApplicationConfig()
        .with_title("Hello OUIF")
        .with_size(960, 540));
    Panel panel;
    app.set_root(panel);
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

## Ownership Rules

For the easiest fully-owned tree, let OUIF construct widgets:

```cpp
auto& tile = row.add_child<ColorTile>("#2f6c9c", "#4692c4", ouif::Size { 160.0f, 120.0f });
app.set_root<DemoSurface>();
```

For Qt-like member widgets, keep children as members and register them:

```cpp
class DemoSurface : public ouif::RowLayout {
public:
    DemoSurface()
    {
        children(blue_, green_);
    }

private:
    ColorTile blue_;
    ColorTile green_;
};
```

External/member children auto-detach when they are destroyed. OUIF-owned children are deleted by their parent. Widgets are non-movable so registered addresses stay valid.

## CSS Styling

`set_style()` remains the simplest C++ styling path. For stylesheet-driven UI, call `set_stylesheet()` on a widget tree:

```cpp
row.set_stylesheet(R"css(
    .tile {
        background: #2f6c9c;
        background-hovered: #4692c4;
        width: 50%;
        height: 120px;
        border-radius: 10px;
        border-left: #9fd7ff 8px;
        border-top: #e8edf3 2px;
        border-right: #1c3f5c 4px;
        border-bottom: #102838 6px;
    }

    .tile:selected {
        background: #4692c4;
        border: #e8edf3 4px;
    }
)css");
```

Use `add_class("tile")`, `set_name("primary")`, and `set_type_name("Panel")` for selectors. Supported state selectors include `:hover`, `:active`, `:selected`, `:checked`, and `:focus`.
