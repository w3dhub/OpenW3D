#
# BGFX CMake integration for OpenW3D
#
# BGFX uses genie (from bx) for building. This cmake provides integration
# by setting up include paths and linking when bgfx is built.
#
# IMPORTANT - Build Requirements:
#   - genie build tool: https://github.com/bkaradzic/genie
#   - C++20 compiler (GCC 11+, Clang 11+, MSVC 17.5+)
#
# To enable and build bgfx:
#   # First build bgfx itself:
#   cd external/bgfx
#   make linux-gcc-release64
#
#   # Then build OpenW3D with bgfx backend:
#   cd ../..
#   mkdir build && cd build
#   cmake .. -DENABLE_BGFX_BACKEND=ON
#   make
#

set(BGFX_DIR "${CMAKE_SOURCE_DIR}/external/bgfx")
set(BX_DIR "${CMAKE_SOURCE_DIR}/external/bx")

if(NOT ENABLE_BGFX_BACKEND)
    return()
endif()

# Verify bx submodule is present
if(NOT EXISTS "${BX_DIR}/include/bx/bx.h")
    message(FATAL_ERROR "bx submodule not initialized. Run:\n  git submodule update --init --recursive")
endif()

# Verify bgfx submodule is present
if(NOT EXISTS "${BGFX_DIR}/include/bgfx/bgfx.h")
    message(FATAL_ERROR "bgfx submodule not initialized. Run:\n  git submodule update --init --recursive")
endif()

# Create interface library for bgfx headers and dependencies
add_library(bgfx INTERFACE)

target_include_directories(bgfx INTERFACE
    "${BGFX_DIR}/include"
    "${BGFX_DIR}/include/c99"
    "${BX_DIR}/include"
)

# BGFX requires C++20
target_compile_features(bgfx INTERFACE cxx_std_20)

# Platform defines
if(WIN32)
    target_compile_definitions(bgfx INTERFACE BGFX_PLATFORM_WINDOWS=1)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(bgfx INTERFACE BGFX_PLATFORM_LINUX=1)
elseif(APPLE)
    target_compile_definitions(bgfx INTERFACE BGFX_PLATFORM_OSX=1)
endif()

# Find system dependencies for bgfx
if(WIN32)
    target_link_libraries(bgfx INTERFACE winmm user32 gdi32 dxguid)
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(bgfx INTERFACE OpenGL::GL X11::X11 X11::Xrandr X11::Xcursor dl pthread rt)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
    target_link_libraries(bgfx INTERFACE ${COCOA_LIBRARY} ${FOUNDATION_LIBRARY})
endif()

# Look for bgfx library in common build output locations
set(BGFX_LIB_FOUND FALSE)

foreach(CONFIG release release64 debug debug64)
    set(CANDIDATE "${BGFX_DIR}/.build/linux64_gcc/bin/libbgfx${CONFIG}.a")
    if(EXISTS "${CANDIDATE}")
        set(BGFX_LIB "${CANDIDATE}")
        set(BGFX_LIB_FOUND TRUE)
        break()
    endif()
endforeach()

if(NOT BGFX_LIB_FOUND)
    file(GLOB_RECURSE BGFX_LIB_CANDIDATES "${BGFX_DIR}/.build/**/libbgfx*.a")
    if(BGFX_LIB_CANDIDATES)
        list(GET BGFX_LIB_CANDIDATES 0 BGFX_LIB)
        set(BGFX_LIB_FOUND TRUE)
    endif()
endif()

if(BGFX_LIB_FOUND)
    message(STATUS "Found bgfx library: ${BGFX_LIB}")

    add_library(bgfximpl STATIC IMPORTED GLOBAL)
    set_target_properties(bgfximpl PROPERTIES IMPORTED_LOCATION "${BGFX_LIB}")
    target_link_libraries(bgfx INTERFACE bgfximpl)
else()
    message(WARNING "BGFX library not found. The bgfx backend will compile but won't link.\n"
        "\n"
        "To build bgfx:\n"
        "  1. cd ${BGFX_DIR}\n"
        "  2. make linux-gcc-release64\n"
        "  3. Rebuild this project with -DENABLE_BGFX_BACKEND=ON\n"
        "\n"
        "Without the library, only the null backend can be used.")

    add_library(bgfximpl STATIC IMPORTED GLOBAL)
    set_target_properties(bgfximpl PROPERTIES
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/libbgfx_stub.a"
    )
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/libbgfx_stub.a")
        file(WRITE "${CMAKE_BINARY_DIR}/libbgfx_stub.a" "!<arch>\n")
    endif()
endif()

message(STATUS "BGFX backend enabled")
message(STATUS "  BGFX_DIR: ${BGFX_DIR}")
message(STATUS "  BX_DIR: ${BX_DIR}")
