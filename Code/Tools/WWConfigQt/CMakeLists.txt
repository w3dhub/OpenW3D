include(qt_tool_helpers)

set(WWCONFIG_QT_WIDGET_SRC
    MainWindow.cpp
    MainWindow.h
    PerformancePage.cpp
    PerformancePage.h
    VideoPage.cpp
    VideoPage.h
    AudioPage.cpp
    AudioPage.h
    WWConfigBackend.h
    resources.qrc
)

set(WWCONFIG_QT_UI
    MainWindow.ui
    VideoPage.ui
    AudioPage.ui
    PerformancePage.ui
)

# Share compiled widgets and resource objects with both the app and its tests.
add_library(wwconfig_qt_widgets OBJECT ${WWCONFIG_QT_WIDGET_SRC} ${WWCONFIG_QT_UI})
set_target_properties(wwconfig_qt_widgets PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

target_include_directories(wwconfig_qt_widgets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wwconfig_qt_widgets
    PUBLIC
        w3d_qt_toolkit
        wwcommon
    PRIVATE
        wwconfig_core
        wwaudio
)

if(WIN32)
    target_link_libraries(wwconfig_qt_widgets PRIVATE
        wwdebug
        wwmath
        wwphys
        wwsaveload
        ww3d2
    )
    target_link_libraries(wwconfig_qt_widgets PRIVATE winmm)
endif()

# Tests provide their own backend implementation.
add_executable(wwconfig_qt WIN32 main.cpp WWConfigBackend.cpp)
target_link_libraries(wwconfig_qt PRIVATE wwconfig_qt_widgets wwconfig_core)

if(WIN32)
    target_sources(wwconfig_qt PRIVATE wwconfig_qt.rc)
endif()

w3d_deploy_qt_runtime(wwconfig_qt)

if(BUILD_TESTING)
    find_package(Qt6 ${W3D_QT_MIN_VERSION} COMPONENTS Test REQUIRED)
    add_executable(wwconfig_qt_settings_tests tests/SettingsPageTests.cpp)
    target_link_libraries(wwconfig_qt_settings_tests PRIVATE wwconfig_qt_widgets)
    w3d_configure_qt_test(wwconfig_qt_settings_tests)
    w3d_add_designer_test(wwconfig_qt_designer_forms ${WWCONFIG_QT_UI})
endif()
