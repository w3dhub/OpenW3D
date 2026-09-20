include(qt_tool_helpers)

set(WDUMP_QT_WIDGET_SRC
    MainWindow.cpp
    MainWindow.h
    resources.qrc
)

set(WDUMP_QT_UI
    MainWindow.ui
)

# Share compiled widgets and resource objects with both the app and its tests.
add_library(wdump_qt_widgets OBJECT ${WDUMP_QT_WIDGET_SRC} ${WDUMP_QT_UI})

target_include_directories(wdump_qt_widgets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(wdump_qt_widgets
    PUBLIC
        w3d_qt_toolkit
        wdump_core
        wwcommon
    PRIVATE
        qtcommon
        wwdebug
        wwlib
)

if(WIN32)
    target_link_libraries(wdump_qt_widgets PRIVATE winmm)
endif()

set_target_properties(wdump_qt_widgets PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

add_executable(wdump_qt WIN32 main.cpp)
target_link_libraries(wdump_qt PRIVATE wdump_qt_widgets)

if(WIN32)
    target_sources(wdump_qt PRIVATE wdump_qt.rc)
endif()

w3d_deploy_qt_runtime(wdump_qt)

if(BUILD_TESTING)
    find_package(Qt6 ${W3D_QT_MIN_VERSION} COMPONENTS Test REQUIRED)
    add_executable(wdump_qt_window_tests tests/MainWindowTests.cpp)
    target_link_libraries(wdump_qt_window_tests PRIVATE wdump_qt_widgets)
    w3d_configure_qt_test(wdump_qt_window_tests)
    w3d_add_designer_test(wdump_qt_designer_forms ${WDUMP_QT_UI})
endif()
