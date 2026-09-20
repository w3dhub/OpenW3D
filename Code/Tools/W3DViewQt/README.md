# W3DViewQt

W3DViewQt is the Qt Widgets port of the legacy MFC W3D Viewer. Widget hierarchy, labels, layouts, static menus, actions, toolbars, and tab order live in the checked-in `.ui` files in this directory. Selection-specific and data-driven menu entries remain in C++, along with engine behavior, validation, and signal handling.

## Editing layouts

Open a `.ui` file with Qt Designer, save it in place, and rebuild the target. Do not edit generated `ui_*.h` files; CMake AUTOUIC regenerates them in the build tree. `MainWindow.ui` promotes its central viewport widget to `W3DViewport`, whose Direct3D implementation intentionally remains in C++.

When adding a form, list it in `W3DVIEW_QT_UI` in `CMakeLists.txt` and add its filename to `tests/VerifyDesignerForms.cmake`. The Designer-form test checks that every expected form is tracked and accepted by the selected Qt `uic`.

## Build and test

From the repository root in a Visual Studio 2022 x64 developer environment:

```powershell
$env:VCPKG_ROOT = 'C:\path\to\vcpkg'
cmake --preset windows-qt-tools -B build/w3dview-qt
cmake --build build/w3dview-qt --config Release --target w3dview_qt w3dview_qt_asset_io_tests w3dview_qt_main_window_tests w3dview_qt_settings_save_mask_tests w3dview_qt_scene_light_tests w3dview_qt_emitter_edit_tests w3dview_qt_primitive_shader_tests w3dview_qt_background_object_dialog_tests w3dview_qt_sound_dialog_tests w3dview_qt_resolution_dialog_tests w3dview_qt_export_directory_dialog_tests w3dview_qt_export_utils_tests
ctest --test-dir build/w3dview-qt -C Release --output-on-failure
```

The preset uses `C:\vcpkg.installed` for installed packages; override
`VCPKG_INSTALLED_DIR` when configuring if your package tree lives elsewhere.
The `windows-qt-tools` preset enables FFmpeg and OpenAL Soft for the x64 viewer
and disables Miles.
Audio-aware viewer tests select OpenAL Soft's null output driver automatically,
so the default CTest run does not require speakers. The configure presets
available in a checkout are listed by `cmake --list-presets`.

Building `w3dview_qt` runs `windeployqt` after linking. Launch the executable
directly from its configuration directory and keep the generated plugin folders,
especially `platforms/qwindows.dll`, beside it when copying or packaging the
viewer. Windows systems also need the legacy DirectX June 2010 runtime that
provides `d3dx9_43.dll`; it is not deployed by `windeployqt`.

## Native workflow validation

Enable the Direct3D tests explicitly in a Windows Qt build:

```powershell
cmake --preset windows-qt-tools -B build/w3dview-qt -DBUILD_TESTING=ON -DW3DVIEW_QT_ENABLE_NATIVE_VIEWPORT_TESTS=ON
cmake --build build/w3dview-qt --config Release --target w3dview_qt_native_workflow_tests w3dview_qt_viewport_fog_tests
ctest --test-dir build/w3dview-qt -C Release -L native --output-on-failure
```

The workflow suite uses generated W3D models, a generated WAV, isolated application
settings, and temporary files. It opens a native viewport outside the visible
desktop. It exercises aggregate creation and editing, lineup mutation, sound
creation and editing, animation sound playback, spatial attenuation and culling,
settings round trips, HLOD switching, and quick-settings keys 1–9. Existing
`settings1.dat` through `settings9.dat` beside the test executable are never
overwritten; the test fails if a slot is occupied.

The detailed native workflow report is written to `native-workflows.txt` in
the build's `Code/Tools/W3DViewQt` directory. Audio uses OpenAL Soft's null
output driver. This tests playback state, source gain, and listener-driven
culling without requiring speakers.
