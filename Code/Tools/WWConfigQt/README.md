# WWConfigQt

Qt Widgets front end for the shared WWConfig settings backend.

## Designer forms

Open these files directly in Qt Designer:

- `MainWindow.ui`: banner, tabs, and save/cancel buttons
- `VideoPage.ui`: device and display controls
- `AudioPage.ui`: volume and playback controls
- `PerformancePage.ui`: presets and expert settings

The forms own static widgets, layouts, fallback text, label buddies, and tab order. The main window's tab layouts host the three separately designed pages, which are constructed with their shared backend in C++. Hardware-dependent choices, settings values, legacy locale overrides, and signal handling remain in C++.

CMake AUTOUIC generates `ui_*.h` in the build directory. Edit the forms, not those generated headers. List new forms in `WWCONFIG_QT_UI`; the Designer-form test checks the list against the directory and validates every form with `uic`.

## Build and test

From the repository root in a Visual Studio x64 developer shell with `VCPKG_ROOT` set:

```powershell
cmake --preset windows-qt-tools -B build/qt-tools -DW3D_BUILD_OPTION_FFMPEG=ON -DW3D_BUILD_OPTION_OPENAL=ON -DW3D_BUILD_OPTION_MILES=OFF -DBUILD_TESTING=ON
cmake --build build/qt-tools --config Release --target wwconfig_qt wwconfig_qt_settings_tests
ctest --test-dir build/qt-tools -C Release -R "^wwconfig_qt_" --output-on-failure
```

Override `VCPKG_INSTALLED_DIR` when configuring if packages are installed outside the preset's `C:/vcpkg.installed` directory.

The UI tests substitute a deterministic backend to exercise settings bindings, presets, save/cancel, and failed saves without changing real INI or registry settings. They do not test actual persistence or hardware enumeration. Set `OPENW3D_QT_SCREENSHOT_DIR` to an output directory to capture all three pages at compact and larger sizes during testing.

On Windows, building the application deploys the matching Qt runtime and platform plugin using `windeployqt`. Keep the generated plugin directories beside the executable when copying it. The legacy DirectX runtime remains a separate system requirement.
