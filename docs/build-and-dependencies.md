# Build And Dependencies

OUIF aims to make the user side simple:

- users include OUIF headers
- users link `OUIF::ouif`
- users do not configure bgfx directly
- OUIF builds and packages its renderer assets

## Dependency Layout

Dependencies are fetched into `external/`:

```text
external/
  bgfx.cmake/
    bgfx/
    bx/
    bimg/
  glfw/
  katana/
  pugixml/
  vg-renderer/
```

This mirrors what `bgfx.cmake` expects, but avoids recursive submodule fetches during CMake configure.

Fetch or update dependencies:

```powershell
.\scripts\fetch-deps.ps1
```

## Configure

```powershell
cmake -S . -B build/full-example -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
```

Useful options:

- `OUIF_BUILD_SHARED`: build OUIF as a shared library
- `OUIF_BUILD_EXAMPLES`: build examples
- `OUIF_BUILD_TESTS`: build tests
- `OUIF_FETCH_BGFX`: use `external/bgfx.cmake`
- `OUIF_FETCH_GLFW`: use `external/glfw`
- `OUIF_USE_KATANA`: build Katana CSS parser from `external/katana`
- `OUIF_USE_PUGIXML`: build pugixml XML parser from `external/pugixml`
- `OUIF_USE_VG_RENDERER`: build vg-renderer vector backend from `external/vg-renderer`
- `OUIF_INSTALL`: generate install targets
- `OUIF_DEPS_DIR`: dependency source directory

## Build

```powershell
cmake --build build/full-example --target ouif_hello
```

## Install

```powershell
cmake --install build/full-example --prefix dist/ouif
```

The install exports:

- `OUIF::ouif`
- `OUIFConfig.cmake`
- `OUIFConfigVersion.cmake`
- `OUIFFunctions.cmake`
- public headers
- runtime shader assets when built

## CMake Consumer API

```cmake
find_package(OUIF CONFIG REQUIRED)

ouif_add_app(my_app SOURCES main.cpp)
```

`ouif_add_app` creates an executable, links `OUIF::ouif`, and applies OUIF’s default C++20/warning settings.

Direct linking also works:

```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE OUIF::ouif)
```

## Runtime Files

On Windows shared builds, put these next to the user executable:

- `libouif.dll`
- `ouif-shaders/`

The example build already emits that layout under `build/full-example/bin`.

## Current Renderer Scope

The renderer currently supports:

- bgfx initialization
- resize
- frame begin/end
- filled rectangles
- stroked rectangles
- rounded rectangles and rounded borders
- text through font atlases
- image decoding through bimg and texture drawing through bgfx
- SVG/vector drawing through vg-renderer
- clipping and transforms
- internal render targets for shader-backed backdrop blur
- internally compiled shaders

SVG support includes static shapes, path commands, transforms, local symbols/use references, linear/radial gradients, clipPath/mask clipping, and Gaussian blur filter metadata routed through the vector renderer.
