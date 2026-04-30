# RenderCompare - Minimal side-by-side DX8 vs BGFX comparison harness

add_executable(render_compare main.cpp)

target_link_libraries(render_compare PRIVATE
    wwcommon
    ww3d2
    wwlib
    wwmath
    wwdebug
)

if(WIN32)
    target_link_libraries(render_compare PRIVATE
        user32
        gdi32
    )
endif()
