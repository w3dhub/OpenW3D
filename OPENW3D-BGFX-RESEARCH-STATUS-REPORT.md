# OpenW3D BGFX Research + Status Report

Prepared on 2026-04-30 for the Dadud/OpenW3D BGFX prototype effort.

Scope:
- research the upstream `w3dhub/OpenW3D` mainline
- reconstruct what changed since EA published the original archived Renegade source
- evaluate the current BGFX prototype branches
- explain how BGFX should properly slot into the modernized codebase
- summarize project status, progress, and likely direction

---

# Executive summary

## Short answer
OpenW3D is **not just the EA archive anymore**.
It is now a **substantially modernized portability branch of the Renegade codebase** with:
- CMake
- CI
- modern compiler support
- partial Linux/MinGW support
- DX9/DXVK groundwork
- SDL3/platform abstractions
- config modernization
- Unicode/ICU support
- FFmpeg/Bink replacement path
- ongoing warnings/64-bit cleanup

Your BGFX work is therefore **not being inserted into the raw EA archive**.
It is being inserted into a codebase that has already started migrating toward:
- cross-platform builds
- portable abstractions
- fewer Windows-only assumptions
- backend seams in multiple subsystems

That is good news, because it means the codebase already wants this direction.

## Main conclusion
The current BGFX prototype is **architecturally halfway right**:
- the top-level backend abstraction is a good move
- using `DX8Wrapper` as the short-term interception point is a pragmatic bootstrap

But the prototype is **still fighting the legacy design** because BGFX is being forced to act too much like a fake DX8/DX9 device.

The clean long-term path is:
1. keep `WW3DBackend` as the device/backend selection seam
2. move rendering semantics into a **typed ww3d render-state/material-pass translation layer**
3. let BGFX consume that translation layer as an independent backend
4. stop depending on DX9-owned resources as BGFX’s source of truth

---

# 1. Upstream lineage: from EA archive to OpenW3D mainline

## Likely original archive point
The practical EA import lineage in this repo is:
- `58ed459` — `LFeenanEA`: **Initial commit of Command & Conquer Renegade source code**
- followed by EA-authored README/license/bootstrap cleanup commits

That early state was effectively the archival drop:
- preservation-oriented
- no contribution workflow
- legacy VC6/Win32 assumptions
- dependency on many proprietary SDKs
- no real cross-platform story

## Since EA’s import
There are **297 commits after the EA source import** on `upstream/main`.

That means the current upstream is already a significant post-archive engineering effort, not just a repackaged source dump.

---

# 2. What upstream main now has that the EA archive did not

## Build system and CI
The biggest transformation is build infrastructure.

### Added since archive
- full **CMake-based build system**
- feature toggles for:
  - client
  - FDS/headless
  - tools
  - SDL3
  - webbrowser
  - FFmpeg
  - Bink
  - Miles
  - ICU
- modular `cmake/` helpers
- `vcpkg.json`
- GitHub Actions CI matrix

### Current CI matrix
From `.github/workflows/openw3d.yml`, upstream CI currently builds across:
- **MSVC x86**
- **MSVC x64**
- **MinGW x86**
- **MinGW x64**
- **MinGW x64 SDL3**
- **Linux x64** partial targets

This is extremely different from the archive-era “open the VC6 workspace” expectation.

## Platform portability work
Upstream has already done a large amount of portability cleanup:
- case-sensitive path/include fixes
- modern filesystem/path handling
- standard integer/pointer-width cleanup
- x64 compatibility fixes
- inline asm reduction/guarding
- `TCHAR`/wstring cleanup
- portable process/thread abstractions
- socket wrappers / network portability
- INI-based config replacing registry dependence
- Windows dialog/resource de-coupling into C structs

## Rendering-related modernization already present
Upstream main is still fundamentally on the old ww3d/DX wrapper model, but it already includes important steps toward modernization:
- the practical rendering layer is **D3D9-based**, not frozen in pure DX8-era tooling assumptions
- **DXVK groundwork** exists for Linux/native D3D9 compatibility flow
- **SDL3** exists as an option and appears in CI/build paths
- the codebase is already accepting platform abstraction changes that reduce Win32 lock-in

## Middleware modernization
Major additions beyond the archive include:
- **FFmpeg-based Bink replacement path**
- **WWAudio backend abstraction**
- **ICU UTF-16/Unicode support**

This matters because it shows the project’s direction is already:
- abstract old proprietary dependencies when practical
- make runtime features portable
- move subsystem boundaries toward backend-like seams

---

# 3. Major upstream milestones since the archive

## Phase A: Make the archive buildable in a modern world
High-level themes:
- CMake introduced
- repo cleanup
- initial CI
- drop VC6 as a practical target
- move toward modern compilers

## Phase B: Make it compile on more than one toolchain/platform
High-level themes:
- warning cleanup
- x64 cleanup
- MinGW fixes
- Linux partial build support
- path/casing cleanup
- networking/process/thread portability

## Phase C: Make runtime assumptions less Windows-bound
High-level themes:
- config moved from registry to INI/openw3d.conf
- UI dialog specs moved out of `.rc`
- SDL3/platform abstractions introduced
- DXVK native groundwork

## Phase D: Start unlocking subsystem replacement
High-level themes:
- WWAudio backend abstraction
- FFmpeg-based replacement work
- ICU/Unicode support
- backend-style seams becoming acceptable in the codebase

This is why the BGFX effort fits conceptually: it is not the first modernization seam in the repo.

---

# 4. Current BGFX branch stack and status

## Practical branch stack
Your BGFX work reconstructs as:
1. `pr-1-backend-abstraction`
2. `pr-2-bgfx-backend`
3. `pr-3-bgfx-rendering-clean`
4. `fix/review-pr3` / `pr-3-bgfx-rendering-fixed`

## What these mean

### PR 1 — backend abstraction
Adds:
- `WW3DBackend`
- `DX8Backend`
- `NullBackend`
- backend selection plumbing

This was the right first step.

### PR 2 — BGFX structure
Adds:
- BGFX backend class
- submodules (`bgfx`, `bx`, `bimg`)
- CMake integration
- build-time structure

This is infrastructure, not full renderer correctness.

### PR 3 — BGFX rendering implementation
Adds the actual prototype render path:
- texture bridge
- vertex/index buffer wrappers
- shader scaffolding
- material/light state mapping
- backend dispatch seams
- framebuffer/scissor helpers
- fixed-function emulation attempts

### Post-review fix branch
`fix/review-pr3` is the latest meaningful local tip.
It improves:
- real draw submission
- buffer upload ownership/copy semantics
- compressed texture size handling
- shader build details
- resource cache cleanup
- front-buffer safety guards
- device switching safety

## Current prototype status in one sentence
The BGFX path is **well beyond a toy skeleton**, but it is **not yet validated enough to be considered cleanly integrated**.

---

# 5. How BGFX should properly slot into OpenW3D

## The key architectural question
Should BGFX be inserted as:
1. a fake DX8/DX9 device behind the existing wrapper, or
2. a true peer backend fed by backend-neutral ww3d render semantics?

## Recommended answer
**BGFX should be a peer backend at the `WW3DBackend` level, but it should consume a backend-neutral render-state/material-pass translation layer, not a growing pile of DX8 one-off calls.**

## What is currently right
The prototype already made the correct first move:
- add `WW3DBackend`
- select backend at `WW3D`
- use `DX8Wrapper` as the short-term collection/interception point

That is a reasonable bootstrap path.

## What is currently wrong long-term
The current prototype still makes BGFX behave too much like a fake D3D-style endpoint by asking it to implement things like:
- `Set_DX8_Render_State(...)`
- `_Get_DX8_Front_Buffer()`
- DX8-style lighting hooks
- DX9-resource-derived texture upload behavior

That makes BGFX depend on assumptions that upstream is already trying to escape.

## Proper slotting model

### Keep this seam
Use `WW3DBackend` for:
- init/shutdown
- device selection
- resolution/window mode
- swap interval
- high-level backend lifecycle

### Add a new typed seam
Introduce a backend-neutral ww3d render-state/pass description that owns:
- world/view/projection
- material/pass state
- shader flags
- texture stage ops
- sampler state
- texture transforms / texcoord generation
- lights / light environment
- viewport/scissor
- render target
- VB/IB bindings and draw topology

### Then split backend application
- **DX8/D3D9 backend** translates that description to current DX wrapper behavior
- **BGFX backend** translates that same description to:
  - bgfx state bits
  - shader permutation/program selection
  - uniform uploads
  - texture/sampler binding
  - framebuffer/view setup
  - draw submission

## Why this is better
This preserves compatibility while preventing `WW3DBackend` from turning into an ever-growing bag of DX8-specific methods.

---

# 6. Where the current BGFX prototype is sound vs where it is fighting the codebase

## Architecturally sound parts

### A. Backend seam at `WW3D`
Good move. This matches the direction of other modernizations in the repo.

### B. Staged branch approach
Also good:
- abstraction first
- structure second
- implementation third
- cleanup after review

That is much healthier than one giant unreviewable prototype dump.

### C. Using `DX8Wrapper` as a temporary interception point
Reasonable as a bootstrap because the engine already centralizes a lot of render behavior there.

## Places where the prototype is fighting the design

### A. Projection and render semantics are not fully bridged
The current seam does not appear to cleanly capture every semantic path the engine uses, especially around transforms/projection and other fixed-function assumptions.

### B. Texture ownership is backwards
Right now BGFX texture upload still leans on DX9-era texture internals.
That is expedient for bring-up, but wrong as a long-term architecture.

### C. Texture stage / sampler / texgen coverage is incomplete
The engine relies on a lot of DX8-style texture-stage behavior.
Current BGFX work covers some of this, but not the whole semantic surface.

### D. Resource caches are still lifetime-risky
The local assessment already identified pointer-keyed cache invalidation as a likely bug source.

### E. Shader model is still prototype-thin
Current shaders are enough for bootstrap rendering, but not necessarily enough to represent full ww3d material semantics cleanly.

### F. Framebuffer/scissor/viewport semantics need stronger validation
The infrastructure exists, but the correctness story is not finished.

---

# 7. Current BGFX progress assessment

## What appears largely done
- backend abstraction exists
- null backend exists
- BGFX structure exists
- build wiring exists
- texture-stage bridge exists at a basic level
- stable material uniforms exist
- light environment upload is substantially implemented
- active draw-path stubs were largely removed
- shader build plumbing exists
- fake-success persistence stubs were corrected to honest failure

## What appears partial
- texture stage edge cases
- material semantic correctness
- point/spot light correctness in real scenes
- render-state parity with DX8
- runtime shader loading robustness
- device switching/reinit correctness
- framebuffer/scissor/viewport correctness

## What appears still open
- real content validation
- known-good/known-bad issue matrix
- CI-complete support for the BGFX path
- Windows runtime proof on your actual machine
- clean review-ready documentation of what works vs what is deferred

---

# 8. Project status right now

## Upstream mainline status
`w3dhub/OpenW3D` currently looks like a serious modernization branch of the archive, not a dead mirror.

Repo snapshot:
- stars: 17
- forks: 5
- open PRs: 7
- closed PRs: 97
- default branch: `main`
- latest upstream push in local metadata: 2026-03-25

### What upstream main can currently claim
- builds through CI on multiple Windows toolchains
- partial Linux build support exists
- modern build system exists
- portability and cleanup work is extensive
- several legacy subsystems already have modernization seams

### What upstream main still cannot claim
- full engine modernization is complete
- full Linux client path is done
- legacy assumptions are fully removed
- renderer architecture has been deeply reauthored yet

## Dadud fork / BGFX effort status
Your fork is ahead specifically in the renderer-modernization direction.

### What it can currently claim
- a coherent BGFX prototype direction exists
- branch stack and PR intent can be reconstructed
- post-review cleanup happened after the renderer draft PR
- there is enough code to justify deeper investment

### What it cannot yet claim
- full Windows-local runtime success
- full CI-green BGFX path
- production-grade backend-neutral rendering abstraction
- safe branch cleanup/review simplification yet

---

# 9. Potential direction options

## Direction 1 — Prototype-completion first, architecture cleanup second
This is the path you already chose.

### Meaning
- finish `fix/review-pr3`
- get Windows runtime success
- get CI green
- then clean up branches/PRs and refine architecture

### Why this is good
- fastest way to prove viability
- avoids branch churn while the prototype is unstable
- lets real runtime failures tell us where the abstraction actually needs to change

### Risk
- temporary architecture hacks may grow if not kept under control

## Direction 2 — Harden the abstraction before deeper runtime work
### Meaning
Pause and refactor the render seam toward a typed render-state translator now.

### Why it’s attractive
- cleaner long-term design
- fewer BGFX-vs-DX9 conceptual mismatches

### Why I do **not** recommend it first
- high risk of losing momentum
- easy to get stuck in abstraction work without proving the prototype
- less useful until we know exactly what breaks on real Windows runtime + CI

## Direction 3 — Hybrid path
This is my recommended direction.

### Meaning
Use the current prototype path to get runtime/CI proof, but make targeted architectural corrections only where they directly unblock correctness.

### Concretely
Do **not** re-architect everything yet.
Do:
- fix missing semantic seams that are obviously required
- reduce dependence on DX9-owned resources where practical
- improve diagnostics and validation
- keep notes on which abstraction changes are proven necessary by runtime evidence

This gives you a prototype that informs the eventual proper refactor instead of guessing ahead of time.

---

# 10. Recommended next engineering priorities

## Highest-priority prototype goals
1. **Windows local runtime proof**
   - configure
   - build
   - shader assets compile/load
   - backend initializes
   - real scene renders plausibly

2. **CI green for the BGFX path**
   - identify existing unrelated upstream CI failures vs BGFX-introduced failures
   - reduce the delta until BGFX branch is not making CI worse

3. **Validation matrix**
   Minimum tests:
   - opaque textured mesh
   - lit mesh
   - alpha test
   - alpha blend
   - fog
   - sorting renderer path
   - render target / framebuffer path
   - screenshot/readback behavior

## Highest-priority architecture corrections that still fit prototype-first work
1. improve projection/transform seam if missing
2. tighten texture-stage/sampler/texgen translation
3. reduce DX9-resource dependence in texture upload path
4. improve resource invalidation/lifetime behavior
5. improve shader/program failure diagnostics

## Defer until after prototype success
- branch/PR cleanup
- full review slicing and public branch rename strategy
- deeper generalized renderer abstraction cleanup
- SDL3 gamepad branch work

---

# 11. Final recommendation

## Recommended strategic stance
Treat BGFX as a **prototype-backed architecture exercise**, not just a patchset.

### Specifically
- **short term:** get the prototype working on your Windows machine and passing CI
- **medium term:** convert prototype lessons into a cleaner render-state translation layer
- **long term:** make BGFX a first-class peer backend that no longer depends on DX9-era resource assumptions

## In one sentence
The right direction is:

> finish the BGFX prototype on top of the already-modernized OpenW3D mainline, but use the results to evolve from “BGFX as a fake DX8 device” toward “BGFX as a true backend consuming ww3d render semantics.”

---

# 12. Bottom-line status call

If I had to give a single status line for the whole project:

> Upstream OpenW3D is a serious modernization of the EA archive, and your BGFX branch is a credible renderer prototype that now needs runtime proof, CI proof, and a cleaner semantic seam before it becomes a proper long-term backend.
