# Recovered BGFX / SDL3 Context

Recovered on 2026-04-30 from local branches in `Dadud/OpenW3D` and draft PRs on `w3dhub/OpenW3D`.

## Executive summary

The BGFX migration was being intentionally landed as a staged chain:

1. **Backend abstraction**
2. **BGFX backend structure**
3. **BGFX rendering implementation**
4. **Post-review local fix pass**
5. **Optional follow-up SDL3 work** (gamepad, window)

The clean architectural story is preserved in draft PRs **#120, #121, #122**.
The latest local BGFX code is **not** the PR branch; it is **`fix/review-pr3`** (same tip as `pr-3-bgfx-rendering-fixed`).

---

## Branch → PR mapping

### Early precursor branches / PRs
- `pr/render-device-caps` → PR **#114**
  - `Add WW3DBackend pure-virtual interface with BackendDeviceCapabilities`
- `pr/render-device-enum` → PR **#115**
  - `Add DX8Backend implementing WW3DBackend interface`
- `bgfx-integration` → PR **#116** (closed draft)
  - `Add bgfx Rendering Backend`
  - First mixed prototype branch; later split into the clean chain below.

### Clean BGFX chain
- `pr-1-backend-abstraction` → PR **#120** (open draft)
  - `Backend Abstraction + NullBackend`
- `pr-2-bgfx-backend` → PR **#121** (open draft)
  - `BGFX Backend Support (Structure)`
- `pr-3-bgfx-rendering-clean` → PR **#122** (open draft)
  - `BGFX Rendering Implementation`

### Local continuation after draft PRs
- `pr-3-bgfx-rendering-fixed` → local only
- `fix/review-pr3` → local only
  - both point at commit `40e53ff`
  - latest BGFX fixup pass after PR #122 was opened

### SDL3 branches
- `sdl3-window` → PR **#123** (closed draft)
  - narrow SDL3 window creation experiment
- `feature/sdl3-gamepad` → **no PR draft found**
  - local-only follow-up branch on top of the BGFX work

---

## What each PR/branch meant

### PR #120 / `pr-1-backend-abstraction`
Purpose: create the seam needed to swap renderers.

Main contents:
- `WW3DBackend`
- `NullBackend`
- `DX8Backend`
- `ww3d.cpp` backend selection refactor

Interpretation:
- This is the architectural prerequisite for BGFX.
- The engine was being decoupled from direct DX8/DX9 assumptions.

### PR #121 / `pr-2-bgfx-backend`
Purpose: add BGFX as a structured backend target.

Main contents:
- `Code/ww3d2/backends/bgfx/...`
- `cmake/bgfx.cmake`
- `.gitmodules` for `bgfx`, `bx`, `bimg`
- backend selection wiring

Interpretation:
- This was meant to land BGFX infrastructure first.
- Rendering correctness was intentionally deferred.

### PR #122 / `pr-3-bgfx-rendering-clean`
Purpose: actually map WW3D’s DX8-style rendering behavior onto BGFX.

Main contents:
- vertex/index buffer wrappers
- texture upload and sampler mapping
- material + lighting handling
- viewport/scissor/framebuffer support
- shader scaffolding
- DX8 wrapper dispatch seam
- `BGFX-FINISH-CHECKLIST.md`

Interpretation:
- This is the real implementation branch.
- The design goal was explicitly **backend swap first, programmable shader flexibility later**.

### `fix/review-pr3` / `pr-3-bgfx-rendering-fixed`
Purpose: fix issues found after the draft renderer PR was assembled.

Single extra commit beyond PR #122:
- `40e53ff review(bgfx): fix stubs, ownership, shaders, and style issues`

Interpretation:
- This is the latest meaningful BGFX state.
- It is a cleanup/stabilization pass, not a new architecture shift.

### `feature/sdl3-gamepad`
Purpose: SDL3 input modernization after or alongside BGFX work.

Interpretation:
- This appears to be next-phase work stacked on the renderer modernization effort.
- It was never posted as a PR draft upstream.

### PR #123 / `sdl3-window`
Purpose: replace Win32 window creation with SDL3 when `OPENW3D_SDL3` is enabled.

Interpretation:
- Small, isolated experiment branch.
- Best treated as separate from the core BGFX stabilization effort.

---

## Why PR #116 was abandoned

PR **#116** (`bgfx-integration`) was the original mixed prototype.
It was closed after review and explicitly replaced by the clean PR chain.

Recovered reasoning from comments:
- the branch mixed abstraction, build fixes, backend wiring, and rendering work
- reviewers said it had too many blockers and too many concerns at once
- the response was to split it into focused, reviewable stages

So the staged PR chain was a deliberate reaction to review feedback, not random churn.

---

## Review feedback that shaped the split

PR #116 got a detailed sanity review noting issues such as:
- `DX8Backend` not fully implementing `WW3DBackend`
- DX9 CMake path selecting the wrong wrapper without the expected define
- BGFX device enumeration/state selection issues
- draw path not functionally rendering yet
- incomplete Vulkan/Linux path
- backend option/selection ambiguity

Net effect:
- land abstraction first
- then land BGFX structure
- then land rendering implementation

---

## Post-draft changes: PR #122 → `fix/review-pr3`

Diff target:
- from `pr-3-bgfx-rendering-clean` (`79fde36`)
- to `fix/review-pr3` (`40e53ff`)

Only one additional commit exists, but it touches 23 files.
Net diff:
- **211 insertions**
- **484 deletions**

### Main changes made after PR #122

#### 1. Real draw submission fix
- `BGFXBackend::Draw()` and `Draw_Indexed()` stop submitting `BGFX_INVALID_HANDLE`
- they now submit `m_currentProgram`

Why it matters:
- this is a major functional fix; PR #122 snapshot still had a critical rendering-path stub here.

#### 2. Correct vertex/index buffer memory ownership
- vertex buffer creation changed from `bgfx::makeRef(...)` of only `layout.getStride()` bytes
  to copying the **full vertexCount * stride** payload
- index buffer creation changed from `makeRef(...)` to `copy(...)`
- `Create_Vertex_Buffer(...)` signature now takes `vertexCount`

Why it matters:
- fixes under-copy / lifetime bugs in GPU buffer upload
- strongly suggests the earlier branch still had invalid/stale memory semantics

#### 3. Texture upload sizing fixed for compressed formats
- `BGFXTexture` now computes total texture data size correctly
- explicit handling added for BC1/BC2/BC3 block-compressed formats

Why it matters:
- compressed texture support existed in the clean branch, but this makes the upload math trustworthy

#### 4. White fallback texture ownership moved into backend state
- static local placeholder texture replaced with member `m_whiteTexture`
- destroyed cleanly in `Shutdown()`

Why it matters:
- avoids process-lifetime static state and makes backend shutdown/reinit cleaner

#### 5. Resource cache clearing added
- new `Clear_Resource_Caches()`
- destroys cached vertex/index/texture handles
- called during shutdown / renderer switching paths

Why it matters:
- addresses ownership / reinit issues
- not a perfect lifetime solution yet; header note still warns about stale pointer-keyed caches

#### 6. Device switching hardened
- `Set_Next_Render_Device()` now guards against zero render devices before modulo
- backend switching clears caches before shutdown/reinit

Why it matters:
- fixes obvious crash/undefined behavior path

#### 7. Registry save/load stubs made explicitly unsupported
- functions changed from returning `true` to returning `false`

Why it matters:
- removes false-success behavior from unimplemented paths

#### 8. Color packing corrected
- clear color conversion changed from `Color_To_BGRA` to `Color_To_ABGR`

Why it matters:
- likely fixes color channel ordering issues in bgfx clear/setup paths

#### 9. Shader build fixes
- shader profile typo fixed: `" spirv"` → `"spirv"`
- varying definition added as explicit dependency
- output naming simplified/fixed

Why it matters:
- shader pipeline reliability improved
- this aligns with the commit message mention of shader fixes

#### 10. Linux / bgfx CMake integration cleanup
- bgfx consumer standard loosened from forced C++20 to C++17 minimum
- OpenGL made optional on Linux in bgfx linkage path
- stub bgfx static library creation improved so the linker gets a minimally valid archive
- Windows null-backend fallback warning added when no backend is enabled

Why it matters:
- this was a build-system stabilization pass, not just render code cleanup

#### 11. Screenshot / movie capture hardened
- front-buffer access now casts carefully and bails if unsupported

Why it matters:
- avoids assuming every backend exposes a DX8-style front buffer

#### 12. Primitive-type debug diagnostics added in DX8 wrapper bridge
- explicit warnings for unsupported primitive types in BGFX path

Why it matters:
- useful for tracking remaining rendering correctness gaps

#### 13. Material mapper category tweak
- additive/sort-category logic adjusted in `BGFXMaterialMapper`

Why it matters:
- likely part of sorting/blend correctness cleanup

#### 14. General cleanup / style simplification
- many file headers replaced with concise OpenW3D backend comments
- a lot of churn is cleanup, but the functional changes above are the important part

---

## Best current source-of-truth

### For intent and architecture
Use the PRs:
- **#120** abstraction
- **#121** bgfx structure
- **#122** renderer implementation

### For latest BGFX code
Use local branch:
- **`fix/review-pr3`**

### For next-phase optional work
- **`feature/sdl3-gamepad`**
- **`sdl3-window`**

---

## Recommended resume strategy

1. **Resume from `fix/review-pr3`**, not `pr-3-bgfx-rendering-clean`
2. Treat **PR #120 → #121 → #122** as the narrative/history for why the code is shaped this way
3. Keep **SDL3 gamepad/windowing** separate until BGFX stabilization is under control
4. Use `BGFX-FINISH-CHECKLIST.md` as the outstanding-work list
5. Expect remaining work to be mostly:
   - render-state correctness
   - resource lifetime/invalidation
   - shader/runtime loading robustness
   - real-content validation
   - platform build verification

---

## Current practical conclusion

The project did **not** lose its direction.
The direction is recoverable and clear:

- abstract the renderer
- land BGFX as the new backend
- preserve the fixed-function WW3D behavior first
- modernize SDL platform/input separately

The main thing lost was conversational continuity, not the implementation story.