cmake_minimum_required(VERSION 3.25...4.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

project(commando LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# This file handles extra settings wanted/needed for different compilers.
include(compilers)

# Set where the build results will end up
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output archives")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output binaries")

option(W3D_CLIENT "Build full game client." ON)
add_feature_info(OpenW3DClient W3D_CLIENT "Build OpenW3D game client")

option(W3D_FDS "Build as headless server." ON)
add_feature_info(OpenW3DHeadless W3D_FDS "Build OpenW3D headless server")

# Do we want to build extra SDK stuff or just the game binary?
option(W3D_TOOLS "Build additional tools." ON)
add_feature_info(OpenW3DTools W3D_TOOLS "Build OpenW3D Mod Tools")

option(SYNTAX_CHECK_ONLY "Syntax check the files only?" OFF)
add_feature_info(SyntaxCheck SYNTAX_CHECK_ONLY "Syntax check files only")
if(SYNTAX_CHECK_ONLY)
    add_compile_options("-fsyntax-only")
endif(SYNTAX_CHECK_ONLY)

# Do we want to build with SDL3 for windows and inputs?
cmake_dependent_option(W3D_BUILD_OPTION_SDL3 "Build OpenW3D with SDL3." OFF WIN32 ON)
add_feature_info(SDL3Build W3D_BUILD_OPTION_SDL3 "Build OpenW3D with SDL3")

# Do we want to build with ffmpeg for audio/video decoding?
option(W3D_BUILD_OPTION_FFMPEG "Build with ffmpeg." OFF)
add_feature_info(FFMpegBuild W3D_BUILD_OPTION_FFMPEG "Build OpenW3D with FFMpeg")

if(NOT WIN32)
    add_compile_definitions(__cdecl=)
    add_compile_definitions(__stdcall=)
    add_compile_definitions(_stdcall=)
    add_compile_definitions(stricmp=strcasecmp)
    add_compile_definitions(strnicmp=strncasecmp)
    add_compile_definitions(wcsnicmp=wcsncasecmp)
    add_compile_definitions(wcsicmp=wcscasecmp)
endif()

include(FetchContent)

if(W3D_BUILD_OPTION_FFMPEG)
    include(ffmpeg)
endif()

if(W3D_BUILD_OPTION_SDL3)
    include(sdl3)
    add_compile_definitions(-DOPENW3D_SDL3=1)
elseif(WIN32)
    add_compile_definitions(-DOPENW3D_WIN32=1)
else()
    message(FATAL_ERROR "Invalid backend")
endif()

if(WIN32)
    # Do we want to build with bink for video decoding?
    option(W3D_BUILD_OPTION_BINK "Build with bink." ON)
    add_feature_info(BinkBuild W3D_BUILD_OPTION_BINK "Build OpenW3D with Bink")

    if(W3D_BUILD_OPTION_BINK)
        include(bink)
        target_compile_definitions(binkstub INTERFACE W3D_HAS_BINK)
    endif()
    
    # Do we want to build with miles for audio playback?
    option(W3D_BUILD_OPTION_MILES "Build with miles." ON)
    add_feature_info(MilesBuild W3D_BUILD_OPTION_MILES "Build OpenW3D with Miles Audio")

    if(W3D_BUILD_OPTION_MILES)
        include(miles)
    endif()

    include(dx9)

    option(W3D_BUILD_OPTION_ICU "Build with ICU." OFF)
    add_feature_info(ICUBuild W3D_BUILD_OPTION_ICU "Build OpenW3D with ICU")

    if(W3D_BUILD_OPTION_ICU)
        include(icu)
    endif()
else()
    set(W3D_BUILD_OPTION_ICU ON)
    include(icu)
endif()

include(gamespy)

# Add main build targets
add_subdirectory(Code)

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
