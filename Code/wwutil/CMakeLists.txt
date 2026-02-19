# Set source files
set(WWUTIL_SRC
    mathutil.cpp
    mathutil.h
    miscutil.cpp
    miscutil.h
    ProcessManager.cpp
    ProcessManager.h
    $<$<BOOL:${WIN32}>:stackdump-windows.cpp>
    $<$<BOOL:${LINUX}>:stackdump-linux.cpp>
    stackdump.h
)

# Targets to build.
add_library(wwutil STATIC)

target_link_libraries(wwutil PRIVATE
    wwcommon
)
if(W3D_BUILD_OPTION_SDL3)
    target_link_libraries(wwutil PRIVATE SDL3::SDL3)
endif()

target_sources(wwutil PRIVATE ${WWUTIL_SRC})
