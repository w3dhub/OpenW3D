#pragma once

#include <QList>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class QAction;
class QActionGroup;
class QCloseEvent;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
class QModelIndex;
class QPoint;
class QMenu;
class QSettings;
class QStandardItem;
class QStandardItemModel;
class QToolBar;
class QTimer;
class QTreeView;
class AudibleSoundClass;
class ParticleEmitterDefClass;
class W3DViewport;

namespace Ui {
class W3DViewMainWindow;
}

class W3DViewMainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit W3DViewMainWindow(QWidget *parent = nullptr);
    ~W3DViewMainWindow() override;
    bool openFilePath(const QString &path);
    bool loadSettingsPath(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void openRecentFile();
    void openTexturePathsDialog();
    void loadSettingsFile();
    void saveSettingsFile();
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void toggleWireframe(bool enabled);
    void toggleSorting(bool enabled);
    void toggleRestrictAnims(bool enabled);
    void toggleStatusBar(bool visible);
    void toggleBackfaceCulling(bool inverted);
    void setAmbientLight();
    void setSceneLight();
    void increaseAmbientLight();
    void decreaseAmbientLight();
    void increaseSceneLight();
    void decreaseSceneLight();
    void killSceneLight();
    void toggleLightRotateY(bool enabled);
    void toggleLightRotateYBack();
    void toggleLightRotateZ(bool enabled);
    void toggleLightRotateZBack();
    void toggleExposePrelit(bool enabled);
    void setPrelitVertex();
    void setPrelitMultipass();
    void setPrelitMultitex();
    void setBackgroundColor();
    void setBackgroundBitmap();
    void toggleFog(bool enabled);
    void setCameraFront();
    void setCameraBack();
    void setCameraLeft();
    void setCameraRight();
    void setCameraTop();
    void setCameraBottom();
    void resetCamera();
    void setCameraRotateX(bool enabled);
    void setCameraRotateY(bool enabled);
    void setCameraRotateZ(bool enabled);
    void toggleCameraAnimate(bool enabled);
    void toggleCameraResetOnDisplay(bool enabled);
    void toggleCameraBonePosX(bool enabled);
    void openCameraSettings();
    void openCameraDistance();
    void copyScreenSize();
    void changeResolution();
    void openGammaDialog();
    void toggleGammaCorrection(bool enabled);
    void toggleMungeSortOnLoad(bool enabled);
    void toggleAutoExpandAssetTree(bool enabled);
    void openBackgroundObjectDialog();
    void captureScreenshot();
    void makeMovie();
    void selectPrevAsset();
    void selectNextAsset();
    void showTreeContextMenu(const QPoint &pos);
    void startAnimation();
    void pauseAnimation();
    void stopAnimation();
    void stepAnimationForward();
    void stepAnimationBackward();
    void openAnimationSettings();
    void openAdvancedAnimation();
    void generateLod();
    void makeAggregate();
    void renameAggregate();
    void openBoneManagement();
    void autoAssignBoneModels();
    void bindSubobjectLod();
    void createEmitter();
    void scaleEmitter();
    void editEmitter();
    void createSphere();
    void createRing();
    void editPrimitive();
    void createSoundObject();
    void editSoundObject();
    void openAnimatedSoundOptions();
    void importFacialAnims();
    void exportAggregate();
    void exportEmitter();
    void exportLod();
    void exportPrimitive();
    void exportSoundObject();
    void listMissingTextures();
    void copyAssets();
    void addToLineup();
    void showAbout();
    void toggleMainToolbar(bool visible);
    void toggleObjectToolbar(bool visible);
    void toggleAnimationToolbar(bool visible);
    void recordLodScreenArea();
    void toggleLodIncludeNull(bool enabled);
    void selectPrevLod();
    void selectNextLod();
    void toggleLodAutoSwitch(bool enabled);
    void toggleObjectRotateX(bool enabled);
    void toggleObjectRotateY(bool enabled);
    void toggleObjectRotateYBack();
    void toggleObjectRotateZ(bool enabled);
    void toggleObjectRotateZBack();
    void resetObject();
    void toggleAlternateMaterials();
    void showObjectProperties();
    void setNpatchesLevel(int level);
    void toggleNpatchesGap(bool enabled);
    void updateStatusBar();

private:
    Ui::W3DViewMainWindow *_ui = nullptr;
    void updateSpecialMenu(const QModelIndex &current);
    void updateEmittersEditMenu();
    void refreshAnimationMenu();
    void refreshAggregateMenu();
    void refreshLodMenu();
    void editEmitterByName(const QString &name);
    bool commitEmitterDefinition(const ParticleEmitterDefClass &definition,
                                 const QString &registeredName,
                                 bool reloadCurrentObject,
                                 bool attachedToAggregate);
    void applySettings(QSettings &settings);
    void writeSettings(QSettings &settings, bool saveLighting, bool saveBackground) const;
    void loadQuickSettings(int slot);
    void cyclePaneFocus(bool reverse);
    void playAnimationSound();
    void stopAnimationSound();
    void applyMainToolbarIcons();
    bool loadAssetsFromFile(const QString &path);
    void rebuildAssetTree();
    void addMaterialItems(QStandardItem *parent);
    void addRenderObjectItems(QStandardItem *meshParent,
                              QStandardItem *hierarchyParent,
                              QStandardItem *hlodParent,
                              QStandardItem *collectionParent,
                              QStandardItem *aggregateParent,
                              QStandardItem *emitterParent,
                              QStandardItem *primitivesParent,
                              QStandardItem *soundParent);
    void addAnimationItems(QStandardItem *hierarchyParent,
                           QStandardItem *hlodParent,
                           QStandardItem *aggregateParent);
    void loadAppSettings();
    void loadDefaultSettings();
    void applyTexturePath(const QString &path);
    void setTexturePaths(const QString &path1, const QString &path2);
    void reloadLightmapModels();
    void reloadDisplayedObject();
    void updateRecentFilesMenu();
    void addRecentFile(const QString &path);
    bool confirmExportTarget(const QString &path);

    QTreeView *_treeView = nullptr;
    QStandardItemModel *_treeModel = nullptr;
    W3DViewport *_viewport = nullptr;
    AudibleSoundClass *_animationSound = nullptr;
    QMenu *_fileMenu = nullptr;
    QMenu *_animationMenu = nullptr;
    QMenu *_hierarchyMenu = nullptr;
    QMenu *_aggregateMenu = nullptr;
    QMenu *_lodMenu = nullptr;
    QMenu *_emittersEditMenu = nullptr;
    QLabel *_statusPolysLabel = nullptr;
    QLabel *_statusParticlesLabel = nullptr;
    QLabel *_statusCameraLabel = nullptr;
    QLabel *_statusFramesLabel = nullptr;
    QLabel *_statusFpsLabel = nullptr;
    QLabel *_statusResolutionLabel = nullptr;
    QTimer *_statusTimer = nullptr;
    QToolBar *_mainToolbar = nullptr;
    QToolBar *_objectToolbar = nullptr;
    QToolBar *_animationToolbar = nullptr;
    QAction *_toolbarMainAction = nullptr;
    QAction *_toolbarObjectAction = nullptr;
    QAction *_toolbarAnimationAction = nullptr;
    QAction *_newAction = nullptr;
    QAction *_openAction = nullptr;
    QAction *_recentFilesPlaceholderAction = nullptr;
    QList<QAction *> _recentFileActions;
    QAction *_texturePathsAction = nullptr;
    QAction *_autoExpandTreeAction = nullptr;
    QAction *_loadSettingsAction = nullptr;
    QAction *_saveSettingsAction = nullptr;
    QAction *_enableGammaAction = nullptr;
    QAction *_mungeSortAction = nullptr;
    QAction *_exportAggregateAction = nullptr;
    QAction *_exportEmitterAction = nullptr;
    QAction *_exportLodAction = nullptr;
    QAction *_exportPrimitiveAction = nullptr;
    QAction *_exportSoundObjectAction = nullptr;
    QAction *_editSoundObjectAction = nullptr;
    QAction *_editEmitterAction = nullptr;
    QAction *_scaleEmitterAction = nullptr;
    QAction *_editPrimitiveAction = nullptr;
    QAction *_listMissingTexturesAction = nullptr;
    QAction *_copyAssetsAction = nullptr;
    QAction *_addToLineupAction = nullptr;
    QAction *_aboutAction = nullptr;
    QAction *_specialMenuAction = nullptr;
    QAction *_objectMenuAction = nullptr;
    QAction *_wireframeAction = nullptr;
    QAction *_sortingAction = nullptr;
    QAction *_restrictAnimsAction = nullptr;
    QAction *_statusBarAction = nullptr;
    QAction *_fogAction = nullptr;
    QAction *_gammaAction = nullptr;
    QAction *_invertBackfaceCullingAction = nullptr;
    QAction *_backgroundObjectAction = nullptr;
    QAction *_captureScreenshotAction = nullptr;
    QAction *_makeMovieAction = nullptr;
    QAction *_slideshowPrevAction = nullptr;
    QAction *_slideshowNextAction = nullptr;
    QAction *_objectRotateXAction = nullptr;
    QAction *_objectRotateYAction = nullptr;
    QAction *_objectRotateZAction = nullptr;
    QAction *_objectResetAction = nullptr;
    QAction *_objectAlternateAction = nullptr;
    QAction *_objectPropertiesAction = nullptr;
    QAction *_animationPlayAction = nullptr;
    QAction *_animationPauseAction = nullptr;
    QAction *_animationStopAction = nullptr;
    QAction *_animationStepBackAction = nullptr;
    QAction *_animationStepForwardAction = nullptr;
    QAction *_lodRecordAction = nullptr;
    QAction *_lodIncludeNullAction = nullptr;
    QAction *_lodPrevAction = nullptr;
    QAction *_lodNextAction = nullptr;
    QAction *_lodAutoSwitchAction = nullptr;
    QAction *_aggregateBindSubobjectAction = nullptr;
    QAction *_cameraFrontAction = nullptr;
    QAction *_cameraBackAction = nullptr;
    QAction *_cameraLeftAction = nullptr;
    QAction *_cameraRightAction = nullptr;
    QAction *_cameraTopAction = nullptr;
    QAction *_cameraBottomAction = nullptr;
    QAction *_cameraRotateXAction = nullptr;
    QAction *_cameraRotateYAction = nullptr;
    QAction *_cameraRotateZAction = nullptr;
    QAction *_cameraCopyScreenAction = nullptr;
    QAction *_cameraAnimateAction = nullptr;
    QAction *_cameraResetOnDisplayAction = nullptr;
    QAction *_cameraResetAction = nullptr;
    QAction *_cameraBonePosXAction = nullptr;
    QAction *_cameraSettingsAction = nullptr;
    QAction *_cameraDistanceAction = nullptr;
    QActionGroup *_npatchesGroup = nullptr;
    QAction *_npatchesGapAction = nullptr;
    QAction *_lightRotateYAction = nullptr;
    QAction *_lightRotateZAction = nullptr;
    QAction *_exposePrelitAction = nullptr;
    QActionGroup *_prelitGroup = nullptr;
    QAction *_prelitVertexAction = nullptr;
    QAction *_prelitMultipassAction = nullptr;
    QAction *_prelitMultitexAction = nullptr;
    QString _lastOpenedPath;
    QStringList _loadedFiles;
    QString _texturePath1;
    QString _texturePath2;
    bool _restrictAnims = true;
    bool _sortingEnabled = true;
    bool _animateCamera = false;
    bool _autoResetCamera = true;
    bool _selectionIsAnimation = false;
    bool _showAnimationToolbar = true;
    bool _changingAnimationToolbarForSelection = false;
    bool _autoExpandAssetTree = true;
};
