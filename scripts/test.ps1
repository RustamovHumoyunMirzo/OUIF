param(
    [ValidateSet("dev")]
    [string]$Preset = "dev"
)

$ErrorActionPreference = "Stop"

cmake --build --preset $Preset
ctest --preset $Preset
