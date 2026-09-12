include(qt_tool_helpers)

set(WWCONFIG_QT_SRC
    main.cpp
    MainWindow.cpp
    MainWindow.h
    PerformancePage.cpp
    PerformancePage.h
    VideoPage.cpp
    VideoPage.h
    AudioPage.cpp
    AudioPage.h
    WWConfigBackend.cpp
    WWConfigBackend.h
    resources.qrc
)

if(WIN32)
    list(APPEND WWCONFIG_QT_SRC
        wwconfig_qt.rc
    )
endif()

set(WWCONFIG_QT_UI
    MainWindow.ui
    VideoPage.ui
    AudioPage.ui
    PerformancePage.ui
)

add_executable(wwconfig_qt WIN32)

target_sources(wwconfig_qt PRIVATE ${WWCONFIG_QT_SRC} ${WWCONFIG_QT_UI})
set_target_properties(wwconfig_qt PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

target_include_directories(wwconfig_qt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wwconfig_qt
    PRIVATE
        w3d_qt_toolkit
        wwconfig_core
        wwcommon
        wwaudio
)

if(WIN32)
    target_link_libraries(wwconfig_qt PRIVATE
        wwdebug
        wwmath
        wwphys
        wwsaveload
        ww3d2
    )
    target_link_libraries(wwconfig_qt PRIVATE winmm)
endif()

w3d_deploy_qt_runtime(wwconfig_qt)

if(BUILD_TESTING)
    find_package(Qt6 ${W3D_QT_MIN_VERSION} COMPONENTS Test REQUIRED)
    set(WWCONFIG_QT_TEST_SRC ${WWCONFIG_QT_SRC})
    list(REMOVE_ITEM WWCONFIG_QT_TEST_SRC main.cpp WWConfigBackend.cpp wwconfig_qt.rc)
    add_executable(wwconfig_qt_settings_tests
        tests/SettingsPageTests.cpp ${WWCONFIG_QT_TEST_SRC} ${WWCONFIG_QT_UI})
    target_include_directories(wwconfig_qt_settings_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    get_target_property(WWCONFIG_QT_LIBRARIES wwconfig_qt LINK_LIBRARIES)
    target_link_libraries(wwconfig_qt_settings_tests PRIVATE ${WWCONFIG_QT_LIBRARIES})
    w3d_configure_qt_test(wwconfig_qt_settings_tests)
    w3d_add_designer_test(wwconfig_qt_designer_forms ${WWCONFIG_QT_UI})
endif()
