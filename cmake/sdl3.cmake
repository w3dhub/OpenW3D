# SDL3 configuration for OpenW3D
# Supports both native builds (find_package) and MinGW cross-compilation (bundled)

if(NOT W3D_BUILD_OPTION_SDL3)
    return()
endif()

if(CMAKE_CROSSCOMPILING AND MINGW)
    # MinGW cross-compilation: use bundled SDL3 development library
    set(SDL3_ROOT "${CMAKE_SOURCE_DIR}/external/sdl3")
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/include")
    set(SDL3_LIBRARY "${SDL3_ROOT}/lib/libSDL3.dll.a")
    
    if(NOT EXISTS ${SDL3_LIBRARY})
        message(FATAL_ERROR "SDL3 MinGW library not found at ${SDL3_LIBRARY}. "
                "Please download SDL3-devel-3.4.8-mingw.tar.gz and extract to external/sdl3/")
    endif()
    
    add_library(SDL3::SDL3 SHARED IMPORTED)
    set_target_properties(SDL3::SDL3 PROPERTIES
        IMPORTED_LOCATION "${SDL3_ROOT}/bin/SDL3.dll"
        IMPORTED_IMPLIB "${SDL3_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}"
    )
    add_library(SDL3::SDL3-shared ALIAS SDL3::SDL3)
    add_library(SDL3::Headers INTERFACE IMPORTED)
    set_target_properties(SDL3::Headers PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}"
    )
    
    include_directories("${SDL3_INCLUDE_DIR}")
    
    message(STATUS "Using bundled SDL3 for MinGW cross-compilation: ${SDL3_ROOT}")
else()
    # Native build: use system SDL3 (REQUIRED for DXVK)
    find_package(SDL3 REQUIRED COMPONENTS SDL3-shared Headers)
endif()
