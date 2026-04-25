# Use the same headers that dxvk uses
FetchContent_Declare(
    dx9
    GIT_REPOSITORY https://github.com/misyltoad/mingw-directx-headers.git
    GIT_TAG        9df86f2341616ef1888ae59919feaa6d4fad693d
)

FetchContent_MakeAvailable(dx9)

add_library(d3d9lib INTERFACE)
target_include_directories(d3d9lib INTERFACE "${dx9_SOURCE_DIR}")
