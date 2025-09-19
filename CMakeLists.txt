cmake_minimum_required(VERSION 3.25...4.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

project(commando LANGUAGES C CXX)

# This file handles extra settings wanted/needed for different compilers.
include(cmake/compilers.cmake)

# Set where the build results will end up
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output archives")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output binaries")

option(RENEGADE_CLIENT "Build full game client." ON)
add_feature_info(RenegadeClient RENEGADE_CLIENT "Build Renegade game client")

option(RENEGADE_FDS "Build as headless server." ON)
add_feature_info(RenegadeHeadless RENEGADE_FDS "Build Renegade headless server")

# Do we want to build extra SDK stuff or just the game binary?
option(RENEGADE_TOOLS "Build additional tools." ON)
add_feature_info(RenegadeTools RENEGADE_TOOLS "Build Renegade Mod Tools")

option(SYNTAX_CHECK_ONLY "Syntax check the files only?" OFF)
add_feature_info(SyntaxCheck SYNTAX_CHECK_ONLY "Syntax check files only")
if(SYNTAX_CHECK_ONLY)
    add_compile_options("-fsyntax-only")
endif(SYNTAX_CHECK_ONLY)

# Do we want to build with ffmpeg for audio/video decoding?
option(W3D_BUILD_OPTION_FFMPEG "Build with ffmpeg." OFF)
add_feature_info(FFMpegBuild W3D_BUILD_OPTION_FFMPEG "Build Renegade with FFMpeg")

if(NOT WIN32)
    add_compile_definitions(__cdecl=)
    add_compile_definitions(__stdcall=)
    add_compile_definitions(_stdcall=)
endif()

include(FetchContent)

if(W3D_BUILD_OPTION_FFMPEG)
    include(cmake/ffmpeg.cmake)
endif()

# Find/Add build dependencies and stubs.
# Only 32bit Windows can make use of these APIs as the libraries shipped with the game.
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        include(cmake/miles.cmake)
        # Do we want to build with bink for video decoding?
        option(W3D_BUILD_OPTION_BINK "Build with bink." ON)
        add_feature_info(BinkBuild W3D_BUILD_OPTION_BINK "Build Renegade with Bink")
        
        if(W3D_BUILD_OPTION_BINK)
            include(cmake/bink.cmake)
            target_compile_definitions(binkstub INTERFACE W3D_HAS_BINK)
        endif()
    endif()
    
    include(cmake/dx9.cmake)
endif()

include(cmake/gamespy.cmake)

# Add main build targets
add_subdirectory(Code)

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
