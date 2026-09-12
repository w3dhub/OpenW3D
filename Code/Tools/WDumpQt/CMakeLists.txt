include(qt_tool_helpers)

set(WDUMP_QT_SRC
    main.cpp
    MainWindow.cpp
    MainWindow.h
    resources.qrc
)

if(WIN32)
    list(APPEND WDUMP_QT_SRC
        wdump_qt.rc
    )
endif()

set(WDUMP_QT_UI
    MainWindow.ui
)

add_executable(wdump_qt WIN32)

target_sources(wdump_qt PRIVATE ${WDUMP_QT_SRC} ${WDUMP_QT_UI})

target_include_directories(wdump_qt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(wdump_qt
    PRIVATE
        qtcommon
        w3d_qt_toolkit
        wdump_core
        wwdebug
        wwlib
        wwcommon
)

if(WIN32)
    target_link_libraries(wdump_qt PRIVATE winmm)
endif()

set_target_properties(wdump_qt PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

w3d_deploy_qt_runtime(wdump_qt)

if(BUILD_TESTING)
    find_package(Qt6 ${W3D_QT_MIN_VERSION} COMPONENTS Test REQUIRED)
    add_executable(wdump_qt_window_tests
        tests/MainWindowTests.cpp MainWindow.cpp MainWindow.h resources.qrc ${WDUMP_QT_UI})
    target_include_directories(wdump_qt_window_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    get_target_property(WDUMP_QT_LIBRARIES wdump_qt LINK_LIBRARIES)
    target_link_libraries(wdump_qt_window_tests PRIVATE ${WDUMP_QT_LIBRARIES})
    w3d_configure_qt_test(wdump_qt_window_tests)
    w3d_add_designer_test(wdump_qt_designer_forms ${WDUMP_QT_UI})
endif()
