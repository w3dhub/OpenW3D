# OpenW3D BGFX Build Guide

## Quick Start (Windows)

### Option 1: Local Build Script
```powershell
# From the repo root:
.\Build-BGFX-Windows.ps1
```

### Option 2: Manual Steps
```powershell
# 1. Checkout the right branch
git checkout fix/review-pr3

# 2. Initialize submodules (CRITICAL)
git submodule update --init --recursive

# 3. Build bgfx
cd external\bgfx
..\..\bx\tools\bin\windows\genie.exe vs2022
msbuild .build\projects\vs2022\bgfx.sln /p:Configuration=Release /p:Platform=x64 /m
cd ..\..

# 4. Build OpenW3D with BGFX
mkdir build-bgfx && cd build-bgfx
cmake .. -GNinja -DENABLE_BGFX_BACKEND=ON -DWANT_DX9=OFF -DCMAKE_BUILD_TYPE=Release
ninja

# 5. Run
cd ..
.\build-bgfx\renegade.exe
```

## Prerequisites
- Visual Studio 2022 with C++ Desktop Development
- CMake 3.25+
- Ninja (recommended)
- Git

## CI
The `.github/workflows/openw3d-bgfx.yml` workflow runs on push to `fix/review-pr3` and provides downloadable artifacts.

## Troubleshooting
| Problem | Solution |
|---------|----------|
| Submodules empty | `git submodule update --init --recursive` |
| bgfx.lib not found | Build bgfx.sln first |
| shaderc not found | Copy from bgfx `.build/win64_vs2022/bin/` |
| Link errors | Ensure all submodules are initialized |

## Output
Build produces `renegade.exe` in the build directory. The game will use BGFX as the renderer when built with `-DENABLE_BGFX_BACKEND=ON`.
