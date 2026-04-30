# Local CI Replica for BGFX Builds
# Run on Windows with: pwsh ./local-ci-bgfx.ps1

$ErrorActionPreference = "Stop"

Write-Host "=== Local CI: OpenW3D + BGFX ===" -ForegroundColor Cyan

# Replicate CI environment
$env:W3D_COMPILER_FLAGS = ""

# CI does: actions/checkout@v6 (already have repo)
# CI does: ilammy/msvc-dev-cmd@v1.13.0 (need VS dev env)
$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    Write-Host "ERROR: Visual Studio with C++ tools not found" -ForegroundColor Red
    exit 1
}

# Load MSVC environment
$devCmd = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
$tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
"@echo off
call `"$devCmd`"
set > `"$tempBat.env`"" | Out-File -Encoding ASCII $tempBat
& $tempBat
$envContent = Get-Content "$tempBat.env"
foreach ($line in $envContent) {
    if ($line -match '^([^=]+)=(.*)$') {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}
Remove-Item $tempBat, "$tempBat.env" -Force

Write-Host "[OK] MSVC environment loaded" -ForegroundColor Green

# CI does: cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release
Write-Host "`n=== CMake Configure ===" -ForegroundColor Cyan
$buildDir = "build-ci-bgfx"
if (Test-Path $buildDir) {
    Remove-Item $buildDir -Recurse -Force
}

cmake -S . -B $buildDir `
    -GNinja `
    -DCMAKE_BUILD_TYPE=Release `
    -DENABLE_BGFX_BACKEND=ON `
    -DWANT_DX9=OFF `
    -DW3D_BUILD_OPTION_SDL3=OFF `
    -DW3D_BUILD_OPTION_WEBBROWSER=ON `
    -DW3D_BUILD_OPTION_BINK=ON `
    -DW3D_BUILD_OPTION_MILES=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Configure failed" -ForegroundColor Red
    exit 1
}

# CI does: cmake --build build --parallel
Write-Host "`n=== CMake Build ===" -ForegroundColor Cyan
cmake --build $buildDir --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== BUILD SUCCESS ===" -ForegroundColor Green
Write-Host "Binaries in: $(Resolve-Path $buildDir)" -ForegroundColor Cyan

# Find the main executable
$renegade = Get-ChildItem $buildDir -Filter "renegade.exe" -Recurse | Select-Object -First 1
if ($renegade) {
    Write-Host "`nLaunch with: $(Resolve-Path $renegade.FullName)" -ForegroundColor Yellow
}
