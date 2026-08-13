param(
    [string]$Prefix = "$PSScriptRoot/../dist"
)

$ErrorActionPreference = "Stop"

cmake --preset release -DCMAKE_INSTALL_PREFIX="$Prefix"
cmake --build --preset release
cmake --install build/release
