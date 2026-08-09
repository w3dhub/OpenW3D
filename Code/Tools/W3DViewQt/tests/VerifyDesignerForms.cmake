if(NOT DEFINED W3DVIEW_QT_SOURCE_DIR OR NOT IS_DIRECTORY "${W3DVIEW_QT_SOURCE_DIR}")
    message(FATAL_ERROR "W3DVIEW_QT_SOURCE_DIR must name the W3DViewQt source directory")
endif()
if(NOT DEFINED W3DVIEW_QT_BINARY_DIR)
    message(FATAL_ERROR "W3DVIEW_QT_BINARY_DIR is required")
endif()
if(NOT DEFINED QT_UIC_EXECUTABLE OR NOT EXISTS "${QT_UIC_EXECUTABLE}")
    message(FATAL_ERROR "QT_UIC_EXECUTABLE does not exist: ${QT_UIC_EXECUTABLE}")
endif()

set(expected_forms
    AddToLineupDialog.ui
    AdvancedAnimationDialog.ui
    AggregateNameDialog.ui
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
    MainWindow.ui
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
list(SORT expected_forms)

file(GLOB actual_forms RELATIVE "${W3DVIEW_QT_SOURCE_DIR}" "${W3DVIEW_QT_SOURCE_DIR}/*.ui")
list(SORT actual_forms)
if(NOT actual_forms STREQUAL expected_forms)
    message(FATAL_ERROR
        "Designer form set differs from the required one.\nExpected: ${expected_forms}\nActual: ${actual_forms}")
endif()

file(READ "${W3DVIEW_QT_SOURCE_DIR}/CMakeLists.txt" source_manifest)
file(MAKE_DIRECTORY "${W3DVIEW_QT_BINARY_DIR}")
foreach(form IN LISTS expected_forms)
    string(FIND "${source_manifest}" "    ${form}" manifest_index)
    if(manifest_index EQUAL -1)
        message(FATAL_ERROR "${form} is not listed in the W3DViewQt CMake source manifest")
    endif()

    get_filename_component(form_name "${form}" NAME_WE)
    execute_process(
        COMMAND "${QT_UIC_EXECUTABLE}"
            -o "${W3DVIEW_QT_BINARY_DIR}/ui_${form_name}.h"
            "${W3DVIEW_QT_SOURCE_DIR}/${form}"
        RESULT_VARIABLE uic_result
        OUTPUT_VARIABLE uic_output
        ERROR_VARIABLE uic_error
    )
    if(NOT uic_result EQUAL 0)
        message(FATAL_ERROR "uic failed for ${form}:\n${uic_output}\n${uic_error}")
    endif()
endforeach()

file(GLOB dialog_sources "${W3DVIEW_QT_SOURCE_DIR}/*Dialog.cpp")
list(APPEND dialog_sources "${W3DVIEW_QT_SOURCE_DIR}/MainWindow.cpp")
foreach(source IN LISTS dialog_sources)
    file(READ "${source}" source_text)
    if(source_text MATCHES "new[ \t\r\n]+Q(VBox|HBox|Grid|Form|Stacked)Layout")
        message(FATAL_ERROR "Runtime layout construction remains in ${source}")
    endif()
endforeach()

file(READ "${W3DVIEW_QT_SOURCE_DIR}/MainWindow.ui" main_window_form)
foreach(required_object IN ITEMS
        menuBar MainToolbar ObjectToolbar AnimationToolbar mainSplitter assetTreeView viewport
        permanentStatusPanel statusPolysLabel statusParticlesLabel statusCameraLabel
        statusFramesLabel statusFpsLabel statusResolutionLabel actionChangeResolution)
    string(FIND "${main_window_form}" "name=\"${required_object}\"" object_index)
    if(object_index EQUAL -1)
        message(FATAL_ERROR "MainWindow.ui is missing ${required_object}")
    endif()
endforeach()

file(READ "${W3DVIEW_QT_SOURCE_DIR}/EmitterEditDialog.ui" emitter_form)
string(REGEX MATCHALL "<attribute name=\"title\">" emitter_tab_titles "${emitter_form}")
list(LENGTH emitter_tab_titles emitter_tab_count)
if(NOT emitter_tab_count EQUAL 10)
    message(FATAL_ERROR "EmitterEditDialog.ui must contain exactly 10 property tabs; found ${emitter_tab_count}")
endif()
foreach(tab_title IN ITEMS General Particle Physics Color Size User Line Rotation Frame "Line Group")
    string(FIND "${emitter_form}" "<attribute name=\"title\"><string>${tab_title}</string></attribute>" tab_index)
    if(tab_index EQUAL -1)
        message(FATAL_ERROR "EmitterEditDialog.ui is missing the ${tab_title} tab")
    endif()
endforeach()

file(READ "${W3DVIEW_QT_SOURCE_DIR}/W3DViewQt.qrc" resource_manifest)
foreach(resource_alias IN ITEMS app.ico main-toolbar.bmp play.bmp pause.bmp stop.bmp reverse.bmp ffwd.bmp)
    string(FIND "${resource_manifest}" "alias=\"${resource_alias}\"" resource_index)
    if(resource_index EQUAL -1)
        message(FATAL_ERROR "W3DViewQt.qrc is missing ${resource_alias}")
    endif()
endforeach()

message(STATUS "Validated ${emitter_tab_count} emitter tabs and all 28 Designer forms")
