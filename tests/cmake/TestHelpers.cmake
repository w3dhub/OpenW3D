include_guard(GLOBAL)

function(w3d_add_test target)
    cmake_parse_arguments(PARSE_ARGV 1 test "" "" "SOURCES;LIBRARIES;ARGS")
    if(test_UNPARSED_ARGUMENTS OR test_KEYWORDS_MISSING_VALUES OR NOT test_SOURCES)
        message(FATAL_ERROR "Invalid arguments for test ${target}")
    endif()
    add_executable(${target} ${test_SOURCES})
    if(test_LIBRARIES)
        target_link_libraries(${target} PRIVATE ${test_LIBRARIES})
    endif()
    add_test(NAME ${target} COMMAND ${target} ${test_ARGS})
    add_dependencies(w3d_tests ${target})
endfunction()

function(w3d_configure_qt_test target)
    target_link_libraries(${target} PRIVATE Qt6::Test)
    set_target_properties(${target} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

    set(plugin_path "$<TARGET_FILE_DIR:Qt6::Test>/../plugins")
    if(TARGET Qt6::QOffscreenIntegrationPlugin)
        set(plugin_path "$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>/..")
    elseif(DEFINED QT6_INSTALL_PREFIX AND DEFINED QT6_INSTALL_PLUGINS)
        set(plugin_path "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}")
    endif()

    set(test_environment "QT_QPA_PLATFORM=offscreen")
    if(WIN32 AND DEFINED ENV{WINDIR})
        list(APPEND test_environment "QT_QPA_FONTDIR=$ENV{WINDIR}/Fonts")
    endif()
    set_tests_properties(${target} PROPERTIES
        TIMEOUT 60
        ENVIRONMENT "${test_environment}"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Test>;QT_PLUGIN_PATH=set:${plugin_path}")
endfunction()

function(w3d_add_designer_test name target)
    # Read the production target's form list instead of maintaining a second one.
    get_target_property(source_dir ${target} SOURCE_DIR)
    get_target_property(sources ${target} SOURCES)
    set(ui_files)
    foreach(source IN LISTS sources)
        if(source MATCHES "\\.ui$")
            if(IS_ABSOLUTE "${source}")
                file(RELATIVE_PATH source "${source_dir}" "${source}")
            endif()
            list(APPEND ui_files "${source}")
        endif()
    endforeach()
    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            "-DQT_SOURCE_DIR=${source_dir}"
            "-DQT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/${name}"
            "-DQT_UI_FILES=${ui_files}"
            "-DQT_UIC_EXECUTABLE=$<TARGET_FILE:Qt6::uic>"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/verify_qt_designer_forms.cmake")
endfunction()
