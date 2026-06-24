if(WIN32)
    if(NOT MINGW)
        FetchContent_Declare(
            dx9
            GIT_REPOSITORY https://github.com/madebr/min-dx9-sdk.git
            GIT_TAG        efa003f57d5c6282c45559b50616b24f32c4f442
        )

        FetchContent_MakeAvailable(dx9)
    else()
        add_library(d3d9lib INTERFACE)
        target_link_libraries(d3d9lib INTERFACE d3d9 d3dx9)
    endif()
elseif(LINUX)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(dxvk REQUIRED IMPORTED_TARGET dxvk-d3d9)
    add_library(d3d9lib INTERFACE)
    target_link_libraries(d3d9lib INTERFACE PkgConfig::dxvk)
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

