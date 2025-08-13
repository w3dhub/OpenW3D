# Embed some git version information in the binary.
set(PRE_CONFIGURE_FILE "gitinfo.cpp.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp")
if(NOT W3D_BUILD_AUTHOR)
    set(W3D_BUILD_AUTHOR $ENV{W3D_BUILD_AUTHOR})
endif()
set(GIT_BUILD_AUTHOR ${W3D_BUILD_AUTHOR})

if(W3D_EMBED_GIT_INFO)
    set(GIT_INFO_ENABLED TRUE)
endif()

include(GitWatcher.cmake)

add_library(gitinfo OBJECT ${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp)
target_include_directories(gitinfo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
