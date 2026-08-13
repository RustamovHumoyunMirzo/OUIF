param(
    [ValidateSet("dev", "release")]
    [string]$Preset = "dev"
)

$ErrorActionPreference = "Stop"

cmake --preset $Preset
cmake --build --preset $Preset
