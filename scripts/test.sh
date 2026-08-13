#!/usr/bin/env sh
set -eu

preset="${1:-dev}"

cmake --build --preset "$preset"
ctest --preset "$preset"
