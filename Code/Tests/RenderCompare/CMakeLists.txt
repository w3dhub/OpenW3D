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
    ${CMAKE_SOURCE_DIR}/Code/wwlib/wchar
    ${CMAKE_SOURCE_DIR}/Code/DirectX/Include
)

target_link_libraries(render_compare PRIVATE
    ww3d2
    wwlib
    wwmath
    wwdebug
    wwutil
)

if(WIN32)
    target_link_libraries(render_compare PRIVATE
        user32
        gdi32
        d3d9
        dxguid
    )
endif()

if(MSVC)
    target_link_options(render_compare PRIVATE
        "/NODEFAULTLIB:libci.lib"
    )
endif()
