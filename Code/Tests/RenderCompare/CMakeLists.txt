# RenderCompare - Minimal side-by-side DX8 vs BGFX comparison harness

add_executable(render_compare main.cpp)

target_compile_features(render_compare PRIVATE cxx_std_20)
target_compile_definitions(render_compare PRIVATE NOMINMAX)

target_include_directories(render_compare PRIVATE
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/ww3d2
    ${CMAKE_SOURCE_DIR}/Code/wwlib
    ${CMAKE_SOURCE_DIR}/Code/wwmath
    ${CMAKE_SOURCE_DIR}/Code/wwdebug
    ${CMAKE_SOURCE_DIR}/Code/wwutil
    ${CMAKE_SOURCE_DIR}/Code/wwsaveload
    ${CMAKE_SOURCE_DIR}/Code/wwphys
    ${CMAKE_SOURCE_DIR}/Code/Combat
    ${CMAKE_SOURCE_DIR}/Code/wwui
    ${CMAKE_SOURCE_DIR}/Code/wwonline
    ${CMAKE_SOURCE_DIR}/Code/wwnet
    ${CMAKE_SOURCE_DIR}/Code/wwtranslatedb
    ${CMAKE_SOURCE_DIR}/Code/wwbitpack
    ${CMAKE_SOURCE_DIR}/Code/SControl
    ${CMAKE_SOURCE_DIR}/Code/BinkMovie
    ${CMAKE_SOURCE_DIR}/Code/wwaudio
    ${CMAKE_SOURCE_DIR}/Code/wwlib/wchar
    ${CMAKE_SOURCE_DIR}/Code/DirectX/Include
)

target_link_libraries(render_compare PRIVATE
    ww3d2
    wwlib
    wwmath
    wwdebug
    wwutil
    wwsaveload
    wwaudio
    wwphys
    combat
    binkmovie
    scontrol
    wwbitpack
    wwnet
    wwonline
    wwtranslatedb
    wwui
    gamespy
    bgfxRelease
    bimgRelease
    bxRelease
)

if(WIN32)
    target_link_libraries(render_compare PRIVATE
        user32
        gdi32
        d3d9
        d3dx9
        dinput8
        dxguid
        DxErr
        ws2_32
        winmm
        version
        vfw32
    )
endif()

if(MSVC)
    target_link_options(render_compare PRIVATE
        "/NODEFAULTLIB:libci.lib"
        "/NODEFAULTLIB:libcpmt.lib"
        "/NODEFAULTLIB:LIBCMT"
    )
endif()
