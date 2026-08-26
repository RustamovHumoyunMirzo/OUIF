param(
    [string]$DepsDir = ""
)

$ErrorActionPreference = "Stop"

function Sync-Repo {
    param(
        [string]$Url,
        [string]$Path
    )

    if (Test-Path "$Path/.git") {
        git -C $Path pull --ff-only
        return
    }

    if (Test-Path $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
    }

    git clone --depth 1 $Url $Path
}

if ([string]::IsNullOrWhiteSpace($DepsDir)) {
    $Deps = Join-Path (Resolve-Path "$PSScriptRoot/..") "external"
} else {
    $Deps = $DepsDir
}
$BgfxCMake = Join-Path $Deps "bgfx.cmake"

New-Item -ItemType Directory -Force -Path $Deps | Out-Null

Sync-Repo "https://github.com/bkaradzic/bgfx.cmake.git" $BgfxCMake
Sync-Repo "https://github.com/bkaradzic/bgfx.git" (Join-Path $BgfxCMake "bgfx")
Sync-Repo "https://github.com/bkaradzic/bx.git" (Join-Path $BgfxCMake "bx")
Sync-Repo "https://github.com/bkaradzic/bimg.git" (Join-Path $BgfxCMake "bimg")
Sync-Repo "https://github.com/glfw/glfw.git" (Join-Path $Deps "glfw")
Sync-Repo "https://github.com/hackers-painters/katana-parser.git" (Join-Path $Deps "katana")
Sync-Repo "https://github.com/zeux/pugixml.git" (Join-Path $Deps "pugixml")
Sync-Repo "https://github.com/RudjiGames/vg_renderer.git" (Join-Path $Deps "vg-renderer")

Write-Host "OUIF dependencies are ready in $Deps"
