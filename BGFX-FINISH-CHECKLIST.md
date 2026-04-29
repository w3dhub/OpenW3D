# BGFX Finish Checklist

Goal: finish the current BGFX implementation on the existing PR stack, with PR #122 as the active implementation branch.

## Definition of Done
- BGFX backend initializes cleanly on supported test setups
- Shader assets compile/load reliably
- Core textured + lit mesh rendering works
- Main render-state paths are wired and validated
- No critical stubs remain in active rendering paths
- Remaining failures are clearly out-of-scope or external to BGFX

---

## 1. Core Backend Stabilization

### 1.1 Texture binding and stage behavior
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- `Code/ww3d2/backends/bgfx/bgfxbackend.h`
- `Code/ww3d2/backends/bgfx/BGFXTexture.h`
- `Code/ww3d2/backends/bgfx/BGFXTexture.cpp`

**Functions / areas:**
- `BGFXBackend::Set_Texture(...)`
- `BGFXBackend::Set_Texture_Stage(...)`
- any per-stage state tracking added to `BGFXBackend`
- sampler flag mapping in `BGFXTexture` / `BGFXSamplerState`

**Tasks:**
- [x] Replace texture binding stub with actual sampler/uniform binding
- [x] Track per-stage bound texture + flags + mip level
- [x] Route `Set_Texture_Stage(...)` through actual texture binding path
- [ ] Add explicit stage enable/disable semantics
- [ ] Map stage 0/1 filtering/addressing behavior from ww3d state
- [ ] Validate UV source/index handling for multi-stage paths
- [ ] Verify behavior when texture handle is invalid/null

**Done when:**
- Stage 0 textured meshes render correctly
- Stage disable/replace behavior is deterministic
- Stage 1 does not silently corrupt output

### 1.2 Material uniform stability
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- `Code/ww3d2/backends/bgfx/bgfxbackend.h`
- `Code/ww3d2/backends/bgfx/BGFXMaterialMapper.h`
- shaders under `Code/ww3d2/backends/bgfx/shaders/source/`

**Functions / areas:**
- `BGFXBackend::Set_Material_Colors(...)`
- `BGFXBackend::Set_Material_Uniforms(...)`
- uniform creation/destruction in `Init()` / `Shutdown()`

**Tasks:**
- [x] Stop creating material uniforms every call
- [x] Create stable material uniforms during backend init
- [ ] Validate diffuse/specular/emissive/ambient/shininess layout against shaders
- [ ] Validate alpha/opacity semantics against real materials
- [ ] Ensure defaults are sane when data is absent

**Done when:**
- Material uniforms are stable across draws
- Opaque/alpha/emissive materials behave plausibly

### 1.3 Light environment upload
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- `Code/ww3d2/backends/bgfx/bgfxbackend.h`
- `Code/ww3d2/backends/bgfx/shaders/source/fs_mesh.sc`
- `Code/ww3d2/backends/bgfx/shaders/source/vs_mesh.sc`
- any related material/light mapper code

**Functions / areas:**
- `BGFXBackend::Set_Light_Environment(...)`
- shader uniform definitions for light arrays / ambient / camera position

**Tasks:**
- [x] Replace dead stub with at least a real lighting enable uniform
- [x] Inspect `LightEnvironmentClass` structure and call sites
- [x] Upload ambient light data
- [x] Upload active light count
- [x] Upload directional light data
- [ ] Upload point/spot data if used in main paths
- [x] Clamp to supported light count and document limit
- [x] Verify camera-position uniform is consistently set for specular lighting

**Done when:**
- Lit/unlit scenes differ correctly
- Ambient + diffuse + specular behave predictably

### 1.4 Platform lifecycle cleanup
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- `Code/ww3d2/backends/bgfx/bgfxbackend.h`

**Functions / areas:**
- `BGFXBackend::Init(...)`
- `BGFXBackend::Shutdown()`
- device reset/reinit paths

**Tasks:**
- [x] Keep ownership of Linux X11 display and close it on shutdown
- [ ] Audit resource recreation on reinit / resolution switch
- [ ] Verify framebuffer, textures, uniforms, and programs are destroyed safely
- [ ] Verify multiple init/shutdown cycles don’t leak or double-destroy

**Done when:**
- Backend can reinit cleanly without obvious resource lifetime bugs

---

## 2. Draw Submission Correctness

### 2.1 End-to-end draw path audit
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- `Code/ww3d2/backends/bgfx/BGFXMaterialMapper.cpp`
- `Code/ww3d2/backends/bgfx/BGFXMaterialMapper.h`

**Functions / areas:**
- `BGFXBackend::Draw(...)`
- `BGFXBackend::Draw_Indexed(...)`
- `BGFXBackend::Apply_Shader(...)`
- material pass helpers

**Tasks:**
- [x] Trace full draw submission path from ww3d state to bgfx submit
- [x] Verify vertex buffer/index buffer/program/state bindings are complete
- [x] Verify texture/sampler/uniform submission order
- [x] Verify material pass boundaries reset/retain the right state
- [ ] Identify stale-state hazards between draw calls

**Done when:**
- A draw call can be explained and debugged end-to-end

### 2.2 Render-state mapping audit
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- `Code/ww3d2/backends/bgfx/BGFXMaterialMapper.h`

**Functions / areas:**
- `FillMode_To_BGFX(...)`
- `CullMode_To_BGFX(...)`
- blend/stencil/depth mapping helpers
- `Apply_Render_State(...)`
- `Set_DX8_Render_State(...)`

**Tasks:**
- [x] Fix `SOLID` fill mode not forcing triangle-strip mode
- [ ] Audit primitive topology assumptions in draw paths
- [ ] Verify cull winding matches existing backend expectations
- [ ] Verify blend modes used by common materials
- [ ] Verify depth test/write behavior
- [ ] Verify stencil state mapping
- [ ] Verify fog-related state handling

**Done when:**
- No obviously wrong core-state mappings remain

### 2.3 View/framebuffer/scissor correctness
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- framebuffer helpers and related shader paths if needed

**Functions / areas:**
- `Set_Viewport(...)`
- `Enable_Scissor(...)`
- `Set_Scissor(...)`
- `Set_FrameBuffer(...)`
- `Create_FrameBuffer(...)`

**Tasks:**
- [ ] Validate viewport depth range assumptions
- [ ] Verify scissor rect behavior against viewport changes
- [ ] Verify default framebuffer restore path
- [ ] Validate render-target attach/detach behavior
- [ ] Validate width/height tracking when render targets switch

**Done when:**
- Offscreen and onscreen rendering state transitions behave predictably

---

## 3. Shader Pipeline Hardening

### 3.1 Build-time shader compilation
**Files:**
- `Code/ww3d2/backends/bgfx/CMakeLists.txt`
- shader sources under `Code/ww3d2/backends/bgfx/shaders/`

**Tasks:**
- [x] Fix shaderc discovery to allow `shadercRelease`
- [x] Add missing `varying.def.sc`
- [x] Wire shader varying definition into shader build command
- [x] Remove currently failing profile target(s) from active local build path
- [ ] Review whether Linux-only profile selection is enough for repo use
- [ ] Decide whether Windows-specific shader profiles need explicit build entries
- [ ] Make shader build output layout predictable/documented

**Done when:**
- Fresh configure + shader build works repeatably on the active dev setup

### 3.2 Runtime shader/program loading
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- embedded lookup files if used

**Functions / areas:**
- `Load_Shader(...)`
- `Create_Program(...)`
- embedded shader lookup path

**Tasks:**
- [ ] Audit failure behavior when shader binaries are missing
- [ ] Make shader load errors explicit and debuggable
- [ ] Verify embedded-vs-disk loading rules
- [ ] Remove or avoid any hidden runtime compile shelling if it still exists
- [ ] Ensure program creation failures do not silently continue

**Done when:**
- Shader/program failures are explicit and safe

---

## 4. Validation Against Real Content

### 4.1 Focused render test matrix
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- any test hooks used by render test path

**Tasks:**
- [ ] Keep/expand simple triangle test for backend sanity
- [ ] Add or use a textured mesh sanity case
- [ ] Add or use a lit mesh sanity case
- [ ] Add alpha-blend sanity case
- [ ] Add fog sanity case
- [ ] Add render-target sanity case if needed

**Done when:**
- Multiple isolated rendering features can be verified without launching the full game blindly

### 4.2 Real-scene/game validation
**Tasks:**
- [ ] Test with imported game data on target platform
- [ ] Compare against DX8 backend behavior
- [ ] Check missing textures / wrong UVs / wrong culling / wrong lighting
- [ ] Check UI/video/special-material regressions if they route through ww3d
- [ ] Capture a short known-bad / known-good issue list

**Done when:**
- Representative real scenes render plausibly enough to continue development on BGFX

---

## 5. Persistence / Device Management

### 5.1 Render-device persistence
**Files:**
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp`
- config/persistence code if shared elsewhere

**Functions / areas:**
- `Registry_Save_Render_Device(...)`
- `Registry_Load_Render_Device(...)`
- resolution/device selection paths

**Tasks:**
- [ ] Replace fake-success stubs with real behavior or explicit not-supported behavior
- [ ] Verify backend/device selection persists correctly
- [ ] Verify resolution/windowed state restore path

**Done when:**
- Restart/config flows do not lie about success

### 5.2 Device switching/reset
**Tasks:**
- [ ] Validate `Set_Any_Render_Device()` / `Set_Render_Device()` / `Toggle_Windowed()` flows
- [ ] Verify resource recreation on renderer switch
- [ ] Verify swap interval path after reinit

**Done when:**
- Common device changes do not leave the backend half-initialized

---

## 6. Review / Cleanup Pass

### 6.1 Remove misleading comments and dead paths
**Tasks:**
- [ ] Delete stale “stubbed out” comments where code now exists
- [ ] Add narrow comments where bgfx behavior is non-obvious
- [ ] Remove dead temporary code that no longer serves validation

### 6.2 Document known limitations honestly
**Tasks:**
- [ ] List supported light count / supported shader profiles
- [ ] List any incomplete texture combiner behavior
- [ ] Note external Linux build blockers outside BGFX
- [ ] Note any platform support caveats

### 6.3 Reviewer-facing PR summary
**Tasks:**
- [ ] Summarize what works now
- [ ] Summarize how it was verified
- [ ] Call out what is intentionally deferred

---

## Immediate execution order
1. ✅ `Set_Light_Environment(...)` real data upload
2. ✅ Draw-path audit in `Draw_Indexed(...)` — backend dispatch seam implemented
3. ✅ Texture-stage state mapping beyond simple binding — real texture bridging with format conversion
4. ✅ Render-state verification (blend/depth/cull/fog) — fill mode merged into shader state
5. Persistence/device stub cleanup
6. Real-scene validation and bug list
7. ✅ Matrix convention validation (row-major W3D → column-major bgfx)
8. ✅ Individual DX8 light wiring for sorting renderer path
9. ✅ Compressed texture support (DXT1/3/5)
10. ✅ Camera position uniform extraction from view matrix

## What is already improved on the branch
- PR #122 was restacked cleanly on top of PR #121
- Backend dispatch seam in `DX8Wrapper` routes all deferred state to BGFXBackend when active
- Full backend dispatch overrides implemented (shader, texture, material, lights, transforms, buffers, draw)
- Real texture bridging with WW3DFormat → bgfx::TextureFormat mapping and surface locking
- Dynamic buffer support via transient vertex/index buffers
- Full light environment upload (ambient + up to 4 directional lights) with updated `fs_mesh.sc`
- Backend dispatch for `Draw_Sorting_IB_VB` (sorting buffer path)
- Material uniforms are stable instead of recreated every call
- `SOLID` fill mode no longer maps to strip topology
- X11 display ownership is at least tracked/closed
- Shader build path was repaired enough to compile current shader targets locally
