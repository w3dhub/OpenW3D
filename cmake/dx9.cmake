if(WIN32)
    if(MSVC)
        FetchContent_Declare(
            dx9
            GIT_REPOSITORY https://github.com/madebr/min-dx9-sdk.git
            GIT_TAG        55973049eaaab204bb35a2b4e33a129a75a16244
        )

        FetchContent_MakeAvailable(dx9)
    else()
        add_library(d3d9lib INTERFACE)
        target_link_libraries(d3d9lib INTERFACE d3d9 d3dx9)
    endif()
else()
    find_path(DXVK_INCLUDE_PATH NAMES "dxvk/d3d9.h" REQUIRED)
    find_library(DXVK_D3D9_LIBRARY NAMES "dxvk_d3d9" REQUIRED)
    add_library(d3d9 UNKNOWN IMPORTED)
    set_property(TARGET d3d9 PROPERTY IMPORTED_LOCATION "${DXVK_D3D9_LIBRARY}")
    set_property(TARGET d3d9 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DXVK_INCLUDE_PATH}")

    add_library(d3d9lib INTERFACE)
    target_link_libraries(d3d9lib INTERFACE d3d9)
    target_include_directories(d3d9lib INTERFACE "${PROJECT_SOURCE_DIR}/Code/dxvk_wrapper")
    target_include_directories(d3d9lib INTERFACE "${DXVK_INCLUDE_PATH}/dxvk")
endif()

