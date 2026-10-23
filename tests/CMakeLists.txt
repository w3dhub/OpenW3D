include(cmake/TestHelpers.cmake)

# Build the registered test executables without building the game or tool apps.
add_custom_target(w3d_tests)

set(code_dir "${PROJECT_SOURCE_DIR}/Code")

w3d_add_test(wwaudio_lifetime_tests
    SOURCES WWAudio/WWAudioLifetimeTests.cpp
    LIBRARIES wwaudio ww3d2 wwdebug wwlib wwmath wwphys wwsaveload wwcommon)
if(WIN32)
    target_link_libraries(wwaudio_lifetime_tests PRIVATE version winmm)
endif()

if(WIN32)
    w3d_add_test(ww3d2_screenshot_api_tests
        SOURCES ww3d2/ScreenshotApiTests.cpp
        LIBRARIES wwcommon wwdebug wwlib wwmath wwsaveload ww3d2 wwaudio)

    w3d_add_test(ww3d2_emitter_serialization_tests
        SOURCES ww3d2/EmitterSerializationTests.cpp
        LIBRARIES wwcommon wwdebug wwlib wwmath wwsaveload ww3d2 wwaudio)

    w3d_add_test(ww3d2_framegrab_tests
        SOURCES ww3d2/FrameGrabTests.cpp "${code_dir}/ww3d2/framgrab.cpp"
        LIBRARIES wwcommon vfw32 winmm)
endif()

# Keep this target independent of wwlib so Linux can exercise the message pump
# before the entire library is portable.
w3d_add_test(wwlib_msgloop_tests
    SOURCES wwlib/MessageLoopTests.cpp "${code_dir}/wwlib/msgloop.cpp")
target_include_directories(wwlib_msgloop_tests PRIVATE "${code_dir}/wwlib")
target_compile_features(wwlib_msgloop_tests PRIVATE cxx_std_20)
if(W3D_BUILD_OPTION_SDL3)
    target_link_libraries(wwlib_msgloop_tests PRIVATE SDL3::SDL3-shared)
else()
    target_link_libraries(wwlib_msgloop_tests PRIVATE user32)
endif()
set_tests_properties(wwlib_msgloop_tests PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy"
    TIMEOUT 10)
if(W3D_BUILD_OPTION_SDL3 AND WIN32)
    set_tests_properties(wwlib_msgloop_tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:SDL3::SDL3-shared>")
endif()

w3d_add_test(wwlib_mempool_tests
    SOURCES wwlib/MempoolTests.cpp
    LIBRARIES wwlib wwdebug wwcommon)
target_include_directories(wwlib_mempool_tests PRIVATE "${code_dir}/wwlib")

if(TARGET wwconfig_qt_widgets OR TARGET wdump_qt_widgets)
    find_package(Qt6 ${W3D_QT_MIN_VERSION} COMPONENTS Test REQUIRED)
endif()

if(TARGET wwconfig_qt_widgets)
    w3d_add_test(wwconfig_qt_settings_tests
        SOURCES Tools/WWConfigQt/SettingsPageTests.cpp
        LIBRARIES wwconfig_qt_widgets
        ARGS -o -,txt)
    w3d_configure_qt_test(wwconfig_qt_settings_tests)
    w3d_add_designer_test(wwconfig_qt_designer_forms wwconfig_qt_widgets)
endif()

if(TARGET wdump_qt_widgets)
    w3d_add_test(wdump_qt_window_tests
        SOURCES Tools/WDumpQt/MainWindowTests.cpp
        LIBRARIES wdump_qt_widgets
        ARGS -o -,txt)
    w3d_configure_qt_test(wdump_qt_window_tests)
    w3d_add_designer_test(wdump_qt_designer_forms wdump_qt_widgets)
endif()
