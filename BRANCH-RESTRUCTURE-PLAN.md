# OpenW3D Branch Restructure Plan

Created on 2026-04-30 to make the BGFX work easier for humans to understand and review.

## Problem

Current remote branch names mix together:
- old prototypes
- superseded precursor branches
- clean staged BGFX branches
- post-review fixup branches
- unrelated SDL follow-up work

That makes it hard to answer basic review questions like:
- which branch is the real baseline?
- which branch is just historical?
- what is the latest code?
- what should be reviewed as one PR vs split further?

---

## Goal

Separate the branch set into three buckets:

1. **stack/** — the canonical staged branch chain
2. **review/** — optional smaller review slices inside the big renderer branch
3. **archive/** — historical or superseded branches kept only for reference

---

## Local branches created

These were created locally only. No remote pushes were performed.

### Canonical stack branches
- `stack/bgfx-01-abstraction` -> `origin/pr-1-backend-abstraction`
- `stack/bgfx-02-bgfx-structure` -> `origin/pr-2-bgfx-backend`
- `stack/bgfx-03-bgfx-rendering` -> `origin/pr-3-bgfx-rendering-clean`
- `stack/bgfx-04-bgfx-review-fixes` -> `origin/fix/review-pr3`

### Optional review slices
These split the big rendering branch into more digestible checkpoints.

- `review/bgfx-03a-core-render-path` -> `7f6fdd4`
- `review/bgfx-03b-lighting-and-state` -> `87c0c66`
- `review/bgfx-03c-build-and-stub-cleanup` -> `79fde36`
- `review/bgfx-04-post-review-fixes` -> `40e53ff`

### Archive branches
- `archive/bgfx-prototype` -> `origin/bgfx-integration`
- `archive/render-device-caps` -> `origin/pr/render-device-caps`
- `archive/render-device-enum` -> `origin/pr/render-device-enum`
- `archive/sdl3-window-experiment` -> `origin/sdl3-window`

---

## Recommended meaning of each branch

## Canonical stack

### `stack/bgfx-01-abstraction`
Review purpose:
- create the renderer seam
- add `WW3DBackend`
- add `NullBackend`
- make backend selection explicit

This is the first branch reviewers should look at.

### `stack/bgfx-02-bgfx-structure`
Review purpose:
- add BGFX backend skeleton
- add CMake/submodules
- wire backend selection without asking reviewers to validate all renderer behavior yet

This should review as infrastructure, not as a fully working renderer.

### `stack/bgfx-03-bgfx-rendering`
Review purpose:
- add the actual BGFX render path
- bridge DX8-style engine behavior onto BGFX

This is the heavy branch that needs further splitting for friendly review.

### `stack/bgfx-04-bgfx-review-fixes`
Review purpose:
- post-review cleanup and bugfixes after the large renderer branch

This should not be the first branch someone reads; it should be read as a delta on top of branch 03.

---

## Optional review slicing for branch 03

The current renderer branch is still too large for comfortable review.
It can be mentally or operationally split like this:

### `review/bgfx-03a-core-render-path` (`7466cfc` -> `7f6fdd4`)
Theme:
- buffer wrappers
- texture bridging
- shader scaffolding
- core backend path wiring
- dynamic buffer support

This is the first major “make BGFX actually able to draw something” step.

### `review/bgfx-03b-lighting-and-state` (`7f6fdd4` -> `87c0c66`)
Theme:
- light environment upload
- DX8 light wiring
- camera position uniform
- fill-mode integration into shader state
- draw sorting backend path work

This is the state/lighting correctness phase.

### `review/bgfx-03c-build-and-stub-cleanup` (`87c0c66` -> `79fde36`)
Theme:
- alpha test / point light support
- build fixes
- removal of remaining active stubs
- checklist/docs update

This is the “clean enough to hand to reviewers” phase of the renderer branch.

### `review/bgfx-04-post-review-fixes` (`79fde36` -> `40e53ff`)
Theme:
- fix stubs still left after draft PR
- ownership/lifetime fixes
- shader build fixes
- resource cache cleanup
- program submission fix

This is the important post-draft stabilization delta.

---

## Recommended remote cleanup strategy

If you want the remotes to become human-friendly too, I recommend this sequence:

### Keep as the public chain
- `bgfx/01-abstraction`
- `bgfx/02-structure`
- `bgfx/03-rendering`
- `bgfx/04-review-fixes`

### Move old names to archive or delete after confirmation
Archive/delete candidates:
- `bgfx-integration`
- `pr/render-device-caps`
- `pr/render-device-enum`
- `pr-3-bgfx-rendering-fixed` (duplicate tip of `fix/review-pr3`)
- `fix/review-pr3` (replace with `bgfx/04-review-fixes` once renamed)

### Leave out for now
- `feature/sdl3-gamepad`
- `sdl3-window`

These should stay separate until BGFX stabilization is settled.

---

## Reviewer-friendly PR sequence

If you want to reopen/reframe this for reviewers, the best structure is:

1. **PR A — Backend abstraction**
   - `stack/bgfx-01-abstraction`
2. **PR B — BGFX structure / build integration**
   - `stack/bgfx-02-bgfx-structure`
3. **PR C1 — BGFX core render path**
   - `review/bgfx-03a-core-render-path`
4. **PR C2 — BGFX lighting + state bridging**
   - `review/bgfx-03b-lighting-and-state`
5. **PR C3 — BGFX cleanup / build / stub removal**
   - `review/bgfx-03c-build-and-stub-cleanup`
6. **PR D — Post-review fixes**
   - `review/bgfx-04-post-review-fixes`

That is much easier to read than “one huge rendering PR + a mysterious fix branch”.

---

## Current recommendation

For now, treat these as canonical locally:
- `stack/bgfx-01-abstraction`
- `stack/bgfx-02-bgfx-structure`
- `stack/bgfx-03-bgfx-rendering`
- `stack/bgfx-04-bgfx-review-fixes`

And treat these as human review aids:
- `review/bgfx-03a-core-render-path`
- `review/bgfx-03b-lighting-and-state`
- `review/bgfx-03c-build-and-stub-cleanup`
- `review/bgfx-04-post-review-fixes`

---

## Important note

No remote branches were renamed, deleted, or pushed as part of this restructure.
This file and the local branches are a safe local reorganization only.

If desired, the next step is to:
- rename/push the cleaned branch names to your remote fork
- optionally update or replace the draft PR chain on `w3dhub/OpenW3D`
- prune duplicate/superseded names after confirming nothing important still points at them
