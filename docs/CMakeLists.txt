cmake_minimum_required(VERSION 3.25...4.0)

# Allow documentation builds without configuring the game or its dependencies.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(OpenW3DDocs LANGUAGES NONE)
endif()

if(CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Use cmake -S docs -B build/docs.")
endif()

find_package(Doxygen REQUIRED COMPONENTS dot)
get_target_property(W3D_DOT_EXECUTABLE Doxygen::dot IMPORTED_LOCATION)
get_filename_component(W3D_DOT_DIRECTORY "${W3D_DOT_EXECUTABLE}" DIRECTORY)

configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_target(docs
    # Doxygen cannot create nested output directories on a fresh checkout.
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMAND Doxygen::doxygen "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)
