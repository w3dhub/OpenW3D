find_package(FFMPEG REQUIRED)

add_library(ffmpeg INTERFACE)
target_include_directories(ffmpeg INTERFACE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(ffmpeg INTERFACE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(ffmpeg INTERFACE ${FFMPEG_LIBRARIES})
target_compile_definitions(ffmpeg INTERFACE W3D_HAS_FFMPEG)
