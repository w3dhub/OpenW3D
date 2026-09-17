include_guard(GLOBAL)

function(w3d_deploy_qt_runtime target)
    if(NOT WIN32)
        return()
    endif()

    if(TARGET Qt6::windeployqt)
        set(deploy_command "$<TARGET_FILE:Qt6::windeployqt>")
    else()
        set(qt_bin_dir)
        if(TARGET Qt6::qmake)
            get_target_property(qmake_path Qt6::qmake IMPORTED_LOCATION)
            if(NOT qmake_path)
                get_target_property(qmake_path Qt6::qmake IMPORTED_LOCATION_RELEASE)
            endif()
            if(qmake_path)
                get_filename_component(qt_bin_dir "${qmake_path}" DIRECTORY)
            endif()
        endif()
        find_program(W3D_WINDEPLOYQT_EXECUTABLE NAMES windeployqt.exe
            HINTS "${qt_bin_dir}" "${Qt6_DIR}/../../tools/Qt6/bin")
        set(deploy_command "${W3D_WINDEPLOYQT_EXECUTABLE}")
    endif()

    if(deploy_command)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${deploy_command}"
                "--$<IF:$<CONFIG:Debug>,debug,release>"
                --no-translations
                --dir "$<TARGET_FILE_DIR:${target}>"
                "$<TARGET_FILE:${target}>"
            COMMENT "Deploying the Qt runtime for ${target}"
            VERBATIM)
    else()
        message(WARNING "windeployqt was not found; ${target} requires manual Qt runtime deployment.")
    endif()
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

    add_test(NAME ${target} COMMAND ${target} -o -,txt)
    set_tests_properties(${target} PROPERTIES
        TIMEOUT 60
        ENVIRONMENT "${test_environment}"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Test>;QT_PLUGIN_PATH=set:${plugin_path}")
endfunction()

function(w3d_add_designer_test name)
    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            "-DQT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DQT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/designer-form-test"
            "-DQT_UI_FILES=${ARGN}"
            "-DQT_UIC_EXECUTABLE=$<TARGET_FILE:Qt6::uic>"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/verify_qt_designer_forms.cmake")
endfunction()
