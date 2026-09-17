# WDumpQt

Qt Widgets inspector for W3D chunk files, using the shared WDumpCore parser.

## Designer form

Open `MainWindow.ui` in Qt Designer to edit the window, splitters, inspector views, menus, actions, toolbar, and tab order. Resources use `resources.qrc`, so the existing application icon is available in Designer.

The form owns the static interface. Chunk models, recent-file entries, the system monospace font, platform-standard icons/shortcuts, and signal handling remain in C++. File, find, and message dialogs use Qt's standard dialogs.

CMake AUTOUIC generates `ui_MainWindow.h` in the build directory. Edit the form, not the generated header. List new forms in `WDUMP_QT_UI`; the Designer-form test checks the manifest and validates each form with `uic`.

## Build and test

From the repository root in a Visual Studio x64 developer shell with `VCPKG_ROOT` set:

```powershell
cmake --preset windows-qt-tools -B build/qt-tools -DW3D_BUILD_OPTION_FFMPEG=ON -DW3D_BUILD_OPTION_OPENAL=ON -DW3D_BUILD_OPTION_MILES=OFF -DBUILD_TESTING=ON
cmake --build build/qt-tools --config Release --target wdump_qt wdump_qt_window_tests
ctest --test-dir build/qt-tools -C Release -R "^wdump_qt_" --output-on-failure
```

Override `VCPKG_INSTALLED_DIR` when configuring if packages are installed outside the preset's `C:/vcpkg.installed` directory.

Tests generate temporary chunk files and isolate recent-file settings. They cover read-only inspection, failed-load recovery, search wrapping, recent files, and view actions. Set `OPENW3D_QT_SCREENSHOT_DIR` to an output directory to capture compact and larger windows during testing.

On Windows, building the application deploys the matching Qt runtime and platform plugin using `windeployqt`. Keep the generated plugin directories beside the executable when copying it.
