#!/usr/bin/env sh
set -eu

prefix="${1:-$(pwd)/dist}"

cmake --preset release -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build --preset release
cmake --install build/release
