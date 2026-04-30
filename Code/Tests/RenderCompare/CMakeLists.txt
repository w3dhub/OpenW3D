# RenderCompare - Minimal side-by-side DX8 vs BGFX comparison harness

add_executable(render_compare main.cpp)

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
