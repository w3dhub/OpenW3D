add_library(wdump_core STATIC)

target_sources(wdump_core
    PRIVATE
        wdump_core.cpp
        wdump_core.h
        wdump_chunk_names.cpp
        wdump_fields.cpp
)

target_include_directories(wdump_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wdump_core
    PRIVATE
        wwlib
        wwcommon
)
