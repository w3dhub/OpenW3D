# BOOTSTRAP - OpenW3D BGFX Backend

## Goal
Add BGFX rendering backend to https://github.com/W3DHub/OpenW3D

## Context
- This is a fork of the OpenW3D game engine (Command & Conquer Renegade)
- Currently uses DirectX8/Direct3D9 rendering on Windows
- Goal is to add bgfx (https://github.com/bkaradzic/bgfx) as a cross-platform rendering backend
- bgfx supports: D3D9/11/12, Vulkan, Metal, OpenGL, OpenGLES, WebGPU

## Starting Point
1. Clone fresh from: https://github.com/w3dhub/OpenW3D (upstream)
2. OR if using fork, ensure it's synced to upstream main first
3. Branch from: origin/main (upstream's main)
4. New branch name: bgfx-backend

## Approach
1. Explore codebase to understand current rendering architecture
2. Document findings in PLAN.md
3. Add bgfx backend with minimal changes
4. Keep CI passing

## Previous Attempt Notes (ignore these)
Previous session got tangled with 21 commits including unrelated "Linux compile fixes".
The right approach is minimal - only add what's needed for bgfx.

## Key Files to Understand
- Code/ww3d2/ - Main 3D rendering code
- Code/ww3d2/dx8wrapper.* - DX8 wrapper/helper classes  
- Code/ww3d2/dx8renderer.* - DX8 renderer implementation
- CMakeLists.txt - Build system
- cmake/ - CMake modules

## DO
- Start completely fresh
- Clone the repo
- Explore and understand first
- Document in PLAN.md
- Implement minimal changes only

## DON'T
- Copy old code from previous sessions
- Add Linux compile fixes (that's a separate concern)
- Make unnecessary changes
