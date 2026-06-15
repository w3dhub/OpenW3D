# find_package(Python3 COMPONENTS Interpreter)
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

    get_property(binary_dir TARGET "${TARGET}" PROPERTY BINARY_DIR)
    get_filename_component(source_name "${er_SOURCE}" NAME)
    set(er_BUILD_SOURCE "${binary_dir}/${source_name}")

    # Create empty output file and fill with boilerplate
    file(WRITE ${er_BUILD_SOURCE} "")
    file(APPEND ${er_BUILD_SOURCE} "#include \"rcfile.h\"\n")
    file(APPEND ${er_BUILD_SOURCE} "#include <array>\n")
    file(APPEND ${er_BUILD_SOURCE} "\n")
    file(APPEND ${er_BUILD_SOURCE} "#ifdef OPENW3D_SDL3\n")
    file(APPEND ${er_BUILD_SOURCE} "\n")
    file(APPEND ${er_BUILD_SOURCE} "namespace {\n")

    # Iterate through input files and build binary arrays
    foreach(bin ${er_RESOURCES})
        # Get short filename
        get_filename_component(filename "${bin}" NAME)
        #string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        file(APPEND ${er_BUILD_SOURCE} "// ${filename}\n") 
        string(TOLOWER ${filename} filename)
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        file(SIZE ${bin} filesize) 
        file(APPEND ${er_BUILD_SOURCE} "std::array<std::uint8_t, ${filesize}> var_${filename} = {\n ")

        # Convert hex data for C compatibility
        string(REGEX MATCHALL "([A-Fa-f0-9][A-Fa-f0-9])" SEPARATED_HEX ${filedata})

        # Create a counter so that we only have 16 hex bytes per line
        set(counter 0)

        # Iterate through each of the bytes from the source file
        foreach (hex IN LISTS SEPARATED_HEX)
            # Write the hex string to the line with an 0x prefix
            # and a , postfix to seperate the bytes of the file.
            string(APPEND output_c " 0x${hex},")
            # Increment the element counter before the newline.
            math(EXPR counter "${counter}+1")
            if (counter GREATER 16)
            # Write a newline so that all of the array initializer
            # gets spread across multiple lines.
                string(APPEND output_c "\n ")
                set(counter 0)
            endif ()
        endforeach ()

        # Append data to output file
        file(APPEND ${er_BUILD_SOURCE} "${output_c}\n")
        file(APPEND ${er_BUILD_SOURCE} "};\n")
        file(APPEND ${er_BUILD_SOURCE} "\n")
    endforeach()

    # Close the anonymous namespace
    file(APPEND ${er_BUILD_SOURCE} "} // namespace\n")
    file(APPEND ${er_BUILD_SOURCE} "\n")

    # Iterate through input files and build map variable initialiser
    file(APPEND ${er_BUILD_SOURCE} "std::unordered_map<std::string, StaticResourceFileClass> Static_Resources = {\n")

    foreach(bin ${er_RESOURCES})
        # Get short filename
        get_filename_component(filename "${bin}" NAME)   
        string(TOLOWER ${filename} filename)
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" varname ${filename})
        file(APPEND ${er_BUILD_SOURCE} "  {{ \"${filename}\", {{ \"${filename}\", var_${varname}.data(), var_${varname}.size(), }}, }},\n")
    endforeach()

    # Close initialiser and ifdef
    file(APPEND ${er_BUILD_SOURCE} "};\n")
    file(APPEND ${er_BUILD_SOURCE} "#endif // OPENW3D_SDL3")
    
    target_sources(${TARGET} PRIVATE ${er_BUILD_SOURCE})
endfunction()
