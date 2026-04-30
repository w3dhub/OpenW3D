# RenderCompare - Minimal side-by-side DX8 vs BGFX comparison harness

add_executable(render_compare main.cpp)

target_link_libraries(render_compare PRIVATE
    wwcommon
    bandtest
    wwdebug
    wwlib
    wwbitpack
    scontrol
    wwutil
    wwmath
    wwnet
    wwonline
    wwsaveload
    wwtranslatedb
    wwui
    wwaudio
    ww3d2
    binkmovie
    wwphys
    combat
)

if(MSVC)
    target_link_options(render_compare PRIVATE
        "/NODEFAULTLIB:libci.lib"
        "/NODEFAULTLIB:libcpmt.lib"
        "/NODEFAULTLIB:LIBCMT"
    )
endif()
