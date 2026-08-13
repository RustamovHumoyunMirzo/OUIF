#!/usr/bin/env sh
set -eu

deps_dir="${1:-$(pwd)/external}"
bgfx_cmake="$deps_dir/bgfx.cmake"

sync_repo() {
    url="$1"
    path="$2"

    if [ -d "$path/.git" ]; then
        git -C "$path" pull --ff-only
    else
        rm -rf "$path"
        git clone --depth 1 "$url" "$path"
    fi
}

mkdir -p "$deps_dir"

sync_repo "https://github.com/bkaradzic/bgfx.cmake.git" "$bgfx_cmake"
sync_repo "https://github.com/bkaradzic/bgfx.git" "$bgfx_cmake/bgfx"
sync_repo "https://github.com/bkaradzic/bx.git" "$bgfx_cmake/bx"
sync_repo "https://github.com/bkaradzic/bimg.git" "$bgfx_cmake/bimg"
sync_repo "https://github.com/glfw/glfw.git" "$deps_dir/glfw"

printf 'OUIF dependencies are ready in %s\n' "$bgfx_cmake"
