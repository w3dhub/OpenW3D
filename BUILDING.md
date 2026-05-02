# Building OpenW3D with CMake

OpenW3D is still a Windows-first codebase. The CMake files can now be used for repeatable configure smoke tests on Linux, but a full native Linux compile still needs source portability work around Win32 and Direct3D types.

## Prerequisites

Common tools:

```bash
cmake --version   # 3.25+
ninja --version
git submodule update --init --recursive
```

Useful Ubuntu packages for configure smoke tests:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build pkg-config \
  libicu-dev libx11-dev libxrandr-dev libxcursor-dev libgl-dev
```

The BGFX preset expects the `external/bgfx`, `external/bimg`, and `external/bx` submodules to exist, and expects prebuilt bgfx libraries under the usual bgfx `.build/.../bin` directory.

## Windows build preset

```bash
cmake --preset win
cmake --build --preset win
```

## Linux configure smoke tests

These presets are intended to make CMake/build-system work easy to reproduce on a Linux host. They intentionally disable legacy DX9/DXVK and SDL3 by default because those are not required for a native CMake smoke test.

```bash
cmake --preset linux-minimal
cmake --preset linux-bgfx
```

To attempt the `ww3d2` target after configuring:

```bash
cmake --build --preset linux-minimal-ww3d2
cmake --build --preset linux-bgfx-ww3d2
```

## Current Linux build status

Known configure status:

- `linux-minimal` configures.
- `linux-bgfx` configures when BGFX libraries are already built.

Known compile blockers after configure:

- `Code/ww3d2/dx8wrapper.h` includes `<d3d9.h>` through shared renderer-facing headers even when `WANT_DX9=OFF`.
- `Code/ww3d2/rddesc.h` exposes `d3d9types.h`-based types to non-DX builds.
- Several shared modules still include Win32-only headers such as `<windows.h>`, `<direct.h>`, and COM headers.
- `wwlib` and `wwutil` have Win32-only thread/process/resource abstractions that need platform seams before full native Linux builds can pass.

Treat those as source/API-boundary tasks, not CMake package-discovery issues. Do not paper over them by globally adding Wine or MinGW headers to a native Linux build.

## Useful manual commands

Equivalent to the `linux-bgfx` preset:

```bash
cmake -S . -B build/linux-bgfx -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DENABLE_BGFX_BACKEND=ON \
  -DWANT_DX9=OFF \
  -DW3D_BUILD_OPTION_SDL3=OFF \
  -DW3D_CLIENT=OFF \
  -DW3D_FDS=OFF \
  -DW3D_TOOLS=OFF
```

List generated targets:

```bash
cmake --build build/linux-bgfx --target help
```
