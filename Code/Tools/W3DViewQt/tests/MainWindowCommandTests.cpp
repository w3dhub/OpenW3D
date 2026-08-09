#include "MainWindow.h"
#include "AdvancedAnimationDialog.h"
#include "AnimationPropertiesDialog.h"
#include "AnimationSettingsDialog.h"
#include "ExportDirectoryDialog.h"
#include "RenderObjUtils.h"
#include "SoundEditDialog.h"
#include "SaveSettingsDialog.h"
#include "W3DExportUtils.h"
#include "W3DViewport.h"
#include "Sound3D.h"
#include "WWAudio.h"
#include "agg_def.h"
#include "assetmgr.h"
#include "chunkio.h"
#include "hanim.h"
#include "hlod.h"
#include "htree.h"
#include "part_ldr.h"
#include "ramfile.h"
#include "rawfile.h"
#include "rendobj.h"
#include "ringobj.h"
#include "soundrobj.h"
#include "sphereobj.h"
#include "vector3.h"
#include "w3d_file.h"
#include "wwmath.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QSettings>
#include <QSlider>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QtTest/QTest>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>

namespace {
QStringList toStringList(std::initializer_list<const char *> values)
{
    QStringList result;
    result.reserve(static_cast<int>(values.size()));
    for (const char *value : values) {
        result.append(QString::fromLatin1(value));
    }
    return result;
}

QStringList commandIds(const QList<QAction *> &actions)
{
    QStringList result;
    result.reserve(actions.size());
    for (QAction *action : actions) {
        if (action->isSeparator()) {
            result.append("<separator>");
        } else if (QMenu *menu = action->menu()) {
            result.append("menu:" + menu->objectName());
        } else {
            result.append(action->objectName());
        }
    }
    return result;
}

QStringList portableShortcuts(const QAction &action)
{
    QStringList result;
    for (const QKeySequence &shortcut : action.shortcuts()) {
        result.append(shortcut.toString(QKeySequence::PortableText));
    }
    return result;
}

template<size_t Size>
void setW3dName(char (&destination)[Size], const char *source)
{
    static_assert(Size > 0);
    std::memset(destination, 0, Size);
    if (!source) {
        return;
    }

    size_t length = std::strlen(source);
    if (length >= Size) {
        length = Size - 1;
    }
    std::memcpy(destination, source, length);
}

bool writeDataChunk(ChunkSaveClass &save,
                    uint32 chunkId,
                    const void *data,
                    size_t size)
{
    if (!save.Begin_Chunk(chunkId)) {
        return false;
    }

    const bool wroteData = save.Write(data, size) == size;
    const bool endedChunk = save.End_Chunk();
    return wroteData && endedChunk;
}

template<typename Struct>
bool writeDataChunk(ChunkSaveClass &save, uint32 chunkId, const Struct &data)
{
    return writeDataChunk(save, chunkId, &data, sizeof(data));
}

bool writeHierarchy(ChunkSaveClass &save, const char *name)
{
    W3dHierarchyStruct header = {};
    header.Version = W3D_CURRENT_HTREE_VERSION;
    setW3dName(header.Name, name);
    header.NumPivots = 1;

    W3dPivotStruct pivot = {};
    setW3dName(pivot.Name, "ROOTTRANSFORM");
    pivot.ParentIdx = static_cast<uint32>(-1);
    pivot.Rotation.Q[3] = 1.0f;

    if (!save.Begin_Chunk(W3D_CHUNK_HIERARCHY)) {
        return false;
    }

    const bool wroteHeader = writeDataChunk(save, W3D_CHUNK_HIERARCHY_HEADER, header);
    const bool wrotePivots = wroteHeader && writeDataChunk(save, W3D_CHUNK_PIVOTS, pivot);
    const bool endedHierarchy = save.End_Chunk();
    return wroteHeader && wrotePivots && endedHierarchy;
}

bool writeHierarchyModel(ChunkSaveClass &save,
                         const char *modelName,
                         const char *hierarchyName)
{
    W3dHModelHeaderStruct header = {};
    header.Version = W3D_CURRENT_HMODEL_VERSION;
    setW3dName(header.Name, modelName);
    setW3dName(header.HierarchyName, hierarchyName);
    header.NumConnections = 0;

    if (!save.Begin_Chunk(W3D_CHUNK_HMODEL)) {
        return false;
    }

    const bool wroteHeader = writeDataChunk(save, W3D_CHUNK_HMODEL_HEADER, header);
    const bool endedModel = save.End_Chunk();
    return wroteHeader && endedModel;
}

bool writeRawAnimation(ChunkSaveClass &save,
                       const char *animationName,
                       const char *hierarchyName)
{
    W3dAnimHeaderStruct header = {};
    header.Version = W3D_CURRENT_HANIM_VERSION;
    setW3dName(header.Name, animationName);
    setW3dName(header.HierarchyName, hierarchyName);
    header.NumFrames = 2;
    header.FrameRate = 30;

    constexpr size_t channelSize =
        offsetof(W3dAnimChannelStruct, Data) + (2 * sizeof(float32));
    static_assert(sizeof(W3dAnimChannelStruct) <= channelSize);
    std::array<unsigned char, channelSize> channelBytes = {};

    W3dAnimChannelStruct channel = {};
    channel.FirstFrame = 0;
    channel.LastFrame = 1;
    channel.VectorLen = 1;
    channel.Flags = ANIM_CHANNEL_X;
    channel.Pivot = 0;
    channel.Data[0] = 0.0f;
    std::memcpy(channelBytes.data(), &channel, sizeof(channel));

    const float32 secondFrame = 1.25f;
    std::memcpy(channelBytes.data() + offsetof(W3dAnimChannelStruct, Data) + sizeof(float32),
                &secondFrame,
                sizeof(secondFrame));

    if (!save.Begin_Chunk(W3D_CHUNK_ANIMATION)) {
        return false;
    }

    const bool wroteHeader = writeDataChunk(save, W3D_CHUNK_ANIMATION_HEADER, header);
    const bool wroteChannel = wroteHeader &&
        writeDataChunk(save,
                       W3D_CHUNK_ANIMATION_CHANNEL,
                       channelBytes.data(),
                       channelBytes.size());
    const bool endedAnimation = save.End_Chunk();
    return wroteHeader && wroteChannel && endedAnimation;
}

bool writeGeneratedAnimationFixture(const QString &path)
{
    const QByteArray nativePath = QDir::toNativeSeparators(path).toLocal8Bit();
    RawFileClass file(nativePath.constData());
    if (!file.Open(FileClass::WRITE)) {
        return false;
    }

    ChunkSaveClass save(&file);
    const bool result = writeHierarchy(save, "TEST_RIG") &&
        writeHierarchy(save, "OTHER_RIG") &&
        writeHierarchyModel(save, "TEST_MODEL", "TEST_RIG") &&
        writeHierarchyModel(save, "OTHER_MODEL", "OTHER_RIG") &&
        writeRawAnimation(save, "TEST_MOVE", "TEST_RIG");
    file.Close();
    return result;
}

QStringList collectAssetNames(AssetIterator *iterator)
{
    std::unique_ptr<AssetIterator> ownedIterator(iterator);
    QStringList names;
    if (!ownedIterator) {
        return names;
    }

    for (ownedIterator->First(); !ownedIterator->Is_Done(); ownedIterator->Next()) {
        const char *name = ownedIterator->Current_Item_Name();
        if (name && name[0]) {
            names.append(QString::fromLatin1(name));
        }
    }
    return names;
}

QModelIndex findDirectChild(const QAbstractItemModel *model,
                            const QModelIndex &parent,
                            const QString &text)
{
    if (!model || !parent.isValid()) {
        return {};
    }

    for (int row = 0; row < model->rowCount(parent); ++row) {
        const QModelIndex candidate = model->index(row, 0, parent);
        if (candidate.data().toString() == text) {
            return candidate;
        }
    }
    return {};
}

template<typename Type>
struct ReleaseRef
{
    void operator()(Type *value) const
    {
        if (value) {
            value->Release_Ref();
        }
    }
};

class CurrentDirectoryRestorer final
{
public:
    CurrentDirectoryRestorer()
        : _path(QDir::currentPath())
    {
    }

    ~CurrentDirectoryRestorer()
    {
        if (!_path.isEmpty()) {
            QDir::setCurrent(_path);
        }
    }

private:
    QString _path;
};

class OneShotShortWriteRAMFile final : public RAMFileClass
{
public:
    OneShotShortWriteRAMFile(void *buffer, int length)
        : RAMFileClass(buffer, length)
    {
    }

    int Write(const void *buffer, int size) override
    {
        // Chunk headers and microchunk headers have different sizes. The first
        // four-byte write is the first sound-definition variable payload.
        if (!_failed && size == static_cast<int>(sizeof(float))) {
            _failed = true;
            return RAMFileClass::Write(buffer, size - 1);
        }
        return RAMFileClass::Write(buffer, size);
    }

    bool failed() const { return _failed; }

private:
    bool _failed = false;
};
} // namespace

class MainWindowCommandTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void designerHierarchyAndTopLevelMenus();
    void staticMenuCommandOrder();
    void toolbarCommandOrder();
    void dynamicMenusAndActionGroups();
    void shortcutParity();
    void emptyStartupCommandState();
    void selectionSpecificMenusAndEmptyPlaceholders();
    void recentFilesMatchMfcPresentationAndLimit();
    void settingsFilesAreValidatedBeforeApply();
    void relativeSettingsPathResolvesBesideExecutable();
    void emptyTexturePathsRemainEmpty();
    void startupManualClipDefaultsMatchMfc();
    void safeActionWiring();
    void restoredToolbarStateStaysSynchronized();
    void aggregateSubobjectNamesAreBounded();
    void soundPrototypeRegistrationRejectsCollisions();
    void soundSerializerReportsOneShotWriteFailure();
    void generatedHierarchyAnimationFixture();
    void externalAnimationAssetBundle();
    void externalRealAssetBundle();

private:
    QAction *action(const char *objectName) const;
    void compareMenu(const char *objectName,
                     std::initializer_list<const char *> expected) const;
    void compareToolbar(const char *objectName,
                        std::initializer_list<const char *> expected) const;
    QModelIndex findRootItem(const QString &prefix) const;

    std::unique_ptr<QTemporaryDir> _settingsDirectory;
    std::unique_ptr<W3DViewMainWindow> _window;
};

void MainWindowCommandTests::initTestCase()
{
    _settingsDirectory = std::make_unique<QTemporaryDir>();
    QVERIFY2(_settingsDirectory->isValid(), "Could not create an isolated settings directory");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, _settingsDirectory->path());
    QCoreApplication::setOrganizationName("OpenW3DTests");
    QCoreApplication::setApplicationName("W3DViewQtMainWindowCommandTests");

    _window = std::make_unique<W3DViewMainWindow>();
    QVERIFY(_window);
    QVERIFY2(!_window->isVisible(), "The offscreen command test must never show the main window");

    for (QTimer *timer : _window->findChildren<QTimer *>()) {
        timer->stop();
    }
}

QAction *MainWindowCommandTests::action(const char *objectName) const
{
    QAction *result = _window->findChild<QAction *>(objectName);
    if (!result) {
        QTest::qFail(qPrintable(QString("Missing action: %1").arg(objectName)), __FILE__, __LINE__);
    }
    return result;
}

void MainWindowCommandTests::compareMenu(
    const char *objectName, std::initializer_list<const char *> expected) const
{
    QMenu *menu = _window->findChild<QMenu *>(objectName);
    QVERIFY2(menu, objectName);
    const QStringList actual = commandIds(menu->actions());
    const QStringList expectedCommands = toStringList(expected);
    QVERIFY2(actual == expectedCommands,
             qPrintable(QString("%1 command order mismatch\nActual: %2\nExpected: %3")
                            .arg(objectName,
                                 actual.join(", "),
                                 expectedCommands.join(", "))));
}

void MainWindowCommandTests::compareToolbar(
    const char *objectName, std::initializer_list<const char *> expected) const
{
    QToolBar *toolbar = _window->findChild<QToolBar *>(objectName);
    QVERIFY2(toolbar, objectName);
    const QStringList actual = commandIds(toolbar->actions());
    const QStringList expectedCommands = toStringList(expected);
    QVERIFY2(actual == expectedCommands,
             qPrintable(QString("%1 command order mismatch\nActual: %2\nExpected: %3")
                            .arg(objectName,
                                 actual.join(", "),
                                 expectedCommands.join(", "))));
}

QModelIndex MainWindowCommandTests::findRootItem(const QString &prefix) const
{
    QTreeView *tree = _window->findChild<QTreeView *>("assetTreeView");
    if (!tree || !tree->model()) {
        return {};
    }
    for (int row = 0; row < tree->model()->rowCount(); ++row) {
        const QModelIndex index = tree->model()->index(row, 0);
        if (index.data().toString().startsWith(prefix)) {
            return index;
        }
    }
    return {};
}

void MainWindowCommandTests::designerHierarchyAndTopLevelMenus()
{
    const char *requiredObjects[] = {
        "centralWidget",
        "mainSplitter",
        "assetTreeView",
        "viewport",
        "menuBar",
        "statusBar",
        "permanentStatusPanel",
        "statusPolysLabel",
        "statusParticlesLabel",
        "statusCameraLabel",
        "statusFramesLabel",
        "statusFpsLabel",
        "statusResolutionLabel",
        "MainToolbar",
        "ObjectToolbar",
        "AnimationToolbar",
    };
    for (const char *objectName : requiredObjects) {
        QVERIFY2(_window->findChild<QObject *>(objectName), objectName);
    }

    QCOMPARE(_window->objectName(), QString("W3DViewMainWindow"));
    W3DViewport *viewport = _window->findChild<W3DViewport *>("viewport");
    QVERIFY(viewport);
    QVERIFY2(!viewport->isVisible(), "The native Direct3D viewport unexpectedly became visible");

    QMenuBar *menuBar = _window->findChild<QMenuBar *>("menuBar");
    QVERIFY(menuBar);
    const QStringList actualMenus = commandIds(menuBar->actions());
    const QStringList expectedMenus = toStringList({"menu:fileMenu",
                                                    "menu:settingsMenu",
                                                    "menu:viewMenu",
                                                    "menu:objectMenu",
                                                    "menu:emittersMenu",
                                                    "menu:primitivesMenu",
                                                    "menu:soundMenu",
                                                    "menu:lightingMenu",
                                                    "menu:cameraMenu",
                                                    "menu:backgroundMenu",
                                                    "menu:movieMenu",
                                                    "menu:helpMenu"});
    QVERIFY(actualMenus == expectedMenus);

    QStringList titles;
    for (QAction *menuAction : menuBar->actions()) {
        QVERIFY(menuAction->menu());
        titles.append(menuAction->menu()->title());
    }
    QVERIFY(titles == toStringList({"&File",
                                    "&Settings",
                                    "&View",
                                    "&Object",
                                    "&Emitters",
                                    "&Primitives",
                                    "&Sound",
                                    "Ligh&ting",
                                    "&Camera",
                                    "&Background",
                                    "&Movie",
                                    "&Help"}));
}

void MainWindowCommandTests::staticMenuCommandOrder()
{
    compareMenu("fileMenu",
                {"actionNew",
                 "actionOpen",
                 "actionMungeSortOnLoad",
                 "actionEnableGammaCorrection",
                 "<separator>",
                 "actionSaveSettings",
                 "actionLoadSettings",
                 "<separator>",
                 "actionImportFacialAnims",
                 "menu:exportMenu",
                 "<separator>",
                 "actionFileTexturePath",
                 "actionAnimatedSoundOptions",
                 "<separator>",
                 "actionRecentFilesPlaceholder",
                 "<separator>",
                 "actionExit"});
    compareMenu("exportMenu",
                {"actionExportAggregate",
                 "actionExportEmitter",
                 "actionExportLod",
                 "actionExportPrimitive",
                 "actionExportSoundObject"});
    compareMenu("settingsMenu", {"actionTexturePaths", "actionAutoExpandAssetTree"});
    compareMenu("viewMenu",
                {"menu:toolbarsMenu",
                 "actionStatusBar",
                 "<separator>",
                 "actionSlideshowPrev",
                 "actionSlideshowNext",
                 "<separator>",
                 "actionChangeResolution",
                 "<separator>",
                 "actionWireframe",
                 "actionSorting",
                 "actionInvertBackfaceCulling",
                 "actionGamma",
                 "<separator>",
                 "menu:npatchesMenu",
                 "actionNpatchesGap"});
    compareMenu("toolbarsMenu",
                {"actionToolbarMain", "actionToolbarObject", "actionToolbarAnimation"});
    compareMenu("objectMenu",
                {"actionObjectRotateX",
                 "actionObjectRotateY",
                 "actionObjectRotateZ",
                 "<separator>",
                 "actionObjectProperties",
                 "<separator>",
                 "actionRestrictAnims",
                 "<separator>",
                 "actionObjectReset",
                 "<separator>",
                 "actionObjectAlternateMaterials"});
    compareMenu("emittersMenu",
                {"actionCreateEmitter",
                 "actionScaleEmitter",
                 "<separator>",
                 "actionEditEmitter",
                 "menu:emittersEditMenu"});
    compareMenu("primitivesMenu",
                {"actionCreateSphere",
                 "actionCreateRing",
                 "<separator>",
                 "actionEditPrimitive"});
    compareMenu("soundMenu",
                {"actionCreateSoundObject", "<separator>", "actionEditSoundObject"});
    compareMenu("lightingMenu",
                {"actionLightRotateY",
                 "actionLightRotateZ",
                 "<separator>",
                 "actionAmbientLight",
                 "actionSceneLight",
                 "<separator>",
                 "actionIncreaseAmbientLight",
                 "actionDecreaseAmbientLight",
                 "actionIncreaseSceneLight",
                 "actionDecreaseSceneLight",
                 "<separator>",
                 "actionExposePrelit",
                 "actionKillSceneLight",
                 "<separator>",
                 "actionPrelitVertex",
                 "actionPrelitMultipass",
                 "actionPrelitMultitex"});
    compareMenu("cameraMenu",
                {"actionCameraFront",
                 "actionCameraBack",
                 "actionCameraLeft",
                 "actionCameraRight",
                 "actionCameraTop",
                 "actionCameraBottom",
                 "<separator>",
                 "actionCameraRotateX",
                 "actionCameraRotateY",
                 "actionCameraRotateZ",
                 "actionCameraCopyScreen",
                 "<separator>",
                 "actionCameraAnimate",
                 "actionCameraBonePosX",
                 "<separator>",
                 "actionCameraSettings",
                 "actionCameraDistance",
                 "<separator>",
                 "actionCameraResetOnDisplay",
                 "actionCameraReset"});
    compareMenu("backgroundMenu",
                {"actionBackgroundColor",
                 "actionBackgroundBitmap",
                 "actionBackgroundObject",
                 "<separator>",
                 "actionFog"});
    compareMenu("movieMenu", {"actionMakeMovie", "actionCaptureScreenshot"});
    compareMenu("helpMenu", {"actionAbout"});
}

void MainWindowCommandTests::toolbarCommandOrder()
{
    compareToolbar("MainToolbar",
                   {"actionNew",
                    "actionOpen",
                    "<separator>",
                    "actionExportEmitter",
                    "actionExportAggregate",
                    "actionExportLod",
                    "actionExportPrimitive",
                    "actionExportSoundObject",
                    "<separator>",
                    "actionListMissingTextures",
                    "<separator>",
                    "actionCopyAssets",
                    "<separator>",
                    "actionAddToLineup",
                    "<separator>",
                    "actionAbout"});
    compareToolbar("ObjectToolbar",
                   {"actionCameraRotateY",
                    "actionCameraRotateX",
                    "actionCameraRotateZ",
                    "actionObjectRotateZ"});
    compareToolbar("AnimationToolbar",
                   {"actionToolbarAnimationPlay",
                    "actionToolbarAnimationStop",
                    "actionToolbarAnimationPause",
                    "actionToolbarAnimationStepBack",
                    "actionToolbarAnimationStepForward"});
}

void MainWindowCommandTests::dynamicMenusAndActionGroups()
{
    compareMenu("animationMenu",
                {"actionToolbarAnimationPlay",
                 "actionToolbarAnimationPause",
                 "actionToolbarAnimationStop",
                 "<separator>",
                 "actionToolbarAnimationStepBack",
                 "actionToolbarAnimationStepForward",
                 "<separator>",
                 "actionAnimationSettings",
                 "<separator>",
                 "actionAnimationAdvanced"});
    compareMenu("hierarchyMenu",
                {"actionHierarchyGenerateLod", "actionHierarchyMakeAggregate"});
    compareMenu("aggregateMenu",
                {"actionAggregateRename",
                 "<separator>",
                 "actionAggregateBoneManagement",
                 "actionAggregateAutoAssignBones",
                 "<separator>",
                 "actionAggregateBindSubobjectLod",
                 "actionAggregateGenerateLod"});
    compareMenu("lodMenu",
                {"actionLodRecordScreenArea",
                 "actionLodIncludeNull",
                 "<separator>",
                 "actionLodPrevious",
                 "actionLodNext",
                 "actionLodAutoSwitch",
                 "<separator>",
                 "actionLodMakeAggregate"});
    compareMenu("npatchesMenu",
                {"actionNpatchesLevel1",
                 "actionNpatchesLevel2",
                 "actionNpatchesLevel3",
                 "actionNpatchesLevel4",
                 "actionNpatchesLevel5",
                 "actionNpatchesLevel6",
                 "actionNpatchesLevel7",
                 "actionNpatchesLevel8"});

    QActionGroup *npatchesGroup = _window->findChild<QActionGroup *>("npatchesGroup");
    QVERIFY(npatchesGroup);
    QVERIFY(npatchesGroup->isExclusive());
    QCOMPARE(npatchesGroup->actions().size(), 8);
    QCOMPARE(npatchesGroup->checkedAction(), action("actionNpatchesLevel4"));

    QActionGroup *prelitGroup = _window->findChild<QActionGroup *>("prelitGroup");
    QVERIFY(prelitGroup);
    QVERIFY(prelitGroup->isExclusive());
    QCOMPARE(prelitGroup->actions().size(), 3);

    QVERIFY(action("actionAggregateBindSubobjectLod")->isCheckable());
    QVERIFY(action("actionLodIncludeNull")->isCheckable());
    QVERIFY(action("actionLodAutoSwitch")->isCheckable());
}

void MainWindowCommandTests::shortcutParity()
{
    QMap<QString, QStringList> expected;
    auto expect = [&expected](const char *name, std::initializer_list<const char *> shortcuts) {
        expected.insert(QString::fromLatin1(name), toStringList(shortcuts));
    };

    expect("actionNew", {"Ctrl+N"});
    expect("actionOpen", {"Ctrl+O"});
    expect("actionSaveSettings", {"Ctrl+S"});
    expect("actionSlideshowPrev", {"PgUp"});
    expect("actionSlideshowNext", {"PgDown"});
    expect("actionSorting", {"Ctrl+P"});
    expect("actionObjectRotateX", {"Ctrl+X"});
    expect("actionObjectRotateY", {"Up", "Ctrl+Y"});
    expect("actionObjectRotateZ", {"Right", "Ctrl+Z"});
    expect("actionObjectProperties", {"Return"});
    expect("actionLightRotateY", {"Ctrl+Up"});
    expect("actionLightRotateZ", {"Ctrl+Right"});
    expect("actionIncreaseAmbientLight", {"+", "="});
    expect("actionDecreaseAmbientLight", {"-"});
    expect("actionIncreaseSceneLight", {"Ctrl++", "Ctrl+="});
    expect("actionDecreaseSceneLight", {"Ctrl+-"});
    expect("actionKillSceneLight", {"Ctrl+*"});
    expect("actionCameraFront", {"Ctrl+F"});
    expect("actionCameraBack", {"Ctrl+B"});
    expect("actionCameraLeft", {"Ctrl+L"});
    expect("actionCameraRight", {"Ctrl+R"});
    expect("actionCameraTop", {"Ctrl+T"});
    expect("actionCameraBottom", {"Ctrl+M"});
    expect("actionCameraCopyScreen", {"Ctrl+C"});
    expect("actionCameraAnimate", {"F8"});
    expect("actionCameraDistance", {"Ctrl+D"});
    expect("actionFog", {"Ctrl+Alt+F"});
    expect("actionCaptureScreenshot", {"F7"});
    expect("shortcutMakeAggregate", {"Ctrl+A"});
    expect("shortcutAdvancedAnimation", {"Ctrl+V"});
    expect("shortcutLodRecordScreenArea", {"Space"});
    expect("shortcutLodPrevious", {"["});
    expect("shortcutLodNext", {"]"});
    expect("shortcutObjectRotateYBack", {"Down"});
    expect("shortcutObjectRotateZBack", {"Left"});
    expect("shortcutLightRotateYBack", {"Ctrl+Down"});
    expect("shortcutLightRotateZBack", {"Ctrl+Left"});
    for (int slot = 1; slot <= 9; ++slot) {
        expected.insert(QString("shortcutQuickSettings%1").arg(slot),
                        QStringList{QString::number(slot)});
    }
    expect("shortcutNextPane", {"F6"});
    expect("shortcutPreviousPane", {"Shift+F6"});

    QMap<QString, QStringList> actual;
    QMap<QString, QString> shortcutOwners;
    for (QAction *candidate : _window->findChildren<QAction *>()) {
        if (candidate->objectName().isEmpty() || candidate->shortcuts().isEmpty()) {
            continue;
        }
        const QStringList shortcuts = portableShortcuts(*candidate);
        actual.insert(candidate->objectName(), shortcuts);
        QCOMPARE(candidate->shortcutContext(), Qt::WindowShortcut);
        for (const QString &shortcut : shortcuts) {
            QVERIFY2(!shortcutOwners.contains(shortcut),
                     qPrintable(QString("Shortcut %1 is assigned to both %2 and %3")
                                    .arg(shortcut,
                                         shortcutOwners.value(shortcut),
                                         candidate->objectName())));
            shortcutOwners.insert(shortcut, candidate->objectName());
        }
    }

    QVERIFY(actual.keys() == expected.keys());
    for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
        QVERIFY2(actual.value(it.key()) == it.value(), qPrintable(it.key()));
    }
}

void MainWindowCommandTests::emptyStartupCommandState()
{
    const char *disabledActions[] = {
        "actionImportFacialAnims",
        "actionExportAggregate",
        "actionExportEmitter",
        "actionExportLod",
        "actionExportPrimitive",
        "actionExportSoundObject",
        "actionObjectProperties",
        "actionScaleEmitter",
        "actionEditEmitter",
        "actionEditPrimitive",
        "actionEditSoundObject",
        "actionMakeMovie",
        "actionCopyAssets",
        "actionAddToLineup",
        "actionToolbarAnimationPlay",
        "actionToolbarAnimationStop",
        "actionToolbarAnimationPause",
        "actionToolbarAnimationStepBack",
        "actionToolbarAnimationStepForward",
    };
    for (const char *objectName : disabledActions) {
        QAction *candidate = action(objectName);
        QVERIFY(candidate);
        QVERIFY2(!candidate->isEnabled(), objectName);
    }

    const char *checkedActions[] = {
        "actionToolbarMain",
        "actionToolbarObject",
        "actionStatusBar",
        "actionAutoExpandAssetTree",
        "actionSorting",
        "actionRestrictAnims",
        "actionCameraResetOnDisplay",
        "actionPrelitMultipass",
        "actionNpatchesLevel4",
    };
    for (const char *objectName : checkedActions) {
        QAction *candidate = action(objectName);
        QVERIFY(candidate);
        QVERIFY2(candidate->isChecked(), objectName);
    }

    const char *uncheckedActions[] = {
        "actionToolbarAnimation",
        "actionWireframe",
        "actionInvertBackfaceCulling",
        "actionNpatchesGap",
        "actionObjectRotateX",
        "actionObjectRotateY",
        "actionObjectRotateZ",
        "actionLightRotateY",
        "actionLightRotateZ",
        "actionExposePrelit",
        "actionPrelitVertex",
        "actionPrelitMultitex",
        "actionCameraRotateX",
        "actionCameraRotateY",
        "actionCameraRotateZ",
        "actionCameraAnimate",
        "actionCameraBonePosX",
        "actionFog",
    };
    for (const char *objectName : uncheckedActions) {
        QAction *candidate = action(objectName);
        QVERIFY(candidate);
        QVERIFY2(!candidate->isChecked(), objectName);
    }

    QToolBar *mainToolbar = _window->findChild<QToolBar *>("MainToolbar");
    QToolBar *objectToolbar = _window->findChild<QToolBar *>("ObjectToolbar");
    QToolBar *animationToolbar = _window->findChild<QToolBar *>("AnimationToolbar");
    QStatusBar *statusBar = _window->findChild<QStatusBar *>("statusBar");
    QVERIFY(mainToolbar);
    QVERIFY(objectToolbar);
    QVERIFY(animationToolbar);
    QVERIFY(statusBar);
    QVERIFY(!mainToolbar->isHidden());
    QVERIFY(!objectToolbar->isHidden());
    QVERIFY(animationToolbar->isHidden());
    QVERIFY(!statusBar->isHidden());
}

void MainWindowCommandTests::selectionSpecificMenusAndEmptyPlaceholders()
{
    QMenuBar *menuBar = _window->findChild<QMenuBar *>("menuBar");
    QTreeView *tree = _window->findChild<QTreeView *>("assetTreeView");
    QVERIFY(menuBar);
    QVERIFY(tree);
    QVERIFY(tree->selectionModel());

    const QStringList baseMenus = toStringList({"menu:fileMenu",
                                                "menu:settingsMenu",
                                                "menu:viewMenu",
                                                "menu:objectMenu",
                                                "menu:emittersMenu",
                                                "menu:primitivesMenu",
                                                "menu:soundMenu",
                                                "menu:lightingMenu",
                                                "menu:cameraMenu",
                                                "menu:backgroundMenu",
                                                "menu:movieMenu",
                                                "menu:helpMenu"});
    QVERIFY(commandIds(menuBar->actions()) == baseMenus);

    const struct {
        const char *treePrefix;
        const char *menuName;
    } cases[] = {
        {"Hierarchy", "hierarchyMenu"},
        {"H-LOD", "lodMenu"},
        {"Aggregate", "aggregateMenu"},
    };
    for (const auto &testCase : cases) {
        const QModelIndex index = findRootItem(QString::fromLatin1(testCase.treePrefix));
        QVERIFY2(index.isValid(), testCase.treePrefix);
        tree->setCurrentIndex(index);

        QStringList expected = baseMenus;
        const int lightingIndex = expected.indexOf("menu:lightingMenu");
        QVERIFY(lightingIndex >= 0);
        expected.insert(lightingIndex, "menu:" + QString::fromLatin1(testCase.menuName));
        QVERIFY(commandIds(menuBar->actions()) == expected);
    }

    const QModelIndex materials = findRootItem("Materials");
    QVERIFY(materials.isValid());
    tree->setCurrentIndex(materials);
    QVERIFY(commandIds(menuBar->actions()) == baseMenus);

    QVERIFY(!_window->findChild<QMenu *>("recentFilesMenu"));
    QAction *recentFilesPlaceholder = action("actionRecentFilesPlaceholder");
    QCOMPARE(recentFilesPlaceholder->text(), QString("Recent File"));
    QVERIFY(recentFilesPlaceholder->isVisible());
    QVERIFY(!recentFilesPlaceholder->isEnabled());

    QMenu *emittersEditMenu = _window->findChild<QMenu *>("emittersEditMenu");
    QVERIFY(emittersEditMenu);
    QVERIFY(QMetaObject::invokeMethod(emittersEditMenu, "aboutToShow", Qt::DirectConnection));
    QCOMPARE(emittersEditMenu->actions().size(), 1);
    QCOMPARE(emittersEditMenu->actions().first()->text(), QString("(No Emitters)"));
    QVERIFY(!emittersEditMenu->actions().first()->isEnabled());

    QMenu *lodMenu = _window->findChild<QMenu *>("lodMenu");
    QVERIFY(lodMenu);
    QVERIFY(QMetaObject::invokeMethod(lodMenu, "aboutToShow", Qt::DirectConnection));
    QVERIFY(!action("actionLodPrevious")->isEnabled());
    QVERIFY(!action("actionLodNext")->isEnabled());
}

void MainWindowCommandTests::recentFilesMatchMfcPresentationAndLimit()
{
    QStringList paths;
    for (int index = 1; index <= 11; ++index) {
        paths.append(QDir(_settingsDirectory->path()).filePath(QString("file%1.w3d").arg(index)));
    }

    QSettings settings;
    settings.setValue("recentFiles", paths);
    settings.sync();

    {
        W3DViewMainWindow recentWindow;
        for (QTimer *timer : recentWindow.findChildren<QTimer *>()) {
            timer->stop();
        }

        QVERIFY(!recentWindow.findChild<QMenu *>("recentFilesMenu"));
        QMenu *fileMenu = recentWindow.findChild<QMenu *>("fileMenu");
        QAction *placeholder =
            recentWindow.findChild<QAction *>("actionRecentFilesPlaceholder");
        QVERIFY(fileMenu);
        QVERIFY(placeholder);
        QVERIFY(!placeholder->isVisible());
        QVERIFY(!placeholder->isEnabled());

        const QList<QAction *> actions = fileMenu->actions();
        const int placeholderIndex = actions.indexOf(placeholder);
        QVERIFY(placeholderIndex >= 9);
        QAction *firstRecentAction = nullptr;
        for (int index = 1; index <= 9; ++index) {
            QAction *recentAction = actions.at(placeholderIndex - 9 + index - 1);
            if (index == 1) {
                firstRecentAction = recentAction;
            }
            QCOMPARE(recentAction->objectName(), QString("recentFileAction%1").arg(index));
            QCOMPARE(recentAction->text(), QString("&%1 file%1.w3d").arg(index));
            QCOMPARE(recentAction->data().toString(), paths.at(index - 1));
            QVERIFY(recentAction->isVisible());
        }

        settings.sync();
        QCOMPARE(settings.value("recentFiles").toStringList().size(), 9);
        QVERIFY(firstRecentAction);
        QTimer::singleShot(0, &recentWindow, []() {
            if (QWidget *modal = QApplication::activeModalWidget()) {
                modal->close();
            }
        });
        firstRecentAction->trigger();

        settings.sync();
        const QStringList remaining = settings.value("recentFiles").toStringList();
        QCOMPARE(remaining.size(), 8);
        QVERIFY(!remaining.contains(paths.first()));
        QVERIFY(!recentWindow.findChild<QAction *>("recentFileAction9"));
    }

    settings.remove("recentFiles");
    settings.sync();
}

void MainWindowCommandTests::settingsFilesAreValidatedBeforeApply()
{
    W3DViewport *viewport = _window->findChild<W3DViewport *>("viewport");
    QVERIFY(viewport);
    QAction *fogAction = action("actionFog");
    fogAction->setChecked(false);
    viewport->setFogEnabled(false);

    const QString validPath = QDir(_settingsDirectory->path()).filePath("valid-settings.dat");
    QFile validFile(validPath);
    QVERIFY(validFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(validFile.write("[Settings]\nFogEnabled=true\n"), qint64(27));
    validFile.close();

    QVERIFY(_window->loadSettingsPath(validPath));
    QVERIFY(viewport->isFogEnabled());
    fogAction->setChecked(false);
    viewport->setFogEnabled(false);
    QVERIFY(!viewport->isFogEnabled());

    const QString malformedPath =
        QDir(_settingsDirectory->path()).filePath("malformed-settings.dat");
    QFile malformedFile(malformedPath);
    QVERIFY(malformedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(malformedFile.write("[Settings]\nFogEnabled=true\n[Broken\n") > 0);
    malformedFile.close();

    QVERIFY(!_window->loadSettingsPath(malformedPath));
    QVERIFY(!viewport->isFogEnabled());
}

void MainWindowCommandTests::relativeSettingsPathResolvesBesideExecutable()
{
    SaveSettingsDialog dialog;
    const QString expected =
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("Default.dat"));
    QCOMPARE(QFileInfo(dialog.selectedPath()).absoluteFilePath(),
             QFileInfo(expected).absoluteFilePath());
}

void MainWindowCommandTests::emptyTexturePathsRemainEmpty()
{
    QSettings settings;
    settings.remove("Config/TexturePath1");
    settings.remove("Config/TexturePath2");
    settings.sync();

    QString interactionFailure;
    bool acceptedDialog = false;
    QTimer::singleShot(0, _window.get(), [&interactionFailure, &acceptedDialog]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog || dialog->objectName() != "TexturePathDialog") {
            interactionFailure = "The Texture Path dialog did not become active";
            if (dialog) {
                dialog->reject();
            }
            return;
        }

        QLineEdit *path1 = dialog->findChild<QLineEdit *>("path1LineEdit");
        QLineEdit *path2 = dialog->findChild<QLineEdit *>("path2LineEdit");
        if (!path1 || !path2) {
            interactionFailure = "The Texture Path line edits were not found";
            dialog->reject();
            return;
        }

        path1->setText("   ");
        path2->clear();
        acceptedDialog = true;
        dialog->accept();
    });

    action("actionTexturePaths")->trigger();
    QVERIFY2(interactionFailure.isEmpty(), qPrintable(interactionFailure));
    QVERIFY(acceptedDialog);

    settings.sync();
    QVERIFY(!settings.contains("Config/TexturePath1"));
    QVERIFY(!settings.contains("Config/TexturePath2"));
}

void MainWindowCommandTests::startupManualClipDefaultsMatchMfc()
{
    QSettings settings;
    settings.setValue("Config/UseManualClipPlanes", true);
    settings.remove("Config/znear");
    settings.remove("Config/zfar");
    settings.sync();

    {
        W3DViewMainWindow window;
        W3DViewport *viewport = window.findChild<W3DViewport *>("viewport");
        QVERIFY(viewport);
        QVERIFY(viewport->isManualClipPlanesEnabled());

        float nearClip = 0.0f;
        float farClip = 0.0f;
        viewport->cameraClipPlanes(nearClip, farClip);
        QCOMPARE(nearClip, 0.1f);
        QCOMPARE(farClip, 100.0f);
    }

    settings.remove("Config/UseManualClipPlanes");
    settings.sync();
}

void MainWindowCommandTests::safeActionWiring()
{
    W3DViewport *viewport = _window->findChild<W3DViewport *>("viewport");
    QStatusBar *statusBar = _window->findChild<QStatusBar *>("statusBar");
    QToolBar *mainToolbar = _window->findChild<QToolBar *>("MainToolbar");
    QToolBar *animationToolbar = _window->findChild<QToolBar *>("AnimationToolbar");
    QVERIFY(viewport);
    QVERIFY(statusBar);
    QVERIFY(mainToolbar);
    QVERIFY(animationToolbar);
    QVERIFY(!viewport->isVisible());

    action("actionWireframe")->trigger();
    QVERIFY(viewport->isWireframeEnabled());
    action("actionWireframe")->trigger();
    QVERIFY(!viewport->isWireframeEnabled());

    action("actionFog")->trigger();
    QVERIFY(viewport->isFogEnabled());
    action("actionFog")->trigger();
    QVERIFY(!viewport->isFogEnabled());

    action("actionObjectRotateX")->trigger();
    QCOMPARE(viewport->objectRotationFlags(), static_cast<int>(W3DViewport::RotateX));
    action("actionObjectRotateX")->trigger();
    QCOMPARE(viewport->objectRotationFlags(), static_cast<int>(W3DViewport::RotateNone));

    action("shortcutObjectRotateYBack")->trigger();
    QCOMPARE(viewport->objectRotationFlags(), static_cast<int>(W3DViewport::RotateYBack));
    action("shortcutObjectRotateYBack")->trigger();
    QCOMPARE(viewport->objectRotationFlags(), static_cast<int>(W3DViewport::RotateNone));

    action("actionLightRotateY")->trigger();
    QCOMPARE(viewport->lightRotationFlags(), static_cast<int>(W3DViewport::RotateY));
    action("actionLightRotateY")->trigger();
    QCOMPARE(viewport->lightRotationFlags(), static_cast<int>(W3DViewport::RotateNone));

    action("shortcutLightRotateZBack")->trigger();
    QCOMPARE(viewport->lightRotationFlags(), static_cast<int>(W3DViewport::RotateZBack));
    action("shortcutLightRotateZBack")->trigger();
    QCOMPARE(viewport->lightRotationFlags(), static_cast<int>(W3DViewport::RotateNone));

    action("actionCameraRotateX")->trigger();
    QCOMPARE(static_cast<int>(viewport->allowedCameraRotation()),
             static_cast<int>(W3DViewport::CameraRotation::OnlyX));
    action("actionCameraRotateY")->trigger();
    QCOMPARE(static_cast<int>(viewport->allowedCameraRotation()),
             static_cast<int>(W3DViewport::CameraRotation::OnlyY));
    QVERIFY(!action("actionCameraRotateX")->isChecked());
    action("actionCameraRotateY")->trigger();
    QCOMPARE(static_cast<int>(viewport->allowedCameraRotation()),
             static_cast<int>(W3DViewport::CameraRotation::Free));

    action("actionCameraAnimate")->trigger();
    QVERIFY(viewport->isCameraAnimationEnabled());
    action("actionCameraAnimate")->trigger();
    QVERIFY(!viewport->isCameraAnimationEnabled());

    action("actionCameraResetOnDisplay")->trigger();
    QVERIFY(!viewport->isAutoResetEnabled());
    action("actionCameraResetOnDisplay")->trigger();
    QVERIFY(viewport->isAutoResetEnabled());

    action("actionStatusBar")->trigger();
    QVERIFY(statusBar->isHidden());
    action("actionStatusBar")->trigger();
    QVERIFY(!statusBar->isHidden());

    action("actionToolbarMain")->trigger();
    QVERIFY(mainToolbar->isHidden());
    action("actionToolbarMain")->trigger();
    QVERIFY(!mainToolbar->isHidden());

    action("actionToolbarAnimation")->trigger();
    QVERIFY(!animationToolbar->isHidden());
    action("actionToolbarAnimation")->trigger();
    QVERIFY(animationToolbar->isHidden());

    QVERIFY2(!viewport->isVisible(), "A command unexpectedly showed the Direct3D viewport");
}

void MainWindowCommandTests::restoredToolbarStateStaysSynchronized()
{
    QAction *mainAction = action("actionToolbarMain");
    QAction *objectAction = action("actionToolbarObject");
    QToolBar *mainToolbar = _window->findChild<QToolBar *>("MainToolbar");
    QToolBar *objectToolbar = _window->findChild<QToolBar *>("ObjectToolbar");
    QVERIFY(mainAction);
    QVERIFY(objectAction);
    QVERIFY(mainToolbar);
    QVERIFY(objectToolbar);
    QVERIFY(mainAction->isChecked());
    QVERIFY(objectAction->isChecked());

    mainAction->trigger();
    objectAction->trigger();
    QVERIFY(mainToolbar->isHidden());
    QVERIFY(objectToolbar->isHidden());
    const QByteArray hiddenToolbarState = _window->saveState();
    QVERIFY(!hiddenToolbarState.isEmpty());

    mainAction->trigger();
    objectAction->trigger();
    QVERIFY(!mainToolbar->isHidden());
    QVERIFY(!objectToolbar->isHidden());

    QSettings settings;
    settings.setValue("Window/State", hiddenToolbarState);
    settings.sync();
    {
        W3DViewMainWindow restoredWindow;
        for (QTimer *timer : restoredWindow.findChildren<QTimer *>()) {
            timer->stop();
        }

        QToolBar *restoredMainToolbar =
            restoredWindow.findChild<QToolBar *>("MainToolbar");
        QToolBar *restoredObjectToolbar =
            restoredWindow.findChild<QToolBar *>("ObjectToolbar");
        QAction *restoredMainAction =
            restoredWindow.findChild<QAction *>("actionToolbarMain");
        QAction *restoredObjectAction =
            restoredWindow.findChild<QAction *>("actionToolbarObject");
        W3DViewport *restoredViewport =
            restoredWindow.findChild<W3DViewport *>("viewport");
        QVERIFY(restoredMainToolbar);
        QVERIFY(restoredObjectToolbar);
        QVERIFY(restoredMainAction);
        QVERIFY(restoredObjectAction);
        QVERIFY(restoredViewport);
        QVERIFY(restoredMainToolbar->isHidden());
        QVERIFY(restoredObjectToolbar->isHidden());
        QVERIFY(!restoredMainAction->isChecked());
        QVERIFY(!restoredObjectAction->isChecked());
        QVERIFY(!restoredViewport->isVisible());
    }
    settings.remove("Window/State");
    settings.sync();
}

void MainWindowCommandTests::aggregateSubobjectNamesAreBounded()
{
    QTemporaryDir fixtureDirectory;
    QVERIFY2(fixtureDirectory.isValid(), "Could not create the aggregate boundary fixture directory");

    const QString inputPath = QDir(fixtureDirectory.path()).filePath("aggregate-input.w3d");
    const QString outputPath = QDir(fixtureDirectory.path()).filePath("aggregate-output.w3d");

    W3dAggregateInfoStruct inputInfo = {};
    setW3dName(inputInfo.BaseModelName, "BOUNDARY_BASE");
    inputInfo.SubobjectCount = 1;

    W3dAggregateSubobjectStruct inputSubobject;
    std::memset(inputSubobject.SubobjectName, 'S', sizeof(inputSubobject.SubobjectName));
    std::memset(inputSubobject.BoneName, 'B', sizeof(inputSubobject.BoneName));

    {
        const QByteArray nativePath = QDir::toNativeSeparators(inputPath).toLocal8Bit();
        RawFileClass file(nativePath.constData());
        QVERIFY(file.Open(FileClass::WRITE));
        ChunkSaveClass save(&file);
        QVERIFY(save.Begin_Chunk(W3D_CHUNK_AGGREGATE));
        QVERIFY(save.Begin_Chunk(W3D_CHUNK_AGGREGATE_INFO));
        QCOMPARE(save.Write(&inputInfo, sizeof(inputInfo)),
                 static_cast<uint32>(sizeof(inputInfo)));
        QCOMPARE(save.Write(&inputSubobject, sizeof(inputSubobject)),
                 static_cast<uint32>(sizeof(inputSubobject)));
        QVERIFY(save.End_Chunk());
        QVERIFY(save.End_Chunk());
        file.Close();
    }

    AggregateDefClass definition;
    {
        const QByteArray nativePath = QDir::toNativeSeparators(inputPath).toLocal8Bit();
        RawFileClass file(nativePath.constData());
        QVERIFY(file.Open(FileClass::READ));
        ChunkLoadClass load(&file);
        QVERIFY(load.Open_Chunk());
        QCOMPARE(load.Cur_Chunk_ID(), static_cast<uint32>(W3D_CHUNK_AGGREGATE));
        QCOMPARE(definition.Load_W3D(load), WW3D_ERROR_OK);
        QVERIFY(load.Close_Chunk());
        file.Close();
    }

    definition.Set_Name("BOUNDARY_AGGREGATE");
    {
        const QByteArray nativePath = QDir::toNativeSeparators(outputPath).toLocal8Bit();
        RawFileClass file(nativePath.constData());
        QVERIFY(file.Open(FileClass::WRITE));
        ChunkSaveClass save(&file);
        QCOMPARE(definition.Save_W3D(save), WW3D_ERROR_OK);
        file.Close();
    }

    W3dAggregateSubobjectStruct savedSubobject = {};
    bool foundInfo = false;
    {
        const QByteArray nativePath = QDir::toNativeSeparators(outputPath).toLocal8Bit();
        RawFileClass file(nativePath.constData());
        QVERIFY(file.Open(FileClass::READ));
        ChunkLoadClass load(&file);
        QVERIFY(load.Open_Chunk());
        QCOMPARE(load.Cur_Chunk_ID(), static_cast<uint32>(W3D_CHUNK_AGGREGATE));
        while (load.Open_Chunk()) {
            if (load.Cur_Chunk_ID() == W3D_CHUNK_AGGREGATE_INFO) {
                W3dAggregateInfoStruct savedInfo = {};
                QCOMPARE(load.Read(&savedInfo, sizeof(savedInfo)),
                         static_cast<uint32>(sizeof(savedInfo)));
                QCOMPARE(savedInfo.SubobjectCount, static_cast<uint32>(1));
                QCOMPARE(load.Read(&savedSubobject, sizeof(savedSubobject)),
                         static_cast<uint32>(sizeof(savedSubobject)));
                foundInfo = true;
            }
            QVERIFY(load.Close_Chunk());
        }
        QVERIFY(load.Close_Chunk());
        file.Close();
    }

    QVERIFY(foundInfo);
    QCOMPARE(inputSubobject.SubobjectName[sizeof(inputSubobject.SubobjectName) - 1], 'S');
    QCOMPARE(inputSubobject.BoneName[sizeof(inputSubobject.BoneName) - 1], 'B');
    QCOMPARE(savedSubobject.SubobjectName[sizeof(savedSubobject.SubobjectName) - 1], '\0');
    QCOMPARE(savedSubobject.BoneName[sizeof(savedSubobject.BoneName) - 1], '\0');
}

void MainWindowCommandTests::soundPrototypeRegistrationRejectsCollisions()
{
    auto *assetManager = WW3DAssetManager::Get_Instance();
    QVERIFY(assetManager);

    constexpr const char *firstName = "qt_snd_a";
    constexpr const char *secondName = "qt_snd_b";
    struct PrototypeCleanup final
    {
        WW3DAssetManager *manager = nullptr;
        ~PrototypeCleanup()
        {
            if (manager) {
                manager->Remove_Prototype("qt_snd_a");
                manager->Remove_Prototype("qt_snd_b");
            }
        }
    } cleanup{assetManager};

    assetManager->Remove_Prototype(firstName);
    assetManager->Remove_Prototype(secondName);

    SoundRenderObjClass first;
    first.Set_Name(firstName);
    QString errorMessage;
    QVERIFY2(UpdateSoundPrototype(first, QString(), &errorMessage), qPrintable(errorMessage));
    PrototypeClass *firstPrototype = assetManager->Find_Prototype(firstName);
    QVERIFY(firstPrototype);

    SoundRenderObjClass second;
    second.Set_Name(secondName);
    errorMessage.clear();
    QVERIFY2(UpdateSoundPrototype(second, QString(), &errorMessage), qPrintable(errorMessage));
    PrototypeClass *secondPrototype = assetManager->Find_Prototype(secondName);
    QVERIFY(secondPrototype);

    second.Set_Name(firstName);
    errorMessage.clear();
    QVERIFY(!UpdateSoundPrototype(second, secondName, &errorMessage));
    QVERIFY(errorMessage.contains("already exists", Qt::CaseInsensitive));
    QCOMPARE(assetManager->Find_Prototype(firstName), firstPrototype);
    QCOMPARE(assetManager->Find_Prototype(secondName), secondPrototype);

    SoundRenderObjClass duplicate;
    duplicate.Set_Name(firstName);
    errorMessage.clear();
    QVERIFY(!UpdateSoundPrototype(duplicate, QString(), &errorMessage));
    QCOMPARE(assetManager->Find_Prototype(firstName), firstPrototype);

    errorMessage.clear();
    QVERIFY2(UpdateSoundPrototype(first, firstName, &errorMessage), qPrintable(errorMessage));
    QVERIFY(assetManager->Find_Prototype(firstName));
    QVERIFY(assetManager->Find_Prototype(firstName) != firstPrototype);
    QCOMPARE(assetManager->Find_Prototype(secondName), secondPrototype);
}

void MainWindowCommandTests::soundSerializerReportsOneShotWriteFailure()
{
    std::array<char, 4096> storage = {};
    OneShotShortWriteRAMFile file(storage.data(), static_cast<int>(storage.size()));
    QVERIFY(file.Open(FileClass::WRITE));

    ChunkSaveClass save(&file);
    SoundRenderObjDefClass definition;
    definition.Set_Name("WRITE_FAILURE_SOUND");

    QCOMPARE(definition.Save_W3D(save), WW3D_ERROR_SAVE_FAILED);
    QVERIFY(file.failed());
    QVERIFY(save.Has_Write_Error());
    QCOMPARE(save.Cur_Chunk_Depth(), 0);
    file.Close();
}

void MainWindowCommandTests::generatedHierarchyAnimationFixture()
{
    QTemporaryDir fixtureDirectory;
    QVERIFY2(fixtureDirectory.isValid(), "Could not create the generated W3D fixture directory");
    [[maybe_unused]] CurrentDirectoryRestorer restoreCurrentDirectory;

    const QString fixturePath =
        QDir(fixtureDirectory.path()).filePath("generated-animation.w3d");
    QVERIFY2(writeGeneratedAnimationFixture(fixturePath),
             "Could not write the generated hierarchy-animation fixture");
    QVERIFY2(_window->openFilePath(fixturePath),
             "W3DViewQt rejected the generated hierarchy-animation fixture");

    auto *assetManager = WW3DAssetManager::Get_Instance();
    QVERIFY(assetManager);

    const QStringList hierarchyNames =
        collectAssetNames(assetManager->Create_HTree_Iterator());
    QVERIFY(hierarchyNames.contains("TEST_RIG"));
    QVERIFY(hierarchyNames.contains("OTHER_RIG"));

    HTreeClass *testTree = assetManager->Get_HTree("TEST_RIG");
    QVERIFY(testTree);
    QCOMPARE(testTree->Num_Pivots(), 1);

    const QStringList animationNames =
        collectAssetNames(assetManager->Create_HAnim_Iterator());
    QCOMPARE(animationNames.count("TEST_RIG.TEST_MOVE"), 1);

    std::unique_ptr<HAnimClass, ReleaseRef<HAnimClass>> animation(
        assetManager->Get_HAnim("TEST_RIG.TEST_MOVE"));
    QVERIFY(animation);
    QCOMPARE(QString::fromLatin1(animation->Get_HName()), QString("TEST_RIG"));
    QCOMPARE(animation->Get_Num_Frames(), 2);
    QCOMPARE(animation->Get_Frame_Rate(), 30.0f);
    QVERIFY(animation->Has_X_Translation(0));
    Vector3 translation;
    animation->Get_Translation(translation, 0, 1.0f);
    QCOMPARE(translation.X, 1.25f);

    QVERIFY(assetManager->Render_Obj_Exists("TEST_MODEL"));
    QVERIFY(assetManager->Render_Obj_Exists("OTHER_MODEL"));
    std::unique_ptr<RenderObjClass, ReleaseRef<RenderObjClass>> renderObject(
        assetManager->Create_Render_Obj("TEST_MODEL"));
    QVERIFY(renderObject);
    QCOMPARE(renderObject->Class_ID(), static_cast<int>(RenderObjClass::CLASSID_HLOD));
    QVERIFY(renderObject->Get_HTree());
    QCOMPARE(QString::fromLatin1(renderObject->Get_HTree()->Get_Name()), QString("TEST_RIG"));

    QTreeView *treeView = _window->findChild<QTreeView *>("assetTreeView");
    W3DViewport *viewport = _window->findChild<W3DViewport *>("viewport");
    QVERIFY(treeView);
    QVERIFY(viewport);
    auto *model = qobject_cast<QStandardItemModel *>(treeView->model());
    QVERIFY(model);

    const QModelIndex hierarchyGroup = findRootItem("Hierarchy");
    QVERIFY(hierarchyGroup.isValid());
    QCOMPARE(hierarchyGroup.data().toString(), QString("Hierarchy (2)"));

    const QModelIndex testModel = findDirectChild(model, hierarchyGroup, "TEST_MODEL");
    const QModelIndex otherModel = findDirectChild(model, hierarchyGroup, "OTHER_MODEL");
    QVERIFY(testModel.isValid());
    QVERIFY(otherModel.isValid());
    QCOMPARE(model->rowCount(testModel), 1);
    QCOMPARE(model->rowCount(otherModel), 0);

    const QModelIndex testAnimation =
        findDirectChild(model, testModel, "TEST_RIG.TEST_MOVE");
    QVERIFY2(testAnimation.isValid(),
             "The generated animation was not attached beneath its matching hierarchy model");

    treeView->setCurrentIndex(testAnimation);
    QCoreApplication::processEvents();
    QVERIFY(viewport->hasAnimation());
    QCOMPARE(viewport->currentAnimationName(), QString("TEST_RIG.TEST_MOVE"));
    QVERIFY(viewport->currentRenderObject());
    QCOMPARE(QString::fromLatin1(viewport->currentRenderObject()->Get_Name()),
             QString("TEST_MODEL"));
    QCOMPARE(viewport->animationState(), W3DViewport::AnimationState::Playing);
    QVERIFY(action("actionMakeMovie")->isEnabled());

    int currentFrame = -1;
    int totalFrames = 0;
    float framesPerSecond = 0.0f;
    QVERIFY(viewport->animationStatus(currentFrame, totalFrames, framesPerSecond));
    QCOMPARE(currentFrame, 0);
    QCOMPARE(totalFrames, 2);
    QCOMPARE(framesPerSecond, 30.0f);

    AnimationPropertiesDialog properties("TEST_RIG.TEST_MOVE");
    QLabel *frameCountValue = properties.findChild<QLabel *>("frameCountValue");
    QLabel *frameRateValue = properties.findChild<QLabel *>("frameRateValue");
    QLabel *hierarchyNameValue = properties.findChild<QLabel *>("hierarchyNameValue");
    QVERIFY(frameCountValue);
    QVERIFY(frameRateValue);
    QVERIFY(hierarchyNameValue);
    QCOMPARE(frameCountValue->text(), QString("2"));
    QCOMPARE(frameRateValue->text(), QString("30.00 fps"));
    QCOMPARE(hierarchyNameValue->text(), QString("TEST_RIG"));

    AnimationSettingsDialog settings(*viewport);
    QSlider *speedSlider = settings.findChild<QSlider *>("speedSlider");
    QCheckBox *blendCheckBox = settings.findChild<QCheckBox *>("blendCheckBox");
    QVERIFY(speedSlider);
    QVERIFY(blendCheckBox);
    speedSlider->setValue(150);
    blendCheckBox->setChecked(false);
    settings.reject();
    QCOMPARE(viewport->animationSpeed(), 1.5f);
    QVERIFY(!viewport->animationBlend());

    AdvancedAnimationDialog advanced(viewport, "TEST_MODEL");
    QListWidget *mixingList = advanced.findChild<QListWidget *>("mixingListWidget");
    QVERIFY(mixingList);
    QCOMPARE(mixingList->count(), 1);
    QCOMPARE(mixingList->item(0)->text(), QString("TEST_RIG.TEST_MOVE"));
    mixingList->item(0)->setSelected(true);
    QDialogButtonBox *advancedButtons =
        advanced.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(advancedButtons);
    QVERIFY(advancedButtons->button(QDialogButtonBox::Ok));
    advancedButtons->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(advanced.result(), static_cast<int>(QDialog::Accepted));
    QVERIFY(viewport->hasAnimation());
    QCOMPARE(viewport->currentAnimationName(), QString("TEST_RIG.TEST_MOVE"));
    QCOMPARE(QString::fromLatin1(viewport->currentRenderObject()->Get_Name()),
             QString("TEST_MODEL"));

    action("actionToolbarAnimationStop")->trigger();
    QCOMPARE(viewport->animationState(), W3DViewport::AnimationState::Stopped);
    action("actionToolbarAnimationStepForward")->trigger();
    QVERIFY(viewport->animationStatus(currentFrame, totalFrames, framesPerSecond));
    QCOMPARE(currentFrame, 1);
}

void MainWindowCommandTests::externalAnimationAssetBundle()
{
    const QString assetDirectory = qEnvironmentVariable("W3DVIEW_EXTERNAL_ASSET_DIR");
    if (assetDirectory.isEmpty()) {
        QSKIP("Set W3DVIEW_EXTERNAL_ASSET_DIR to run the real-asset animation integration test");
    }
    [[maybe_unused]] CurrentDirectoryRestorer restoreCurrentDirectory;

    const QStringList assetNames = {
        "s_a_human.w3d",
        "s_a_head.w3d",
        "c_nod_ksma_l0.w3d",
        "c_nod_ksma_.w3d",
        "c_nod_ksma_head.w3d",
        "c_ag_nod_ksma.w3d",
        "h_a_cresentkick.w3d",
    };
    for (const QString &name : assetNames) {
        const QString path = QDir(assetDirectory).filePath(name);
        QVERIFY2(QFileInfo::exists(path), qPrintable(QString("Missing integration asset: %1").arg(path)));
        QVERIFY2(_window->openFilePath(path), qPrintable(QString("Failed to load integration asset: %1").arg(path)));
    }

    QTreeView *treeView = _window->findChild<QTreeView *>("assetTreeView");
    W3DViewport *viewport = _window->findChild<W3DViewport *>("viewport");
    QVERIFY(treeView);
    QVERIFY(viewport);

    auto *model = qobject_cast<QStandardItemModel *>(treeView->model());
    QVERIFY(model);
    const QModelIndex hierarchyGroup = findRootItem("Hierarchy");
    QVERIFY(hierarchyGroup.isValid());

    QModelIndex kaneHierarchy;
    for (int row = 0; row < model->rowCount(hierarchyGroup); ++row) {
        const QModelIndex candidate = model->index(row, 0, hierarchyGroup);
        if (candidate.data().toString() == "C_NOD_KSMA_") {
            kaneHierarchy = candidate;
            break;
        }
    }
    QVERIFY(kaneHierarchy.isValid());

    QModelIndex kickAnimation;
    for (int row = 0; row < model->rowCount(kaneHierarchy); ++row) {
        const QModelIndex candidate = model->index(row, 0, kaneHierarchy);
        if (candidate.data().toString() == "S_A_HUMAN.H_A_CRESENTKICK") {
            kickAnimation = candidate;
            break;
        }
    }
    QVERIFY2(kickAnimation.isValid(), "The matching Kane animation was not attached beneath C_NOD_KSMA_");

    treeView->setCurrentIndex(kickAnimation);
    QCoreApplication::processEvents();
    QVERIFY(viewport->hasAnimation());
    QCOMPARE(viewport->currentAnimationName(), QString("S_A_HUMAN.H_A_CRESENTKICK"));
    QCOMPARE(viewport->animationState(), W3DViewport::AnimationState::Playing);

    action("actionToolbarAnimationStop")->trigger();
    QCOMPARE(viewport->animationState(), W3DViewport::AnimationState::Stopped);
    action("actionToolbarAnimationStepForward")->trigger();

    int currentFrame = -1;
    int totalFrames = 0;
    float framesPerSecond = 0.0f;
    QVERIFY(viewport->animationStatus(currentFrame, totalFrames, framesPerSecond));
    QCOMPARE(currentFrame, 1);
}

void MainWindowCommandTests::externalRealAssetBundle()
{
    const QString assetDirectory = qEnvironmentVariable("W3DVIEW_EXTERNAL_ASSET_DIR");
    if (assetDirectory.isEmpty()) {
        QSKIP("Set W3DVIEW_EXTERNAL_ASSET_DIR to run the real aggregate, sound, emitter, "
              "sphere, ring, and HLOD integration test");
    }
    [[maybe_unused]] CurrentDirectoryRestorer restoreCurrentDirectory;

    auto *assetManager = WW3DAssetManager::Get_Instance();
    QVERIFY(assetManager);

    const QStringList assetNames = {
        "s_a_human.w3d",
        "s_a_head.w3d",
        "c_gdi_mgo_l0.w3d",
        "c_gdi_mgo_l1.w3d",
        "c_gdi_mgo_l2.w3d",
        "c_gdi_mgo_l3.w3d",
        "c_gdi_mgo_.w3d",
        "c_gdi_mgo_head.w3d",
        "c_ag_gdi_mgo.w3d",
        "s_b_human.w3d",
        "c_nod_sk_l3.w3d",
        "c_nod_sk_l2.w3d",
        "c_nod_sk_l1.w3d",
        "c_nod_sk_l0.w3d",
        "c_nod_sk_.w3d",
        "e_flare02.w3d",
        "xg_ionc_shock0.w3d",
        "xg_ionc_shock1.w3d",
    };
    for (const QString &name : assetNames) {
        const QString path = QDir(assetDirectory).filePath(name);
        QVERIFY2(QFileInfo::exists(path),
                 qPrintable(QString("Missing integration asset: %1").arg(path)));
        QVERIFY2(_window->openFilePath(path),
                 qPrintable(QString("Failed to load integration asset: %1").arg(path)));
    }

    // Loading through W3DViewMainWindow rebuilds the tree and instantiates
    // each prototype. The native application owns an initialized audio
    // singleton, while this offscreen test intentionally does not initialize
    // audio or Direct3D, so load the sound definition directly.
    const QString soundSource = QDir(assetDirectory).filePath("a10_loop.w3d");
    QVERIFY2(QFileInfo::exists(soundSource),
             qPrintable(QString("Missing integration asset: %1").arg(soundSource)));
    const QByteArray soundSourceNative =
        QDir::toNativeSeparators(soundSource).toLocal8Bit();
    QVERIFY(assetManager->Load_3D_Assets(soundSourceNative.constData()));

    auto *aggregatePrototype = dynamic_cast<AggregatePrototypeClass *>(
        assetManager->Find_Prototype("c_ag_gdi_mgo"));
    QVERIFY(aggregatePrototype);
    AggregateDefClass *aggregateDefinition = aggregatePrototype->Get_Definition();
    QVERIFY(aggregateDefinition);
    QCOMPARE(QString::fromLatin1(aggregateDefinition->Get_Name()), QString("c_ag_gdi_mgo"));
    QCOMPARE(QString::fromLatin1(aggregateDefinition->Get_Base_Model_Name()),
             QString("C_GDI_MGO_"));

    std::unique_ptr<RenderObjClass, ReleaseRef<RenderObjClass>> aggregateObject(
        assetManager->Create_Render_Obj("c_ag_gdi_mgo"));
    QVERIFY(aggregateObject);
    const int headBone = aggregateObject->Get_Bone_Index("C HEAD");
    QVERIFY(headBone >= 0);
    bool foundHead = false;
    for (int index = 0; index < aggregateObject->Get_Num_Sub_Objects_On_Bone(headBone); ++index) {
        std::unique_ptr<RenderObjClass, ReleaseRef<RenderObjClass>> subobject(
            aggregateObject->Get_Sub_Object_On_Bone(index, headBone));
        if (subobject && subobject->Get_Name() &&
            QString::fromLatin1(subobject->Get_Name()).compare(
                "C_GDI_MGO_HEAD", Qt::CaseInsensitive) == 0) {
            foundHead = true;
        }
    }
    QVERIFY2(foundHead, "The real aggregate did not attach C_GDI_MGO_HEAD to C HEAD");

    auto *soundPrototype = dynamic_cast<SoundRenderObjPrototypeClass *>(
        assetManager->Find_Prototype("A10_Loop"));
    QVERIFY(soundPrototype);
    SoundRenderObjDefClass *soundDefinition = soundPrototype->Peek_Definition();
    QVERIFY(soundDefinition);
    QCOMPARE(QString::fromLatin1(soundDefinition->Get_Name()), QString("A10_Loop"));

    auto *emitterPrototype = dynamic_cast<ParticleEmitterPrototypeClass *>(
        assetManager->Find_Prototype("e_flare02"));
    QVERIFY(emitterPrototype);
    ParticleEmitterDefClass *emitterDefinition = emitterPrototype->Get_Definition();
    QVERIFY(emitterDefinition);
    QCOMPARE(QString::fromLatin1(emitterDefinition->Get_Name()), QString("e_flare02"));

    auto *spherePrototype = dynamic_cast<SpherePrototypeClass *>(
        assetManager->Find_Prototype("XG_IonC_Shock0"));
    QVERIFY(spherePrototype);
    QCOMPARE(QString::fromLatin1(spherePrototype->Get_Name()), QString("XG_IonC_Shock0"));

    auto *ringPrototype = dynamic_cast<RingPrototypeClass *>(
        assetManager->Find_Prototype("XG_IonC_Shock1"));
    QVERIFY(ringPrototype);
    QCOMPARE(QString::fromLatin1(ringPrototype->Get_Name()), QString("XG_IonC_Shock1"));

    auto *lodPrototype = dynamic_cast<HLodPrototypeClass *>(
        assetManager->Find_Prototype("C_NOD_SK_"));
    QVERIFY(lodPrototype);
    HLodDefClass *lodDefinition = lodPrototype->Get_Definition();
    QVERIFY(lodDefinition);
    QCOMPARE(QString::fromLatin1(lodDefinition->Get_Name()), QString("C_NOD_SK_"));

    QTemporaryDir outputDirectory;
    QVERIFY2(outputDirectory.isValid(), "Could not create the real-asset round-trip directory");
    QString exportError;
    const auto saveAtomically = [&exportError](
                                    const QString &path,
                                    std::uint32_t expectedChunk,
                                    const W3DExportUtils::ChunkWriter &writer) {
        exportError.clear();
        return W3DExportUtils::SaveChunkFileAtomically(
            path, expectedChunk, writer, &exportError);
    };
    const auto loadSingleDefinition = [](
                                          const QString &path,
                                          std::uint32_t expectedChunk,
                                          const std::function<bool(ChunkLoadClass &)> &load) {
        const QByteArray nativePath = QFile::encodeName(QDir::toNativeSeparators(path));
        RawFileClass file(nativePath.constData());
        if (!file.Open(FileClass::READ)) {
            return false;
        }

        const int fileSize = file.Size();
        ChunkLoadClass chunkLoad(&file);
        const bool opened = chunkLoad.Open_Chunk();
        const bool correctChunk = opened && chunkLoad.Cur_Chunk_ID() == expectedChunk;
        const bool loaded = correctChunk && load(chunkLoad);
        const bool closed = opened && chunkLoad.Close_Chunk();
        const bool consumedFile = file.Tell() == fileSize;
        file.Close();
        return opened && correctChunk && loaded && closed && consumedFile;
    };
    const auto fileBytes = [](const QString &path) {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };

    const QString aggregateFirst = QDir(outputDirectory.path()).filePath("aggregate-first.w3d");
    const QString aggregateSecond = QDir(outputDirectory.path()).filePath("aggregate-second.w3d");
    QVERIFY2(saveAtomically(
                 aggregateFirst,
                 W3D_CHUNK_AGGREGATE,
                 [aggregateDefinition](ChunkSaveClass &save) {
                     return aggregateDefinition->Save_W3D(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));

    AggregateDefClass reloadedAggregate;
    QVERIFY(loadSingleDefinition(
        aggregateFirst,
        W3D_CHUNK_AGGREGATE,
        [&reloadedAggregate](ChunkLoadClass &load) {
            return reloadedAggregate.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(reloadedAggregate.Get_Name()), QString("c_ag_gdi_mgo"));
    QCOMPARE(QString::fromLatin1(reloadedAggregate.Get_Base_Model_Name()),
             QString("C_GDI_MGO_"));
    QVERIFY2(saveAtomically(
                 aggregateSecond,
                 W3D_CHUNK_AGGREGATE,
                 [&reloadedAggregate](ChunkSaveClass &save) {
                     return reloadedAggregate.Save_W3D(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));

    const QString soundFirst = QDir(outputDirectory.path()).filePath("sound-first.w3d");
    const QString soundSecond = QDir(outputDirectory.path()).filePath("sound-second.w3d");
    QVERIFY2(saveAtomically(
                 soundFirst,
                 W3D_CHUNK_SOUNDROBJ,
                 [soundDefinition](ChunkSaveClass &save) {
                     return soundDefinition->Save_W3D(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));

    SoundRenderObjDefClass reloadedSound;
    QVERIFY(loadSingleDefinition(
        soundFirst,
        W3D_CHUNK_SOUNDROBJ,
        [&reloadedSound](ChunkLoadClass &load) {
            return reloadedSound.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(reloadedSound.Get_Name()), QString("A10_Loop"));
    QVERIFY2(saveAtomically(
                 soundSecond,
                 W3D_CHUNK_SOUNDROBJ,
                 [&reloadedSound](ChunkSaveClass &save) {
                     return reloadedSound.Save_W3D(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));

    const QString emitterFirst = QDir(outputDirectory.path()).filePath("emitter-first.w3d");
    const QString emitterSecond = QDir(outputDirectory.path()).filePath("emitter-second.w3d");
    QVERIFY2(saveAtomically(
                 emitterFirst,
                 W3D_CHUNK_EMITTER,
                 [emitterDefinition](ChunkSaveClass &save) {
                     return emitterDefinition->Save_W3D(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));
    ParticleEmitterDefClass reloadedEmitter;
    QVERIFY(loadSingleDefinition(
        emitterFirst,
        W3D_CHUNK_EMITTER,
        [&reloadedEmitter](ChunkLoadClass &load) {
            return reloadedEmitter.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(reloadedEmitter.Get_Name()), QString("e_flare02"));
    QVERIFY2(saveAtomically(
                 emitterSecond,
                 W3D_CHUNK_EMITTER,
                 [&reloadedEmitter](ChunkSaveClass &save) {
                     return reloadedEmitter.Save_W3D(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));

    const QString sphereFirst = QDir(outputDirectory.path()).filePath("sphere-first.w3d");
    const QString sphereSecond = QDir(outputDirectory.path()).filePath("sphere-second.w3d");
    QVERIFY2(saveAtomically(
                 sphereFirst,
                 W3D_CHUNK_SPHERE,
                 [spherePrototype](ChunkSaveClass &save) {
                     return spherePrototype->Save(save);
                 }),
             qPrintable(exportError));
    SpherePrototypeClass reloadedSphere;
    QVERIFY(loadSingleDefinition(
        sphereFirst,
        W3D_CHUNK_SPHERE,
        [&reloadedSphere](ChunkLoadClass &load) { return reloadedSphere.Load(load); }));
    QCOMPARE(QString::fromLatin1(reloadedSphere.Get_Name()), QString("XG_IonC_Shock0"));
    QVERIFY2(saveAtomically(
                 sphereSecond,
                 W3D_CHUNK_SPHERE,
                 [&reloadedSphere](ChunkSaveClass &save) { return reloadedSphere.Save(save); }),
             qPrintable(exportError));

    const QString ringFirst = QDir(outputDirectory.path()).filePath("ring-first.w3d");
    const QString ringSecond = QDir(outputDirectory.path()).filePath("ring-second.w3d");
    QVERIFY2(saveAtomically(
                 ringFirst,
                 W3D_CHUNK_RING,
                 [ringPrototype](ChunkSaveClass &save) { return ringPrototype->Save(save); }),
             qPrintable(exportError));
    RingPrototypeClass reloadedRing;
    QVERIFY(loadSingleDefinition(
        ringFirst,
        W3D_CHUNK_RING,
        [&reloadedRing](ChunkLoadClass &load) { return reloadedRing.Load(load); }));
    QCOMPARE(QString::fromLatin1(reloadedRing.Get_Name()), QString("XG_IonC_Shock1"));
    QVERIFY2(saveAtomically(
                 ringSecond,
                 W3D_CHUNK_RING,
                 [&reloadedRing](ChunkSaveClass &save) { return reloadedRing.Save(save); }),
             qPrintable(exportError));

    const QString lodFirst = QDir(outputDirectory.path()).filePath("lod-first.w3d");
    const QString lodSecond = QDir(outputDirectory.path()).filePath("lod-second.w3d");
    QVERIFY2(saveAtomically(
                 lodFirst,
                 W3D_CHUNK_HLOD,
                 [lodDefinition](ChunkSaveClass &save) {
                     return lodDefinition->Save(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));
    HLodDefClass reloadedLod;
    QVERIFY(loadSingleDefinition(
        lodFirst,
        W3D_CHUNK_HLOD,
        [&reloadedLod](ChunkLoadClass &load) {
            return reloadedLod.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(reloadedLod.Get_Name()), QString("C_NOD_SK_"));
    QVERIFY2(saveAtomically(
                 lodSecond,
                 W3D_CHUNK_HLOD,
                 [&reloadedLod](ChunkSaveClass &save) {
                     return reloadedLod.Save(save) == WW3D_ERROR_OK;
                 }),
             qPrintable(exportError));

    QCOMPARE(fileBytes(aggregateSecond), fileBytes(aggregateFirst));
    QCOMPARE(fileBytes(soundSecond), fileBytes(soundFirst));

    const struct {
        QString source;
        QString first;
        QString second;
    } byteStableExports[] = {
        {QDir(assetDirectory).filePath("xg_ionc_shock0.w3d"), sphereFirst, sphereSecond},
        {QDir(assetDirectory).filePath("xg_ionc_shock1.w3d"), ringFirst, ringSecond},
        {QDir(assetDirectory).filePath("c_nod_sk_.w3d"), lodFirst, lodSecond},
        {QDir(assetDirectory).filePath("e_flare02.w3d"), emitterFirst, emitterSecond},
    };
    for (const auto &exportPaths : byteStableExports) {
        const QByteArray sourceBytes = fileBytes(exportPaths.source);
        const QByteArray firstBytes = fileBytes(exportPaths.first);
        const QByteArray secondBytes = fileBytes(exportPaths.second);
        QVERIFY2(!sourceBytes.isEmpty(), qPrintable(exportPaths.source));
        qsizetype firstDifference = -1;
        qsizetype differenceCount = 0;
        const qsizetype comparableSize = std::min(firstBytes.size(), sourceBytes.size());
        for (qsizetype index = 0; index < comparableSize; ++index) {
            if (firstBytes.at(index) != sourceBytes.at(index)) {
                if (firstDifference < 0) {
                    firstDifference = index;
                }
                ++differenceCount;
            }
        }
        if (firstDifference < 0 && firstBytes.size() != sourceBytes.size()) {
            firstDifference = comparableSize;
        }
        differenceCount += std::max(firstBytes.size(), sourceBytes.size()) - comparableSize;
        const int sourceByte = firstDifference >= 0 && firstDifference < sourceBytes.size()
                                   ? static_cast<unsigned char>(sourceBytes.at(firstDifference))
                                   : -1;
        const int firstByte = firstDifference >= 0 && firstDifference < firstBytes.size()
                                  ? static_cast<unsigned char>(firstBytes.at(firstDifference))
                                  : -1;
        const QString difference = QStringLiteral(
                                       "%1: source=%2 bytes, first=%3 bytes, differences=%4, "
                                       "first difference=%5 (source=0x%6, first=0x%7)")
                                       .arg(exportPaths.source)
                                       .arg(sourceBytes.size())
                                       .arg(firstBytes.size())
                                       .arg(differenceCount)
                                       .arg(firstDifference)
                                       .arg(sourceByte, 2, 16, QLatin1Char('0'))
                                       .arg(firstByte, 2, 16, QLatin1Char('0'));
        QVERIFY2(secondBytes == firstBytes, qPrintable(exportPaths.source));
        QVERIFY2(firstBytes == sourceBytes, qPrintable(difference));
    }

    std::unique_ptr<WWAudioClass> audio(WWAudioClass::Create_Instance());
    QVERIFY(audio != nullptr);
    audio->Initialize();
    const bool openALBackend =
        QString::fromLatin1(audio->Get_3D_Driver_Name().Peek_Buffer()) ==
        QStringLiteral("OpenAL 3D Audio");
    if (openALBackend) {
        QVERIFY2(audio->Get_2D_Sample_Count() > 0,
                 "OpenAL did not create any 2D sources; ensure a playback device or "
                 "ALSOFT_DRIVERS=null is available");
        const QString streamingPath =
            QDir(assetDirectory).filePath("elie_bounce_1.l.wav");
        QVERIFY2(QFileInfo::exists(streamingPath),
                 "The supplied large WAV needed for OpenAL 3D streaming is missing");
        QVERIFY2(QFileInfo(streamingPath).size() > DEF_MAX_3D_BUFFER_SIZE * 2,
                 "The supplied WAV does not cross OpenAL's 3D streaming threshold");
        std::unique_ptr<Sound3DClass, ReleaseRef<Sound3DClass>> streamingSound(
            audio->Create_3D_Sound(
                QDir::toNativeSeparators(streamingPath).toLocal8Bit().constData(),
                CLASSID_3D));
        QVERIFY2(streamingSound, "OpenAL could not create the supplied 3D streaming sound");
        streamingSound->Cull_Sound(false);
        QVERIFY(streamingSound->Play());
        QVERIFY(streamingSound->Is_Playing());
        QVERIFY(streamingSound->Stop());
        QVERIFY(!streamingSound->Is_Playing());
    }

    // Exercise the actual MainWindow export actions and their fixed-filename
    // Designer dialog. Reloading the sound source through the window refreshes
    // the tree after its definition was loaded directly above.
    QVERIFY2(_window->openFilePath(soundSource),
             "Failed to refresh the tree with the real sound definition");
    QTreeView *treeView = _window->findChild<QTreeView *>("assetTreeView");
    auto *treeModel = treeView
        ? qobject_cast<QStandardItemModel *>(treeView->model())
        : nullptr;
    QVERIFY(treeView);
    QVERIFY(treeModel);

    QString commandExportFailure;
    const auto exportThroughMainWindow = [&](const QString &groupPrefix,
                                              const QString &assetName,
                                              const char *actionName) {
        commandExportFailure.clear();
        const QModelIndex group = findRootItem(groupPrefix);
        const QModelIndex item = group.isValid()
            ? findDirectChild(treeModel, group, assetName)
            : QModelIndex();
        if (!item.isValid()) {
            commandExportFailure = QString("Could not find %1 under %2")
                                       .arg(assetName, groupPrefix);
            return QString();
        }

        treeView->setCurrentIndex(item);
        QCoreApplication::processEvents();
        QAction *exportAction = _window->findChild<QAction *>(actionName);
        if (!exportAction || !exportAction->isEnabled()) {
            commandExportFailure = QString("%1 was not enabled for %2")
                                       .arg(QString::fromLatin1(actionName), assetName);
            return QString();
        }

        const QString exactFilename = assetName + ".w3d";
        const QString expectedPath =
            QDir(outputDirectory.path()).filePath(exactFilename);
        bool dialogDriven = false;
        QTimer dialogDriver;
        dialogDriver.setSingleShot(true);
        QObject::connect(&dialogDriver, &QTimer::timeout, _window.get(), [&]() {
            auto *dialog =
                qobject_cast<ExportDirectoryDialog *>(QApplication::activeModalWidget());
            if (!dialog) {
                commandExportFailure = QString("The fixed export dialog did not open for %1")
                                           .arg(assetName);
                if (QWidget *modal = QApplication::activeModalWidget()) {
                    modal->close();
                }
                return;
            }

            QLineEdit *filenameEdit = dialog->findChild<QLineEdit *>("filenameEdit");
            QLineEdit *directoryEdit = dialog->findChild<QLineEdit *>("directoryEdit");
            if (!filenameEdit || !directoryEdit || !filenameEdit->isReadOnly() ||
                filenameEdit->text() != exactFilename) {
                commandExportFailure = QString("The fixed filename was wrong for %1")
                                           .arg(assetName);
                dialog->reject();
                return;
            }

            directoryEdit->setText(outputDirectory.path());
            if (QDir::cleanPath(dialog->selectedPath()) != QDir::cleanPath(expectedPath)) {
                commandExportFailure = QString("The selected export path was wrong for %1")
                                           .arg(assetName);
                dialog->reject();
                return;
            }

            dialogDriven = true;
            dialog->accept();
        });
        dialogDriver.start(0);
        exportAction->trigger();
        dialogDriver.stop();

        if (!dialogDriven || !commandExportFailure.isEmpty()) {
            return QString();
        }
        if (!QFileInfo::exists(expectedPath)) {
            commandExportFailure = QString("The command did not create %1").arg(expectedPath);
            return QString();
        }
        return expectedPath;
    };

    const QString commandAggregate =
        exportThroughMainWindow("Aggregate", "c_ag_gdi_mgo", "actionExportAggregate");
    QVERIFY2(!commandAggregate.isEmpty(), qPrintable(commandExportFailure));
    AggregateDefClass commandAggregateDefinition;
    QVERIFY(loadSingleDefinition(
        commandAggregate,
        W3D_CHUNK_AGGREGATE,
        [&commandAggregateDefinition](ChunkLoadClass &load) {
            return commandAggregateDefinition.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(commandAggregateDefinition.Get_Name()),
             QString("c_ag_gdi_mgo"));

    const QString commandEmitter =
        exportThroughMainWindow("Emitter", "e_flare02", "actionExportEmitter");
    QVERIFY2(!commandEmitter.isEmpty(), qPrintable(commandExportFailure));
    ParticleEmitterDefClass commandEmitterDefinition;
    QVERIFY(loadSingleDefinition(
        commandEmitter,
        W3D_CHUNK_EMITTER,
        [&commandEmitterDefinition](ChunkLoadClass &load) {
            return commandEmitterDefinition.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(commandEmitterDefinition.Get_Name()), QString("e_flare02"));

    const QString commandLod =
        exportThroughMainWindow("H-LOD", "C_NOD_SK_", "actionExportLod");
    QVERIFY2(!commandLod.isEmpty(), qPrintable(commandExportFailure));
    HLodDefClass commandLodDefinition;
    QVERIFY(loadSingleDefinition(
        commandLod,
        W3D_CHUNK_HLOD,
        [&commandLodDefinition](ChunkLoadClass &load) {
            return commandLodDefinition.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(commandLodDefinition.Get_Name()), QString("C_NOD_SK_"));

    const QString commandSphere = exportThroughMainWindow(
        "Primitives", "XG_IonC_Shock0", "actionExportPrimitive");
    QVERIFY2(!commandSphere.isEmpty(), qPrintable(commandExportFailure));
    SpherePrototypeClass commandSphereDefinition;
    QVERIFY(loadSingleDefinition(
        commandSphere,
        W3D_CHUNK_SPHERE,
        [&commandSphereDefinition](ChunkLoadClass &load) {
            return commandSphereDefinition.Load(load);
        }));
    QCOMPARE(QString::fromLatin1(commandSphereDefinition.Get_Name()),
             QString("XG_IonC_Shock0"));

    const QString commandRing = exportThroughMainWindow(
        "Primitives", "XG_IonC_Shock1", "actionExportPrimitive");
    QVERIFY2(!commandRing.isEmpty(), qPrintable(commandExportFailure));
    RingPrototypeClass commandRingDefinition;
    QVERIFY(loadSingleDefinition(
        commandRing,
        W3D_CHUNK_RING,
        [&commandRingDefinition](ChunkLoadClass &load) {
            return commandRingDefinition.Load(load);
        }));
    QCOMPARE(QString::fromLatin1(commandRingDefinition.Get_Name()),
             QString("XG_IonC_Shock1"));

    const QString commandSound =
        exportThroughMainWindow("Sounds", "A10_Loop", "actionExportSoundObject");
    QVERIFY2(!commandSound.isEmpty(), qPrintable(commandExportFailure));
    SoundRenderObjDefClass commandSoundDefinition;
    QVERIFY(loadSingleDefinition(
        commandSound,
        W3D_CHUNK_SOUNDROBJ,
        [&commandSoundDefinition](ChunkLoadClass &load) {
            return commandSoundDefinition.Load_W3D(load) == WW3D_ERROR_OK;
        }));
    QCOMPARE(QString::fromLatin1(commandSoundDefinition.Get_Name()), QString("A10_Loop"));

    // Repeating an export to an existing exact target must stop at one
    // default-No overwrite prompt and leave the prior file untouched.
    const QByteArray aggregateBeforeDecline = fileBytes(commandAggregate);
    const QModelIndex aggregateGroup = findRootItem("Aggregate");
    const QModelIndex aggregateItem = aggregateGroup.isValid()
        ? findDirectChild(treeModel, aggregateGroup, "c_ag_gdi_mgo")
        : QModelIndex();
    QVERIFY(aggregateItem.isValid());
    treeView->setCurrentIndex(aggregateItem);
    QCoreApplication::processEvents();
    QAction *aggregateExportAction = _window->findChild<QAction *>("actionExportAggregate");
    QVERIFY(aggregateExportAction);
    QVERIFY(aggregateExportAction->isEnabled());

    QString overwriteFailure;
    bool overwriteDialogDriven = false;
    bool overwritePromptDriven = false;
    QTimer overwriteDialogDriver;
    overwriteDialogDriver.setSingleShot(true);
    QObject::connect(&overwriteDialogDriver,
                     &QTimer::timeout,
                     _window.get(),
                     [&]() {
        auto *dialog =
            qobject_cast<ExportDirectoryDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            overwriteFailure = "The repeated export did not open ExportDirectoryDialog";
            if (QWidget *modal = QApplication::activeModalWidget()) {
                modal->close();
            }
            return;
        }
        QLineEdit *directoryEdit = dialog->findChild<QLineEdit *>("directoryEdit");
        if (!directoryEdit) {
            overwriteFailure = "The repeated export dialog had no directory field";
            dialog->reject();
            return;
        }
        directoryEdit->setText(outputDirectory.path());
        overwriteDialogDriven = true;
        QTimer::singleShot(0, _window.get(), [&]() {
            auto *warning = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (!warning) {
                overwriteFailure = "The existing-target overwrite prompt did not open";
                if (QWidget *modal = QApplication::activeModalWidget()) {
                    modal->close();
                }
                return;
            }
            if (warning->windowTitle() != "Export W3D" ||
                !warning->text().contains("already exists") ||
                warning->standardButton(warning->defaultButton()) != QMessageBox::No) {
                overwriteFailure = "The existing-target prompt was not the expected default-No warning";
                warning->reject();
                return;
            }
            overwritePromptDriven = true;
            warning->done(QMessageBox::No);
        });
        dialog->accept();
    });
    overwriteDialogDriver.start(0);
    aggregateExportAction->trigger();
    overwriteDialogDriver.stop();
    QVERIFY2(overwriteFailure.isEmpty(), qPrintable(overwriteFailure));
    QVERIFY(overwriteDialogDriven);
    QVERIFY(overwritePromptDriven);
    QCOMPARE(fileBytes(commandAggregate), aggregateBeforeDecline);

    {
        std::unique_ptr<RenderObjClass, ReleaseRef<RenderObjClass>> soundObject(
            assetManager->Create_Render_Obj("A10_Loop"));
        QVERIFY(soundObject);
        QCOMPARE(soundObject->Class_ID(), static_cast<int>(RenderObjClass::CLASSID_SOUND));

        SoundEditDialog soundDialog(static_cast<SoundRenderObjClass *>(soundObject.get()));
        QLineEdit *nameEdit = soundDialog.findChild<QLineEdit *>("nameEdit");
        QLineEdit *fileEdit = soundDialog.findChild<QLineEdit *>("fileEdit");
        QCheckBox *infiniteLoops = soundDialog.findChild<QCheckBox *>("infiniteLoops");
        QCheckBox *stopWhenHidden = soundDialog.findChild<QCheckBox *>("stopWhenHidden");
        QRadioButton *radio3d = soundDialog.findChild<QRadioButton *>("radio3d");
        QRadioButton *radioEffect = soundDialog.findChild<QRadioButton *>("radioEffect");
        QSlider *volume = soundDialog.findChild<QSlider *>("volumeSlider");
        QSlider *priority = soundDialog.findChild<QSlider *>("prioritySlider");
        QDoubleSpinBox *dropOff = soundDialog.findChild<QDoubleSpinBox *>("dropOffEdit");
        QDoubleSpinBox *maxVolume = soundDialog.findChild<QDoubleSpinBox *>("maxVolEdit");
        QPushButton *playButton = soundDialog.findChild<QPushButton *>("playButton");
        QVERIFY(nameEdit);
        QVERIFY(fileEdit);
        QVERIFY(infiniteLoops);
        QVERIFY(stopWhenHidden);
        QVERIFY(radio3d);
        QVERIFY(radioEffect);
        QVERIFY(volume);
        QVERIFY(priority);
        QVERIFY(dropOff);
        QVERIFY(maxVolume);
        QVERIFY(playButton);
        QCOMPARE(nameEdit->text(), QString("A10_Loop"));
        QCOMPARE(fileEdit->text(), QString("aircraft_jet_a10_loop_1.wav"));
        QVERIFY(infiniteLoops->isChecked());
        QVERIFY(stopWhenHidden->isChecked());
        QVERIFY(radio3d->isChecked());
        QVERIFY(radioEffect->isChecked());
        QCOMPARE(volume->value(), 100);
        QCOMPARE(priority->value(), 100);
        QCOMPARE(dropOff->value(), 200.0);
        QCOMPARE(maxVolume->value(), 20.0);

        const QString soundPreviewPath =
            QDir(assetDirectory).filePath("aircraft_jet_a10_loop_1.wav");
        QVERIFY(QFileInfo::exists(soundPreviewPath));
        fileEdit->setText(QDir::toNativeSeparators(soundPreviewPath));

        QString playbackFailure;
        bool playbackDialogDriven = false;
        bool unavailablePreviewHandled = false;
        QTimer::singleShot(0, &soundDialog,
                          [&playbackFailure,
                           &playbackDialogDriven,
                           &unavailablePreviewHandled]() {
            QWidget *activeModal = QApplication::activeModalWidget();
            if (auto *warning = qobject_cast<QMessageBox *>(activeModal)) {
                if (warning->windowTitle() != "Play Sound") {
                    playbackFailure = "An unexpected warning replaced the Play Sound dialog";
                } else {
                    unavailablePreviewHandled = true;
                }
                warning->accept();
                return;
            }

            auto *playDialog = qobject_cast<QDialog *>(activeModal);
            if (!playDialog || playDialog->objectName() != "PlaySoundDialog") {
                playbackFailure = "The Play Sound dialog did not become active";
                if (playDialog) {
                    playDialog->reject();
                }
                return;
            }

            QPushButton *stop = playDialog->findChild<QPushButton *>("stopButton");
            QPushButton *play = playDialog->findChild<QPushButton *>("playButton");
            if (!stop || !play) {
                playbackFailure = "The Play/Stop controls were not found";
                playDialog->reject();
                return;
            }

            stop->click();
            play->click();
            stop->click();
            playbackDialogDriven = true;
            playDialog->reject();
        });
        playButton->click();
        QVERIFY2(playbackFailure.isEmpty(), qPrintable(playbackFailure));
        if (openALBackend) {
            QVERIFY2(playbackDialogDriven,
                     "The OpenAL build could not preview the supplied real sound asset");
            QVERIFY(!unavailablePreviewHandled);
        } else {
            QVERIFY(playbackDialogDriven || unavailablePreviewHandled);
        }
        QVERIFY(!QApplication::activeModalWidget());
        soundDialog.reject();
    }
    if (W3DViewport *viewport = _window->findChild<W3DViewport *>("viewport")) {
        viewport->setRenderObject(nullptr);
    }
    audio.reset();
    QVERIFY(WWAudioClass::Get_Instance() == nullptr);
}

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }

    QApplication application(argc, argv);
    WWMath::Init();
    int result = 0;
    {
        WW3DAssetManager assetManager;
        assetManager.Set_WW3D_Load_On_Demand(true);
        MainWindowCommandTests tests;
        result = QTest::qExec(&tests, argc, argv);
    }
    WWMath::Shutdown();
    return result;
}

#include "MainWindowCommandTests.moc"
