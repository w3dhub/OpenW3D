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
cmake --build build/w3dview-qt --config Release --target w3dview_qt wwaudio_openal_tests w3dview_qt_main_window_tests w3dview_qt_settings_save_mask_tests w3dview_qt_scene_light_tests w3dview_qt_emitter_edit_tests w3dview_qt_primitive_shader_tests w3dview_qt_background_object_dialog_tests w3dview_qt_sound_dialog_tests w3dview_qt_resolution_dialog_tests w3dview_qt_export_directory_dialog_tests w3dview_qt_export_utils_tests ww3d2_framegrab_tests ww3d2_screenshot_api_tests wwlib_mempool_tests
ctest --test-dir build/w3dview-qt -C Release --output-on-failure
```

The preset uses `C:\vcpkg.installed` for installed packages; override
`VCPKG_INSTALLED_DIR` when configuring if your package tree lives elsewhere.
The `windows-qt-tools` preset enables FFmpeg and OpenAL Soft for the x64 viewer
and disables Miles.
The OpenAL tests select OpenAL Soft's null output driver automatically, so the
default CTest run does not require speakers. The configure presets available in
a checkout are listed by `cmake --list-presets`.

Building `w3dview_qt` runs `windeployqt` after linking. Launch the executable
directly from its configuration directory and keep the generated plugin folders,
especially `platforms/qwindows.dll`, beside it when copying or packaging the
viewer. Windows systems also need the legacy DirectX June 2010 runtime that
provides `d3dx9_43.dll`; it is not deployed by `windeployqt`.

Port status, validation evidence, and remaining manual checks are recorded in `W3DViewQt-ExecPlan.md` at the repository root.
