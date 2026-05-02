# Interface libraries to set common defines and includes, avoid duplication later
add_library(wwcommon INTERFACE)

# Build all our code as C++20 code
target_compile_features(wwcommon INTERFACE cxx_std_20)

target_compile_definitions(wwcommon INTERFACE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0601
    DIRECTINPUT_VERSION=0x800
    _CRT_NONSTDC_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    $<$<CONFIG:Debug>:WWDEBUG>
)

# Grab custom compiler flags from the environment if they aren't already defined in CMake options.
if(NOT W3D_COMPILER_FLAGS)
    set(W3D_COMPILER_FLAGS $ENV{W3D_COMPILER_FLAGS})
endif()

target_compile_options(wwcommon INTERFACE
    ${W3D_COMPILER_FLAGS}
)

target_include_directories(wwcommon INTERFACE
    BinkMovie
    Combat
    Commando
    SControl
    ww3d2
    WWAudio
    wwbitpack
    wwdebug
    wwlib
    WWMath
    wwnet
    WWOnline
    wwphys
    wwsaveload
    wwtranslatedb
    wwui
    wwutil
    wolapi
    .
)

# DirectX SDK include path (required for D3DX9 headers)
if(WIN32)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DirectX/Include")
        target_include_directories(wwcommon INTERFACE DirectX/Include)
    elseif(DEFINED ENV{DXSDK_DIR})
        target_include_directories(wwcommon INTERFACE "$ENV{DXSDK_DIR}/Include")
    endif()
endif()

target_link_libraries(wwcommon INTERFACE
    gamespy
)

if(WANT_DX9)
    target_link_libraries(wwcommon INTERFACE
        d3d9lib
    )
endif()

if(TARGET ffmpeg)
    target_link_libraries(wwcommon INTERFACE ffmpeg)
endif()

if(TARGET icu)
    target_include_directories(wwcommon INTERFACE
        wwlib/icu
    )
    target_link_libraries(wwcommon INTERFACE icu)
else()
    target_include_directories(wwcommon INTERFACE
        wwlib/wchar
    )
endif()

# Module static libraries
if(WIN32)
    add_subdirectory(BinkMovie)
endif()
add_subdirectory(Combat)
add_subdirectory(SControl)
add_subdirectory(ww3d2)
add_subdirectory(WWAudio)
add_subdirectory(wwbitpack)
add_subdirectory(wwdebug)
add_subdirectory(wwlib)
add_subdirectory(WWMath)
add_subdirectory(wwnet)
add_subdirectory(WWOnline)
add_subdirectory(wwphys)
add_subdirectory(wwsaveload)
add_subdirectory(wwtranslatedb)
add_subdirectory(wwui)
add_subdirectory(wwutil)

# Final targets
add_subdirectory(BandTest)
option(W3D_BUILD_RENDER_COMPARE "Build the RenderCompare diagnostic harness." OFF)
if(W3D_BUILD_RENDER_COMPARE)
    add_subdirectory(Tests/RenderCompare)
endif()
add_subdirectory(Launcher)
add_subdirectory(Scripts)
add_subdirectory(Commando)

if (W3D_TOOLS)
    add_subdirectory(Tools)
endif()
