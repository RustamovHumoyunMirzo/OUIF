#!/usr/bin/env sh
set -eu

preset="${1:-dev}"

cmake --preset "$preset"
cmake --build --preset "$preset"
