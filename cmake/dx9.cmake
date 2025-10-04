if(MSVC)
    FetchContent_Declare(
        dx9
        GIT_REPOSITORY https://github.com/madebr/min-dx9-sdk.git
        GIT_TAG        55973049eaaab204bb35a2b4e33a129a75a16244
    )

    FetchContent_MakeAvailable(dx9)
else()
    add_library(d3d9lib INTERFACE)
    target_link_libraries(d3d9lib INTERFACE d3d9 d3dx9 dinput8 dxguid)
endif()
