# Set source files
set(WWDEBUG_SRC
    wwdebug.cpp
    wwmemlog.cpp
    wwprofile.cpp
    wwdebug.h
    wwhack.h
    wwmemlog.h
    wwprofile.h
)

# Targets to build.
add_library(wwdebug STATIC)

target_link_libraries(wwdebug PRIVATE
    wwcommon
)

target_sources(wwdebug PRIVATE ${WWDEBUG_SRC})

# This module needs building differently for the level editor
if (W3D_TOOLS)
    # Targets to build.
    add_library(wwdebuge STATIC)

    target_link_libraries(wwdebuge PRIVATE
        wwcommon
    )

    target_compile_definitions(wwdebuge PRIVATE PARAM_EDITING_ON)

    target_sources(wwdebuge PRIVATE ${WWDEBUG_SRC})
endif()
