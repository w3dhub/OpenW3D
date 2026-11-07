set(QTCOMMON_SRC
    RecentFiles.cpp
    RecentFiles.h
    ShortcutHelpers.cpp
    ShortcutHelpers.h
)

add_library(qtcommon STATIC)

target_sources(qtcommon PRIVATE ${QTCOMMON_SRC})

target_include_directories(qtcommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qtcommon
    PUBLIC
        w3d_qt_toolkit
)

set_target_properties(qtcommon PROPERTIES
    AUTOMOC ON
)
