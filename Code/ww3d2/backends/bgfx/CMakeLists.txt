# BGFX Shader Build Rules - CMake integration for shader compilation.
#
# This CMakeLists.txt handles:
#   - Build-time shader compilation using shaderc
#   - Runtime fallback for loading pre-compiled shaders
#   - Hot-reload support for development

# Shader compilation setup
set(BGFX_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(BGFX_SHADER_SOURCE_DIR "${BGFX_SHADER_DIR}/source")
set(BGFX_SHADER_EMBED_DIR "${BGFX_SHADER_DIR}/embed")
set(BGFX_SHADER_VARYING_DEF "${BGFX_SHADER_DIR}/varying.def.sc")
set(BGFX_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

# Find shaderc compiler
find_program(SHADERC_EXECUTABLE
    NAMES shaderc shadercRelease
    PATHS
        "${BGFX_DIR}/.build/linux64_gcc/bin"
        "${BGFX_DIR}/.build/win64_vs2022/bin"
        "${BGFX_DIR}/.build/osx64_clang/bin"
    DOC "bgfx shader compiler"
)

if(NOT SHADERC_EXECUTABLE)
    message(WARNING "shaderc not found. Shaders will need to be compiled manually.")
endif()

# Shader profiles for different renderers
if(WIN32)
    set(SHADER_PROFILES
        "s_5_0"     # Direct3D 11 / 12
    )
    set(SHADER_PLATFORM "windows")
else()
    set(SHADER_PROFILES
        "120"       # OpenGL 2.1 / GLES 2.0
        " spirv"    # Vulkan (note leading space)
    )
    set(SHADER_PLATFORM "linux")
endif()

# Source shaders
set(SHADER_SOURCES
    "${BGFX_SHADER_SOURCE_DIR}/vs_mesh.sc"
    "${BGFX_SHADER_SOURCE_DIR}/fs_mesh.sc"
)

# Function to compile a shader
function(compile_bgfx_shader input_file output_file type profile)
    if(NOT SHADERC_EXECUTABLE)
        return()
    endif()

    get_filename_component(filename ${input_file} NAME)
    
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${SHADERC_EXECUTABLE}
            -f ${input_file}
            -o ${output_file}
            --type ${type}
            --platform ${SHADER_PLATFORM}
            --profile ${profile}
            --varyingdef ${BGFX_SHADER_VARYING_DEF}
            -i "${BGFX_SHADER_SOURCE_DIR}"
            -i "${BGFX_DIR}/src"
            -i "${BGFX_DIR}/include"
            -i "${BGFX_DIR}/examples/common"
        DEPENDS ${input_file}
        COMMENT "Compiling bgfx shader: ${filename}"
        VERBATIM
    )
endfunction()

# Compile all shaders
set(COMPILED_SHADERS)

foreach(SHADER_SRC ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_SRC} NAME_WE)
    get_filename_component(SHADER_EXT ${SHADER_SRC} EXT)
    
    # Determine shader type from extension
    if(SHADER_EXT STREQUAL ".sc")
        if(SHADER_NAME MATCHES "^vs_")
            set(SHADER_TYPE "vertex")
        elseif(SHADER_NAME MATCHES "^fs_")
            set(SHADER_TYPE "fragment")
        elseif(SHADER_NAME MATCHES "^cs_")
            set(SHADER_TYPE "compute")
        else()
            set(SHADER_TYPE "vertex")
        endif()
    endif()
    
    # Compile for each profile
    foreach(PROFILE ${SHADER_PROFILES})
        string(STRIP "${PROFILE}" PROFILE_STRIPPED)
        set(OUTPUT_FILE "${BGFX_SHADER_OUTPUT_DIR}/${SHADER_NAME}_${PROFILE_STRIPPED}.bin")
        compile_bgfx_shader(${SHADER_SRC} ${OUTPUT_FILE} ${SHADER_TYPE} ${PROFILE_STRIPPED})
        list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
    endforeach()
endforeach()

# Create shader output directory
file(MAKE_DIRECTORY ${BGFX_SHADER_OUTPUT_DIR})

# Add custom target for shader compilation
add_custom_target(bgfx-shaders ALL DEPENDS ${COMPILED_SHADERS})

# Copy embedded shaders if they exist
if(EXISTS ${BGFX_SHADER_EMBED_DIR})
    file(GLOB EMBEDDED_SHADERS "${BGFX_SHADER_EMBED_DIR}/*.bin")
    if(EMBEDDED_SHADERS)
        foreach(EMBED ${EMBEDDED_SHADERS})
            get_filename_component(EMBED_NAME ${EMBED} NAME)
            set(OUTPUT "${BGFX_SHADER_OUTPUT_DIR}/${EMBED_NAME}")
            add_custom_command(
                OUTPUT ${OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy ${EMBED} ${OUTPUT}
                DEPENDS ${EMBED}
                COMMENT "Copying embedded shader: ${EMBED_NAME}"
            )
            list(APPEND COMPILED_SHADERS ${OUTPUT})
        endforeach()
    endif()
endif()

# Export shader directory for use by the main build
set(BGFX_SHADER_OUTPUT_DIR "${BGFX_SHADER_OUTPUT_DIR}" PARENT_SCOPE)

message(STATUS "BGFX shader output: ${BGFX_SHADER_OUTPUT_DIR}")
