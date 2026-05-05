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

add_executable(wwconfig_qt WIN32)

target_sources(wwconfig_qt PRIVATE ${WWCONFIG_QT_SRC})

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

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt.exe
        HINTS "${Qt6_DIR}/../../../tools/Qt6/bin"
    )
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET wwconfig_qt POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                "$<TARGET_FILE:wwconfig_qt>"
            COMMENT "Running windeployqt for wwconfig_qt"
            VERBATIM)
    else()
        message(WARNING "windeployqt not found; wwconfig_qt will require manual Qt runtime deployment.")
    endif()
endif()
