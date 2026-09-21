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
- All 20 registered CTest tests passed with the Steam Renegade Data directory
  enabled, including the two native suites, generated MIX archive coverage,
  and all 28 Designer forms.
- The asset round-trip, native workflow, and generated MIX archive suites passed
  with MSVC AddressSanitizer. The main-window suite also passed all 21 cases under
  AddressSanitizer with the Steam archives enabled.
- The native fog/background sanitizer run with the Steam archives exposed an
  existing shared-engine issue during viewport initialization: `strtrim` calls
  `strcpy` with overlapping source/destination ranges while parsing `DAZZLE.INI`
  (`Code/wwlib/trim.cpp:71`). That run did not reach the background assertions.
  The corresponding Release suite passed; the sanitizer finding remains open.
- The main-window suite passed 21 cases, including real-asset animation,
  editing/export, sound preview, and streaming audio. The native fog/background
  suite passed 4 cases. Neither suite skipped a case.
- Legacy emitter exports may add the previously absent default line-properties
  chunk. The real-asset check requires every original chunk byte to be retained
  and an identical second export through the viewer's loaders.
- Qt runtime deployment completed with `windeployqt`.

The broader existing regression suites cover menus/action wiring, animation
fixtures, individual settings save masks, scene lights, emitter/primitive editing,
background and resolution dialogs, export staging/error handling, screenshots,
movie frame capture, audio lifetime, and memory-pool alignment.

Build and native-test commands are in [README.md](README.md). Detailed native
results are written to `Code/Tools/W3DViewQt/native-workflows.txt` under the build
directory.

## Testing from a game installation

Set `W3DVIEW_GAME_DIR` to the installation root or its `Data` folder and run
CTest with `-L external-assets -V` to enable the three external-asset cases.
The tests discover the Always archives and MIX files, load engine dependencies
directly from them, and temporarily stage individual files required by Qt.
The game installation is not modified. Extracted-asset directories remain
supported through `W3DVIEW_EXTERNAL_ASSET_DIR`.

`ExternalTestAssetsTests` checks install-root/Data discovery, multiple archives,
loose-file and Always override priority, case-insensitive archive lookup,
large-file staging, direct engine loading of an archived W3D hierarchy,
missing/invalid inputs, factory restoration, and temporary-file cleanup.
These checks use generated archives; they do not constitute validation against
a real game installation. Commands and lookup details are in [README.md](README.md).

## Validation limits

All three external-asset cases passed in Release against the installed Steam
edition's archives. The native sanitizer run still requires a separate shared-engine
fix for the INI trimming issue described above. Physical speaker output was not
checked; playback, source gain,
attenuation, and culling were checked against OpenAL Soft's null driver. These
automated results do not claim a fresh manual review of every asset or UI gesture.
