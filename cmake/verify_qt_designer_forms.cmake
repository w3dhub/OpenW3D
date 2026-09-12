if(NOT IS_DIRECTORY "${QT_SOURCE_DIR}" OR NOT QT_BINARY_DIR OR NOT QT_UI_FILES)
    message(FATAL_ERROR "A source directory, binary directory, and form list are required")
endif()
if(NOT EXISTS "${QT_UIC_EXECUTABLE}")
    message(FATAL_ERROR "Qt uic does not exist: ${QT_UIC_EXECUTABLE}")
endif()

file(GLOB actual_forms RELATIVE "${QT_SOURCE_DIR}" "${QT_SOURCE_DIR}/*.ui")
list(SORT actual_forms)
list(SORT QT_UI_FILES)
if(NOT actual_forms STREQUAL QT_UI_FILES)
    message(FATAL_ERROR "Designer forms differ from the CMake manifest: ${actual_forms}; expected ${QT_UI_FILES}")
endif()

file(MAKE_DIRECTORY "${QT_BINARY_DIR}")
foreach(form IN LISTS QT_UI_FILES)
    get_filename_component(form_name "${form}" NAME_WE)
    execute_process(
        COMMAND "${QT_UIC_EXECUTABLE}" -o "${QT_BINARY_DIR}/ui_${form_name}.h" "${QT_SOURCE_DIR}/${form}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error)
    if(NOT result EQUAL 0 OR NOT error STREQUAL "")
        message(FATAL_ERROR "uic failed or warned for ${form}:\n${output}\n${error}")
    endif()

    file(READ "${QT_SOURCE_DIR}/${form_name}.cpp" source_text)
    if(source_text MATCHES "new[ \t\r\n]+Q(VBox|HBox|Grid|Form|Stacked)Layout")
        message(FATAL_ERROR "Runtime layout construction remains in ${form_name}.cpp")
    endif()
endforeach()

list(LENGTH QT_UI_FILES form_count)
message(STATUS "Validated all ${form_count} Designer forms")
