# RenderCompare - Minimal side-by-side DX8 vs BGFX comparison harness

add_executable(render_compare main.cpp)

target_link_libraries(render_compare PRIVATE
    ww3d2
    wwlib
    wwmath
    wwdebug
)

target_include_directories(render_compare PRIVATE
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/ww3d2
    ${CMAKE_SOURCE_DIR}/Code/wwlib
    ${CMAKE_SOURCE_DIR}/Code/wwmath
    ${CMAKE_SOURCE_DIR}/Code/wwdebug
)

if(WIN32)
    target_link_libraries(render_compare PRIVATE
        user32
        gdi32
    )
endif()
