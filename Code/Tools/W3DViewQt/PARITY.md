# W3DViewQt parity validation

This records the seven remaining items in [PR #158](https://github.com/w3dhub/OpenW3D/pull/158),
as listed on September 20, 2026. The fixes and tests stay in the viewer; no shared
engine source changes are included.

## Remaining-work audit

| Requirement | Implementation and evidence |
| --- | --- |
| Native aggregate creation/editing and lineup mutation | `ViewerNativeWorkflowTests::aggregateEditingAndLineup` creates an aggregate through its action and dialog, attaches a model to a bone, reloads the saved prototype, cancels removal, renames it, adds two lineup objects, checks their spacing, and clears the lineup. |
| Native sound creation/editing, animation playback, and spatial attenuation | `soundCreationEditingAndAttenuation` creates a looping 3D sound through the dialog, checks its settings and OpenAL playback, measures reduced source gain near the drop-off radius, checks culling and recovery as the camera moves, edits it into a 2D sound, saves/reloads it, and plays a clone after deleting its source object and prototype. `animationTriggeredSound` checks companion-WAV playback on animation selection and resume. |
| Full visible-settings save/change/load round trip | `visibleSettingsRoundTrip` saves both settings groups through the dialog, changes them, reloads, and compares all lighting/background values. Native lighting getters read the actual scene light. Both solid-color and generated-bitmap backgrounds must produce different pixels after the change and identical pixels after reload. |
| Reverse, automatic, and screen-area HLOD behavior | `manualAutomaticAndScreenAreaLod` uses previous/next actions, records the current screen area and reloads the threshold, verifies manual selection survives rendering, verifies automatic near/far switching, and adds/removes the NULL level. |
| Lossless emitter line-property export | `ViewerAssetIOTests::emitterLinePropertiesRoundTrip` covers all five render modes. It exports, reloads through the viewer's asset manager, compares the complete line-properties structure including reserved fields and unknown flags, and requires an identical second export. |
| Lossless HLOD aggregate/proxy-array export | `hlodMetadataSurvivesExportAndLodEdits` requires an identical initial export, preserves aggregate/proxy arrays and unknown chunks through threshold and NULL-level edits, and reloads the resulting prototype. Unresolved model references and empty container chunks are retained. |
| Quick-settings shortcuts 1–9 | `quickSettingsKeys` sends actual key events to the native viewport, loads nine distinct disposable files, and checks the resulting background, ambient light, and fog state. Files are created exclusively and removed by scope guards. |

The native workflow suite contains no skipped cases and does not require external
game assets.

## Defects fixed during validation

- Aggregate edits used the runtime composite's disabled base-name accessor and
  could save the aggregate's own name as its base, causing recursive loading.
  Reconstruction and the bone dialog now use the registered prototype's base.
- Edited and cloned sounds retained a pointer to a temporary audio definition.
  Viewer sound objects now own their definitions throughout playback and cloning.
  They also restore their original gain before each audio update, allowing
  attenuation to recover when the camera returns from the drop-off radius.
- The engine emitter writer omitted line properties, and its reader checked the
  wrong chunk identifier. Viewer-specific serialization and a registered loader
  preserve and reload the complete structure.
- Rebuilding HLOD definitions from live render objects discarded aggregate/proxy
  arrays and missing dependencies. Viewer prototypes now retain the original
  chunks and update only the edited LOD fields.

## Validation

Windows x64 Release, Visual Studio 2022, Qt 6.8.2, Direct3D, OpenAL Soft null output:

- Viewer and all registered test executables built successfully.
- All 19 registered CTest tests passed, including the two native suites and all
  28 Designer forms.
- The asset round-trip suite and native workflow suite also passed with MSVC
  AddressSanitizer.
- The existing main-window suite passed 19 cases; its two optional external-asset
  cases were skipped. The native fog suite passed its generated-scene case;
  its optional external-background case was skipped.
- Qt runtime deployment completed with `windeployqt`.

The broader existing regression suites cover menus/action wiring, animation
fixtures, individual settings save masks, scene lights, emitter/primitive editing,
background and resolution dialogs, export staging/error handling, screenshots,
movie frame capture, audio lifetime, and memory-pool alignment.

Build and native-test commands are in [README.md](README.md). Detailed native
results are written to `Code/Tools/W3DViewQt/native-workflows.txt` under the build
directory.

## Validation limits

The optional tests that require a specific Renegade asset bundle were not run in
this pass. Physical speaker output was not checked; playback, source gain,
attenuation, and culling were checked against OpenAL Soft's null driver. These
automated results do not claim a fresh manual review of every asset or UI gesture.
