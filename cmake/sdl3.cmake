# SDL3 configuration for OpenW3D
# Uses find_package(SDL3) for all build configurations.
# For MinGW cross-compilation, provide SDL3 via CMAKE_PREFIX_PATH.

if(NOT W3D_BUILD_OPTION_SDL3)
    return()
endif()

find_package(SDL3 REQUIRED COMPONENTS SDL3-shared Headers)

message(STATUS "SDL3 found: ${SDL3_DIR}")
