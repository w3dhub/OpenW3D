# Viewer-specific registration; production widgets and forms remain in Code/.
get_target_property(viewer_source_dir w3dview_qt_widgets SOURCE_DIR)
get_target_property(viewer_includes w3dview_qt_widgets INCLUDE_DIRECTORIES)
get_target_property(viewer_libraries w3dview_qt_widgets LINK_LIBRARIES)
set(viewer_tests_dir "${CMAKE_CURRENT_SOURCE_DIR}/Tools/W3DViewQt")

function(w3d_add_viewer_test target)
    w3d_add_test(${target} ${ARGN})
    target_include_directories(${target} PRIVATE ${viewer_includes})
    w3d_configure_qt_test(${target})
endfunction()

add_library(w3dview_qt_test_assets STATIC
    "${viewer_tests_dir}/ExternalTestAssets.cpp"
    "${viewer_tests_dir}/ExternalTestAssets.h")
target_include_directories(w3dview_qt_test_assets PUBLIC "${viewer_tests_dir}")
target_link_libraries(w3dview_qt_test_assets
    PUBLIC w3d_qt_toolkit wwcommon PRIVATE wwlib)

w3d_add_viewer_test(w3dview_qt_main_window_tests
    SOURCES "${viewer_tests_dir}/MainWindowCommandTests.cpp"
    LIBRARIES w3dview_qt_widgets w3dview_qt_test_assets
    ARGS -o main-window-tests.txt,txt -o -,txt)
set_property(TEST w3dview_qt_main_window_tests APPEND PROPERTY LABELS external-assets)

w3d_add_viewer_test(w3dview_qt_settings_save_mask_tests
    SOURCES "${viewer_tests_dir}/SettingsSaveMaskTests.cpp"
    LIBRARIES w3dview_qt_widgets)

w3d_add_viewer_test(w3dview_qt_scene_light_tests
    SOURCES "${viewer_tests_dir}/SceneLightTests.cpp"
        "${viewer_source_dir}/RenderObjUtils.cpp"
        "${viewer_source_dir}/SceneLightDialog.cpp"
        "${viewer_source_dir}/SceneLightDialog.ui"
        "${viewer_source_dir}/W3DViewport.cpp"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_emitter_edit_tests
    SOURCES "${viewer_tests_dir}/EmitterEditDialogTests.cpp"
        "${viewer_source_dir}/EmitterEditDialog.cpp"
        "${viewer_source_dir}/EmitterEditDialog.ui"
        "${viewer_source_dir}/EmitterKeyframeBar.cpp"
        "${viewer_source_dir}/KeyframeTableUtils.cpp"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_primitive_shader_tests
    SOURCES "${viewer_tests_dir}/PrimitiveShaderDialogTests.cpp"
        "${viewer_source_dir}/KeyframeTableUtils.cpp"
        "${viewer_source_dir}/OpacityVectorEditDialog.cpp"
        "${viewer_source_dir}/OpacityVectorEditDialog.ui"
        "${viewer_source_dir}/RingEditDialog.cpp"
        "${viewer_source_dir}/RingEditDialog.ui"
        "${viewer_source_dir}/RenderObjUtils.cpp"
        "${viewer_source_dir}/SphereEditDialog.cpp"
        "${viewer_source_dir}/SphereEditDialog.ui"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_background_object_dialog_tests
    SOURCES "${viewer_tests_dir}/BackgroundObjectDialogTests.cpp"
        "${viewer_source_dir}/BackgroundObjectDialog.cpp"
        "${viewer_source_dir}/BackgroundObjectDialog.ui"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_sound_dialog_tests
    SOURCES "${viewer_tests_dir}/SoundDialogTests.cpp"
        "${viewer_source_dir}/PlaySoundDialog.cpp"
        "${viewer_source_dir}/PlaySoundDialog.ui"
        "${viewer_source_dir}/SoundEditDialog.cpp"
        "${viewer_source_dir}/SoundEditDialog.ui"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_resolution_dialog_tests
    SOURCES "${viewer_tests_dir}/ResolutionDialogTests.cpp"
        "${viewer_source_dir}/ResolutionDialog.cpp"
        "${viewer_source_dir}/ResolutionDialog.ui"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_export_directory_dialog_tests
    SOURCES "${viewer_tests_dir}/ExportDirectoryDialogTests.cpp"
        "${viewer_source_dir}/ExportDirectoryDialog.cpp"
        "${viewer_source_dir}/ExportDirectoryDialog.ui"
    LIBRARIES Qt6::Widgets)

w3d_add_viewer_test(w3dview_qt_export_utils_tests
    SOURCES "${viewer_tests_dir}/W3DExportUtilsTests.cpp"
        "${viewer_source_dir}/W3DExportUtils.cpp"
        "${viewer_source_dir}/W3DWriteTrackingFile.cpp"
    LIBRARIES wwcommon wwlib version)

w3d_add_viewer_test(w3dview_qt_asset_io_tests
    SOURCES "${viewer_tests_dir}/ViewerAssetIOTests.cpp"
        "${viewer_source_dir}/RenderObjUtils.cpp"
    LIBRARIES ${viewer_libraries})

w3d_add_viewer_test(w3dview_qt_external_assets_tests
    SOURCES "${viewer_tests_dir}/ExternalTestAssetsTests.cpp"
    LIBRARIES w3dview_qt_test_assets ${viewer_libraries})

if(W3D_BUILD_OPTION_OPENAL)
    set_property(TEST w3dview_qt_main_window_tests w3dview_qt_sound_dialog_tests
        APPEND PROPERTY ENVIRONMENT "ALSOFT_DRIVERS=null")
endif()

# Retain the viewer's checks for required menus, emitter controls, and resources
# in addition to validating its Designer forms.
add_test(NAME w3dview_qt_designer_forms
    COMMAND "${CMAKE_COMMAND}"
        "-DW3DVIEW_QT_SOURCE_DIR=${viewer_source_dir}"
        "-DW3DVIEW_QT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/w3dview_qt_designer_forms"
        "-DQT_UIC_EXECUTABLE=$<TARGET_FILE:Qt6::uic>"
        -P "${viewer_tests_dir}/VerifyDesignerForms.cmake")

option(W3DVIEW_QT_ENABLE_NATIVE_VIEWPORT_TESTS
    "Enable W3DViewQt tests that require a native Direct3D viewport." OFF)
if(WIN32 AND W3DVIEW_QT_ENABLE_NATIVE_VIEWPORT_TESTS)
    # Engine thumbnail initialization scans Data directly, even when the tests
    # use generated fixtures or load external assets through a file factory.
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Data")

    w3d_add_viewer_test(w3dview_qt_viewport_fog_tests
        SOURCES "${viewer_tests_dir}/W3DViewportFogTests.cpp"
            "${viewer_source_dir}/RenderObjUtils.cpp"
            "${viewer_source_dir}/W3DViewport.cpp"
        LIBRARIES w3dview_qt_test_assets ${viewer_libraries}
        ARGS -o viewport-fog-tests.txt,txt -o -,txt)
    set_target_properties(w3dview_qt_viewport_fog_tests PROPERTIES EXCLUDE_FROM_ALL ON)
    set_tests_properties(w3dview_qt_viewport_fog_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=windows"
        LABELS "native;external-assets")

    w3d_add_viewer_test(w3dview_qt_native_workflow_tests
        SOURCES "${viewer_tests_dir}/ViewerNativeWorkflowTests.cpp"
        LIBRARIES w3dview_qt_widgets w3dview_qt_test_assets
        ARGS -o native-workflows.txt,txt)
    set_tests_properties(w3dview_qt_native_workflow_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=windows;ALSOFT_DRIVERS=null"
        LABELS "native" TIMEOUT 90)
endif()
