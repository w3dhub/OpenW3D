include(qt_tool_helpers)

set(W3DVIEW_QT_SRC
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
    EmitterKeyframeBar.cpp
    EmitterKeyframeBar.h
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
    W3DWriteTrackingFile.cpp
    W3DWriteTrackingFile.h
    W3DViewport.cpp
    W3DViewport.h
    W3DViewQt.qrc
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

add_library(w3dview_qt_asset_io STATIC ViewerAssetManager.cpp ViewerAssetManager.h)
target_include_directories(w3dview_qt_asset_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(w3dview_qt_asset_io
    PUBLIC w3d_qt_toolkit wwcommon
    PRIVATE ww3d2 wwlib wwaudio)

add_library(w3dview_qt_widgets OBJECT)

target_sources(w3dview_qt_widgets PRIVATE ${W3DVIEW_QT_SRC} ${W3DVIEW_QT_UI})
target_link_libraries(w3dview_qt_widgets PUBLIC w3dview_qt_asset_io)

target_include_directories(w3dview_qt_widgets
    PUBLIC
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

target_link_libraries(w3dview_qt_widgets
    PUBLIC
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

set_target_properties(w3dview_qt_widgets PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Share compiled viewer widgets and resources with the application and tests.
add_executable(w3dview_qt WIN32 main.cpp W3DViewQt.rc)
target_link_libraries(w3dview_qt PRIVATE w3dview_qt_widgets)
w3d_deploy_qt_runtime(w3dview_qt)
