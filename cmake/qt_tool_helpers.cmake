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
