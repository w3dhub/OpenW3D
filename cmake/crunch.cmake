FetchContent_Declare(
    crunch
    GIT_REPOSITORY https://github.com/DaemonEngine/crunch.git
    GIT_TAG        7811f177f97e0143c3b593d6ca8c6a5b2c80e412
    #unvanquished/0.56.2
)

set(BUILD_STATIC_LIBCRN ON)
set(BUILD_CRUNCH OFF)

FetchContent_MakeAvailable(crunch)
add_library(crnlib INTERFACE)
target_link_libraries(crnlib INTERFACE crn)
target_include_directories(crnlib INTERFACE ${crunch_SOURCE_DIR}/inc)
