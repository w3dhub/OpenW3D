set(WDUMP_QT_SRC
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

add_executable(wdump_qt WIN32)

target_sources(wdump_qt PRIVATE ${WDUMP_QT_SRC})

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
        wwlib
        wwcommon
)

if(WIN32)
    target_link_libraries(wdump_qt PRIVATE winmm)
endif()

set_target_properties(wdump_qt PROPERTIES
    AUTOMOC ON
)
