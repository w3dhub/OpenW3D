set(W3DVIEW_QT_SRC
    main.cpp
    MainWindow.cpp
    MainWindow.h
    AggregateNameDialog.cpp
    AggregateNameDialog.h
    AddToLineupDialog.cpp
    AddToLineupDialog.h
    AdvancedAnimationDialog.cpp
    AdvancedAnimationDialog.h
    AnimationPropertiesDialog.cpp
    AnimationPropertiesDialog.h
    AnimationSettingsDialog.cpp
    AnimationSettingsDialog.h
    AnimatedSoundOptionsDialog.cpp
    AnimatedSoundOptionsDialog.h
    BackgroundBitmapDialog.cpp
    BackgroundBitmapDialog.h
    BackgroundObjectDialog.cpp
    BackgroundObjectDialog.h
    BoneManagementDialog.cpp
    BoneManagementDialog.h
    CameraDistanceDialog.cpp
    CameraDistanceDialog.h
    CameraSettingsDialog.cpp
    CameraSettingsDialog.h
    ColorLightDialog.cpp
    ColorLightDialog.h
    EmitterEditDialog.cpp
    EmitterEditDialog.h
    ExportDirectoryDialog.cpp
    ExportDirectoryDialog.h
    GammaDialog.cpp
    GammaDialog.h
    KeyframeTableUtils.cpp
    KeyframeTableUtils.h
    HierarchyPropertiesDialog.cpp
    HierarchyPropertiesDialog.h
    RenderObjUtils.cpp
    RenderObjUtils.h
    MeshPropertiesDialog.cpp
    MeshPropertiesDialog.h
    OpacityVectorEditDialog.cpp
    OpacityVectorEditDialog.h
    ResolutionDialog.cpp
    ResolutionDialog.h
    RingEditDialog.cpp
    RingEditDialog.h
    PlaySoundDialog.cpp
    PlaySoundDialog.h
    ScaleDialog.cpp
    ScaleDialog.h
    SaveSettingsDialog.cpp
    SaveSettingsDialog.h
    SceneLightDialog.cpp
    SceneLightDialog.h
    SphereEditDialog.cpp
    SphereEditDialog.h
    SoundEditDialog.cpp
    SoundEditDialog.h
    TexturePathDialog.cpp
    TexturePathDialog.h
    W3DExportUtils.cpp
    W3DExportUtils.h
    W3DViewport.cpp
    W3DViewport.h
    W3DViewQt.qrc
    W3DViewQt.rc
)

set(W3DVIEW_QT_UI
    MainWindow.ui
    AdvancedAnimationDialog.ui
    AggregateNameDialog.ui
    AddToLineupDialog.ui
    AnimatedSoundOptionsDialog.ui
    AnimationPropertiesDialog.ui
    AnimationSettingsDialog.ui
    BackgroundBitmapDialog.ui
    BackgroundObjectDialog.ui
    BoneManagementDialog.ui
    CameraDistanceDialog.ui
    CameraSettingsDialog.ui
    ColorLightDialog.ui
    EmitterEditDialog.ui
    ExportDirectoryDialog.ui
    GammaDialog.ui
    HierarchyPropertiesDialog.ui
    MeshPropertiesDialog.ui
    OpacityVectorEditDialog.ui
    PlaySoundDialog.ui
    ResolutionDialog.ui
    RingEditDialog.ui
    ScaleDialog.ui
    SaveSettingsDialog.ui
    SceneLightDialog.ui
    SoundEditDialog.ui
    SphereEditDialog.ui
    TexturePathDialog.ui
)

add_executable(w3dview_qt WIN32)

target_sources(w3dview_qt PRIVATE ${W3DVIEW_QT_SRC} ${W3DVIEW_QT_UI})

target_include_directories(w3dview_qt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ../../wwdebug
        ../../wwlib
        ../../ww3d2
        ../../WWMath
        ../../wwphys
        ../../wwsaveload
        ../../WWAudio
)

target_link_libraries(w3dview_qt
    PRIVATE
        qtcommon
        w3d_qt_toolkit
        d3d9lib
        version
        winmm
        wwcommon
        wwdebug
        wwlib
        wwmath
        wwphys
        wwsaveload
        ww3d2
        wwaudio
)

set_target_properties(w3dview_qt PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

if(WIN32)
    set(_w3dview_qt_windeployqt_target
        "${W3D_QT_PACKAGE}::windeployqt")
    set(_w3dview_qt_windeployqt_command)

    if(TARGET "${_w3dview_qt_windeployqt_target}")
        set(_w3dview_qt_windeployqt_command
            "$<TARGET_FILE:${_w3dview_qt_windeployqt_target}>")
    else()
        set(_w3dview_qt_windeployqt_hints)
        set(_w3dview_qt_qmake_target "${W3D_QT_PACKAGE}::qmake")
        if(TARGET "${_w3dview_qt_qmake_target}")
            get_target_property(_w3dview_qt_qmake
                "${_w3dview_qt_qmake_target}" IMPORTED_LOCATION)
            if(NOT _w3dview_qt_qmake)
                get_target_property(_w3dview_qt_qmake
                    "${_w3dview_qt_qmake_target}" IMPORTED_LOCATION_RELEASE)
            endif()
            if(_w3dview_qt_qmake)
                get_filename_component(_w3dview_qt_qt_bin
                    "${_w3dview_qt_qmake}" DIRECTORY)
                list(APPEND _w3dview_qt_windeployqt_hints
                    "${_w3dview_qt_qt_bin}")
            endif()
        endif()

        find_program(W3DVIEW_QT_WINDEPLOYQT_EXECUTABLE
            NAMES windeployqt.exe windeployqt
            HINTS ${_w3dview_qt_windeployqt_hints})
        if(W3DVIEW_QT_WINDEPLOYQT_EXECUTABLE)
            set(_w3dview_qt_windeployqt_command
                "${W3DVIEW_QT_WINDEPLOYQT_EXECUTABLE}")
        endif()
    endif()

    if(_w3dview_qt_windeployqt_command)
        add_custom_command(TARGET w3dview_qt POST_BUILD
            COMMAND "${_w3dview_qt_windeployqt_command}"
                "--$<IF:$<CONFIG:Debug>,debug,release>"
                --no-translations
                --dir "$<TARGET_FILE_DIR:w3dview_qt>"
                "$<TARGET_FILE:w3dview_qt>"
            COMMENT "Deploying the Qt runtime for w3dview_qt"
            VERBATIM)
    else()
        message(WARNING
            "windeployqt was not found; w3dview_qt will require manual Qt "
            "runtime and platform-plugin deployment.")
    endif()
endif()

if(BUILD_TESTING)
    find_package(Qt6 ${W3D_QT_MIN_VERSION} COMPONENTS Test REQUIRED)
    set(W3DVIEW_QT_TEST_LIBRARY Qt6::Test)

    set(W3DVIEW_QT_PLUGIN_PATH
        "$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>/../plugins")
    if(TARGET Qt6::QOffscreenIntegrationPlugin)
        set(W3DVIEW_QT_PLUGIN_PATH
            "$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>/..")
    elseif(DEFINED QT6_INSTALL_PREFIX AND DEFINED QT6_INSTALL_PLUGINS)
        set(W3DVIEW_QT_PLUGIN_PATH
            "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}")
    endif()

    set(W3DVIEW_QT_AUDIO_TEST_ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    if(W3D_BUILD_OPTION_OPENAL)
        list(APPEND W3DVIEW_QT_AUDIO_TEST_ENVIRONMENT "ALSOFT_DRIVERS=null")
    endif()

    set(W3DVIEW_QT_MAIN_WINDOW_TEST_SRC ${W3DVIEW_QT_SRC})
    list(REMOVE_ITEM W3DVIEW_QT_MAIN_WINDOW_TEST_SRC main.cpp W3DViewQt.rc)

    add_executable(w3dview_qt_main_window_tests
        tests/MainWindowCommandTests.cpp
        ${W3DVIEW_QT_MAIN_WINDOW_TEST_SRC}
        ${W3DVIEW_QT_UI}
    )

    target_include_directories(w3dview_qt_main_window_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_main_window_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_main_window_tests PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_main_window_tests COMMAND w3dview_qt_main_window_tests)
    set_tests_properties(w3dview_qt_main_window_tests PROPERTIES
        ENVIRONMENT "${W3DVIEW_QT_AUDIO_TEST_ENVIRONMENT}"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_settings_save_mask_tests
        tests/SettingsSaveMaskTests.cpp
        ${W3DVIEW_QT_MAIN_WINDOW_TEST_SRC}
        ${W3DVIEW_QT_UI}
    )

    target_include_directories(w3dview_qt_settings_save_mask_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_settings_save_mask_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_settings_save_mask_tests PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_settings_save_mask_tests
        COMMAND w3dview_qt_settings_save_mask_tests)
    set_tests_properties(w3dview_qt_settings_save_mask_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_scene_light_tests
        tests/SceneLightTests.cpp
        RenderObjUtils.cpp
        RenderObjUtils.h
        SceneLightDialog.cpp
        SceneLightDialog.h
        SceneLightDialog.ui
        W3DViewport.cpp
        W3DViewport.h
    )

    target_include_directories(w3dview_qt_scene_light_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_scene_light_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_scene_light_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_scene_light_tests COMMAND w3dview_qt_scene_light_tests)
    set_tests_properties(w3dview_qt_scene_light_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    option(W3DVIEW_QT_ENABLE_NATIVE_VIEWPORT_TESTS
        "Build and register native W3DViewQt viewport tests that require Direct3D"
        OFF)

    if(WIN32 AND W3DVIEW_QT_ENABLE_NATIVE_VIEWPORT_TESTS)
        add_executable(w3dview_qt_viewport_fog_tests EXCLUDE_FROM_ALL
            tests/W3DViewportFogTests.cpp
            RenderObjUtils.cpp
            RenderObjUtils.h
            W3DViewport.cpp
            W3DViewport.h
        )

        target_include_directories(w3dview_qt_viewport_fog_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/..
                ${CMAKE_CURRENT_SOURCE_DIR}/../..
                ../../wwdebug
                ../../wwlib
                ../../ww3d2
                ../../WWMath
                ../../wwphys
                ../../wwsaveload
                ../../WWAudio
        )

        target_link_libraries(w3dview_qt_viewport_fog_tests
            PRIVATE
                ${W3DVIEW_QT_TEST_LIBRARY}
                qtcommon
                w3d_qt_toolkit
                d3d9lib
                version
                winmm
                wwcommon
                wwdebug
                wwlib
                wwmath
                wwphys
                wwsaveload
                ww3d2
                wwaudio
        )

        set_target_properties(w3dview_qt_viewport_fog_tests PROPERTIES
            AUTOMOC ON
        )

        add_test(NAME w3dview_qt_viewport_fog_tests COMMAND w3dview_qt_viewport_fog_tests)
        set_tests_properties(w3dview_qt_viewport_fog_tests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=windows"
            ENVIRONMENT_MODIFICATION
                "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
            LABELS "native"
        )
    endif()

    add_executable(w3dview_qt_emitter_edit_tests
        tests/EmitterEditDialogTests.cpp
        EmitterEditDialog.cpp
        EmitterEditDialog.h
        EmitterEditDialog.ui
        KeyframeTableUtils.cpp
        KeyframeTableUtils.h
    )

    target_include_directories(w3dview_qt_emitter_edit_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_emitter_edit_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_emitter_edit_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_emitter_edit_tests COMMAND w3dview_qt_emitter_edit_tests)
    set_tests_properties(w3dview_qt_emitter_edit_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_test(
        NAME w3dview_qt_designer_forms
        COMMAND ${CMAKE_COMMAND}
            "-DW3DVIEW_QT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DW3DVIEW_QT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/designer-form-test"
            "-DQT_UIC_EXECUTABLE=$<TARGET_FILE:${W3D_QT_PACKAGE}::uic>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/VerifyDesignerForms.cmake"
    )

    add_executable(w3dview_qt_primitive_shader_tests
        tests/PrimitiveShaderDialogTests.cpp
        KeyframeTableUtils.cpp
        KeyframeTableUtils.h
        OpacityVectorEditDialog.cpp
        OpacityVectorEditDialog.h
        OpacityVectorEditDialog.ui
        RingEditDialog.cpp
        RingEditDialog.h
        RingEditDialog.ui
        RenderObjUtils.cpp
        RenderObjUtils.h
        SphereEditDialog.cpp
        SphereEditDialog.h
        SphereEditDialog.ui
    )

    target_include_directories(w3dview_qt_primitive_shader_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_primitive_shader_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_primitive_shader_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_primitive_shader_tests COMMAND w3dview_qt_primitive_shader_tests)
    set_tests_properties(w3dview_qt_primitive_shader_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_background_object_dialog_tests
        tests/BackgroundObjectDialogTests.cpp
        BackgroundObjectDialog.cpp
        BackgroundObjectDialog.h
        BackgroundObjectDialog.ui
    )

    target_include_directories(w3dview_qt_background_object_dialog_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_background_object_dialog_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_background_object_dialog_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_background_object_dialog_tests
        COMMAND w3dview_qt_background_object_dialog_tests)
    set_tests_properties(w3dview_qt_background_object_dialog_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_sound_dialog_tests
        tests/SoundDialogTests.cpp
        PlaySoundDialog.cpp
        PlaySoundDialog.h
        PlaySoundDialog.ui
        SoundEditDialog.cpp
        SoundEditDialog.h
        SoundEditDialog.ui
    )

    target_include_directories(w3dview_qt_sound_dialog_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_sound_dialog_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_sound_dialog_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_sound_dialog_tests COMMAND w3dview_qt_sound_dialog_tests)
    set_tests_properties(w3dview_qt_sound_dialog_tests PROPERTIES
        ENVIRONMENT "${W3DVIEW_QT_AUDIO_TEST_ENVIRONMENT}"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_resolution_dialog_tests
        tests/ResolutionDialogTests.cpp
        ResolutionDialog.cpp
        ResolutionDialog.h
        ResolutionDialog.ui
    )

    target_include_directories(w3dview_qt_resolution_dialog_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
            ../../wwdebug
            ../../wwlib
            ../../ww3d2
            ../../WWMath
            ../../wwphys
            ../../wwsaveload
            ../../WWAudio
    )

    target_link_libraries(w3dview_qt_resolution_dialog_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            qtcommon
            w3d_qt_toolkit
            d3d9lib
            version
            winmm
            wwcommon
            wwdebug
            wwlib
            wwmath
            wwphys
            wwsaveload
            ww3d2
            wwaudio
    )

    set_target_properties(w3dview_qt_resolution_dialog_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_resolution_dialog_tests
        COMMAND w3dview_qt_resolution_dialog_tests)
    set_tests_properties(w3dview_qt_resolution_dialog_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_export_directory_dialog_tests
        tests/ExportDirectoryDialogTests.cpp
        ExportDirectoryDialog.cpp
        ExportDirectoryDialog.h
        ExportDirectoryDialog.ui
    )

    target_include_directories(w3dview_qt_export_directory_dialog_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(w3dview_qt_export_directory_dialog_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            ${W3D_QT_PACKAGE}::Widgets
    )

    set_target_properties(w3dview_qt_export_directory_dialog_tests PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
    )

    add_test(NAME w3dview_qt_export_directory_dialog_tests
        COMMAND w3dview_qt_export_directory_dialog_tests)
    set_tests_properties(w3dview_qt_export_directory_dialog_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    add_executable(w3dview_qt_export_utils_tests
        tests/W3DExportUtilsTests.cpp
        W3DExportUtils.cpp
        W3DExportUtils.h
    )

    target_include_directories(w3dview_qt_export_utils_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ../../wwlib
    )

    target_link_libraries(w3dview_qt_export_utils_tests
        PRIVATE
            ${W3DVIEW_QT_TEST_LIBRARY}
            wwcommon
            wwlib
            version
    )

    set_target_properties(w3dview_qt_export_utils_tests PROPERTIES
        AUTOMOC ON
    )

    add_test(NAME w3dview_qt_export_utils_tests COMMAND w3dview_qt_export_utils_tests)
    set_tests_properties(w3dview_qt_export_utils_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:${W3DVIEW_QT_TEST_LIBRARY}>"
    )

    set(W3DVIEW_QT_RUNTIME_TESTS
        w3dview_qt_main_window_tests
        w3dview_qt_settings_save_mask_tests
        w3dview_qt_scene_light_tests
        w3dview_qt_emitter_edit_tests
        w3dview_qt_primitive_shader_tests
        w3dview_qt_background_object_dialog_tests
        w3dview_qt_sound_dialog_tests
        w3dview_qt_resolution_dialog_tests
        w3dview_qt_export_directory_dialog_tests
        w3dview_qt_export_utils_tests
    )
    if(WIN32 AND W3DVIEW_QT_ENABLE_NATIVE_VIEWPORT_TESTS)
        list(APPEND W3DVIEW_QT_RUNTIME_TESTS w3dview_qt_viewport_fog_tests)
    endif()

    # PATH locates the Qt DLLs. Set the matching plugin root as well so GUI tests
    # cannot stall behind an invisible platform-plugin error in headless runs.
    set_property(TEST ${W3DVIEW_QT_RUNTIME_TESTS} APPEND PROPERTY
        ENVIRONMENT_MODIFICATION
            "QT_PLUGIN_PATH=set:${W3DVIEW_QT_PLUGIN_PATH}"
    )
endif()
