find_package(Python3 COMPONENTS Interpreter)
function(embed_resources TARGET)
    cmake_parse_arguments(er "" "SOURCE" "RESOURCES" ${ARGN})
    get_property(openw3d_resources TARGET "${TARGET}" PROPERTY INTERFACE_OPENW3D_RESOURCES SET)
    if(openw3d_resources)
        message(FATAL_ERROR "Can only use embed_resources ONCE on a target")
    endif()
    set_property(TARGET ${TARGET} PROPERTY INTERFACE_OPENW3D_RESOURCES "1")
    if(er_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments: ${er_UNARSED_ARGUMENTS}")
    endif()
    if(NOT er_SOURCE)
        message(FATAL_ERROR "Missing required argument: SOURCE")
    endif()
    if(NOT er_RESOURCES)
        message(FATAL_ERROR "Missing required argument: RESOURCES")
    endif()
    if(Python3_FOUND)
        # Use intermediate file so `make clean` does not remove the final file
        get_filename_component(source_name "${er_SOURCE}" NAME)
        get_property(binary_dir TARGET "${TARGET}" PROPERTY BINARY_DIR)
        set(er_BUILD_SOURCE "${binary_dir}/${source_name}")

        add_custom_command(OUTPUT "${er_BUILD_SOURCE}"
            COMMAND Python3::Interpreter "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed.py" -o "${er_BUILD_SOURCE}" ${er_RESOURCES}
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${er_BUILD_SOURCE}" "${er_SOURCE}"
            DEPENDS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed.py" ${er_RESOURCES}
        )
        add_custom_target(gen-${TARGET}-resources DEPENDS "${er_BUILD_SOURCE}")
        add_dependencies(${TARGET} gen-${TARGET}-resources)
    endif()
    get_property(gen1 SOURCE "${er_SOURCE}" PROPERTY GENERATED)
    get_property(gen2 SOURCE "${er_BUILD_SOURCE}" PROPERTY GENERATED)
    message(STATUS "gen1=${gen1} gen2=${gen2}")

#    set_property(SOURCE "${er_FILE}" PROPERTY GENERATED 0)
#
#    get_property(gen1 SOURCE "${er_SOURCE}" PROPERTY GENERATED)
#    get_property(gen2 SOURCE "${er_BUILD_SOURCE}" PROPERTY GENERATED)
#    message(FATAL_ERROR "gen1=${gen1} gen2=${gen2}")
    target_sources(${TARGET} PRIVATE ${er_SOURCE})
endfunction()
