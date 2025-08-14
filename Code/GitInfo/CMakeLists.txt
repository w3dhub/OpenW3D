# Embed some git version information in the binary.
include(GitInfo.cmake)
GitInfo(${CMAKE_SOURCE_DIR})

if(NOT W3D_BUILD_AUTHOR)
    set(W3D_BUILD_AUTHOR $ENV{W3D_BUILD_AUTHOR})
endif()

GitInfoReport()
configure_file("gitinfo.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp" @ONLY)
add_library(gitinfo OBJECT ${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp)
target_include_directories(gitinfo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
