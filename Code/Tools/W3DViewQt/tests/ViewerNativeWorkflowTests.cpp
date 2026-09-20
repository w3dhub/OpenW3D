#include "MainWindow.h"
#include "ViewerAssetManager.h"
#include "W3DViewport.h"

#include "AudibleSound.h"
#include "Sound3D.h"
#include "WWAudio.h"
#include "soundhandle.h"
#include "soundrobj.h"
#include "agg_def.h"
#include "chunkio.h"
#include "hlod.h"
#include "rawfile.h"
#include "scene.h"
#include "sphereobj.h"
#include "w3d_file.h"
#include "wwmath.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDataStream>
#include <QDoubleSpinBox>
#include <QDir>
#include <QRadioButton>
#include <QSlider>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>
#include <QtTest/QTest>

#include <cstring>
#include <functional>
#include <memory>

namespace {
template<class T> bool dataChunk(ChunkSaveClass &save, uint32 id, const T &value)
{
    return save.Begin_Chunk(id) && save.Write(&value, sizeof(value)) == sizeof(value)
        && save.End_Chunk();
}

bool writeFixture(const QString &path)
{
    const QByteArray native = QFile::encodeName(path);
    RawFileClass file(native.constData());
    if (!file.Open(FileClass::WRITE)) return false;
    ChunkSaveClass save(&file);
    W3dHierarchyStruct rig{};
    rig.Version = W3D_CURRENT_HTREE_VERSION;
    std::strcpy(rig.Name, "NATIVE_RIG");
    rig.NumPivots = 1;
    W3dPivotStruct root{};
    std::strcpy(root.Name, "ROOTTRANSFORM");
    root.ParentIdx = static_cast<uint32>(-1);
    root.Rotation.Q[3] = 1.0f;
    if (!save.Begin_Chunk(W3D_CHUNK_HIERARCHY) ||
        !dataChunk(save, W3D_CHUNK_HIERARCHY_HEADER, rig) ||
        !dataChunk(save, W3D_CHUNK_PIVOTS, root) || !save.End_Chunk()) return false;
    W3dHLodHeaderStruct header{};
    header.Version = W3D_CURRENT_HLOD_VERSION;
    header.LodCount = 3;
    std::strcpy(header.Name, "NATIVE_LOD");
    std::strcpy(header.HierarchyName, rig.Name);
    if (!save.Begin_Chunk(W3D_CHUNK_HLOD) ||
        !dataChunk(save, W3D_CHUNK_HLOD_HEADER, header)) return false;
    for (int level = 0; level < 3; ++level) {
        W3dHLodArrayHeaderStruct array{};
        array.ModelCount = level + 1;
        array.MaxScreenSize = level == 2 ? NO_MAX_SCREEN_SIZE : (level + 1) * 0.01f;
        if (!save.Begin_Chunk(W3D_CHUNK_HLOD_LOD_ARRAY) ||
            !dataChunk(save, W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, array)) return false;
        for (int index = 0; index <= level; ++index) {
            W3dHLodSubObjectStruct object{};
            std::strcpy(object.Name, "NATIVE_PART");
            if (!dataChunk(save, W3D_CHUNK_HLOD_SUB_OBJECT, object)) return false;
        }
        if (!save.End_Chunk()) return false;
    }
    if (!save.End_Chunk()) return false;
    W3dAnimHeaderStruct animation{};
    animation.Version = W3D_CURRENT_HANIM_VERSION;
    std::strcpy(animation.Name, "NATIVE_MOVE");
    std::strcpy(animation.HierarchyName, rig.Name);
    animation.NumFrames = 2;
    animation.FrameRate = 30;
    return save.Begin_Chunk(W3D_CHUNK_ANIMATION) &&
        dataChunk(save, W3D_CHUNK_ANIMATION_HEADER, animation) && save.End_Chunk();
}

QModelIndex findItem(const QAbstractItemModel &model, const QString &name,
                    const QModelIndex &parent = {})
{
    for (int row = 0; row < model.rowCount(parent); ++row) {
        const QModelIndex item = model.index(row, 0, parent);
        if (item.data().toString() == name) return item;
        const QModelIndex child = findItem(model, name, item);
        if (child.isValid()) return child;
    }
    return {};
}

bool selectObject(W3DViewMainWindow &window, const QString &name)
{
    auto *tree = window.findChild<QTreeView *>("assetTreeView");
    if (!tree || !tree->model()) return false;
    const QModelIndex item = findItem(*tree->model(), name);
    if (!item.isValid()) return false;
    tree->setCurrentIndex({});
    tree->setCurrentIndex(item);
    return true;
}

bool runDialog(W3DViewMainWindow &window, const char *actionName,
               const std::function<bool(QDialog &)> &interact)
{
    auto *action = window.findChild<QAction *>(actionName);
    if (!action || !action->isEnabled()) return false;
    bool handled = false;
    bool ok = true;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &window, [&] {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) return;
        dialog->move(-3200, -3200);
        if (handled || qobject_cast<QMessageBox *>(dialog)) {
            ok = false;
            dialog->reject();
            return;
        }
        handled = true;
        ok = interact(*dialog);
        if (!ok) dialog->reject();
    });
    timer.start(10);
    action->trigger();
    return handled && ok;
}

bool acceptDialog(QDialog &dialog)
{
    auto *box = dialog.findChild<QDialogButtonBox *>("buttonBox");
    if (!box || !box->button(QDialogButtonBox::Ok)) return false;
    box->button(QDialogButtonBox::Ok)->click();
    return dialog.result() == QDialog::Accepted;
}

QList<Vector3> lineupPositions(SceneClass &scene)
{
    QList<Vector3> result;
    SceneIterator *iterator = scene.Create_Iterator();
    if (!iterator) return result;
    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        RenderObjClass *object = iterator->Current_Item();
        if (object && object->Class_ID() == RenderObjClass::CLASSID_HLOD)
            result.append(object->Get_Position());
    }
    scene.Destroy_Iterator(iterator);
    return result;
}

QByteArray capturePixels(W3DViewport &viewport, const QString &base)
{
    const int number = viewport.captureScreenshot(base);
    if (number <= 0) return {};
    QFile file(QString("%1%2.tga").arg(base).arg(number, 2, 10, QLatin1Char('0')));
    if (!file.open(QIODevice::ReadOnly)) return {};
    const QByteArray bytes = file.readAll();
    return bytes.size() > 18 ? bytes.mid(18) : QByteArray{};
}

bool closeEnough(float a, float b) { return std::abs(a - b) < 0.0001f; }
bool closeEnough(const Vector3 &a, const Vector3 &b)
{
    return closeEnough(a.X, b.X) && closeEnough(a.Y, b.Y) && closeEnough(a.Z, b.Z);
}

// Access the engine handle through a pointer to its protected base-class member;
// the object remains the actual sound created by the viewer and the OpenAL backend.
class SoundInspection : public AudibleSoundClass
{
public:
    static SoundHandleClass *handle(AudibleSoundClass &sound)
    {
        return sound.*&SoundInspection::m_SoundHandle;
    }
};

QByteArray soundFixture()
{
    QByteArray samples(80000, '\0');
    for (int index = 0; index < samples.size(); ++index)
        samples[index] = static_cast<char>((index / 20) % 2 ? 160 : 96);
    QByteArray wav;
    QDataStream stream(&wav, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32(36 + samples.size());
    stream.writeRawData("WAVEfmt ", 8);
    stream << quint32(16) << quint16(1) << quint16(1) << quint32(8000)
           << quint32(8000) << quint16(1) << quint16(8);
    stream.writeRawData("data", 4);
    stream << quint32(samples.size());
    stream.writeRawData(samples.constData(), samples.size());
    return wav;
}

class DisposableFile final
{
public:
    explicit DisposableFile(QString path) : path(std::move(path)) {}
    ~DisposableFile() { if (created) QFile::remove(path); }
    bool write(const QByteArray &bytes)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) return false;
        created = true;
        return file.write(bytes) == bytes.size();
    }
private:
    QString path;
    bool created = false;
};
}

class ViewerNativeWorkflowTests final : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void aggregateEditingAndLineup();
    void visibleSettingsRoundTrip_data();
    void visibleSettingsRoundTrip();
    void quickSettingsKeys();
    void manualAutomaticAndScreenAreaLod();
    void soundCreationEditingAndAttenuation();
    void animationTriggeredSound();
    void cleanupTestCase();
private:
    const QString originalDirectory = QDir::currentPath();
    QTemporaryDir fixtures;
    std::unique_ptr<W3DViewMainWindow> window;
    W3DViewport *viewport = nullptr;
};

void ViewerNativeWorkflowTests::initTestCase()
{
    QVERIFY(fixtures.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, fixtures.path());
    QCoreApplication::setOrganizationName("OpenW3DTests");
    QCoreApplication::setApplicationName("W3DViewQtNativeWorkflows");
    window = std::make_unique<W3DViewMainWindow>();
    window->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                           Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnBottomHint);
    window->setAttribute(Qt::WA_ShowWithoutActivating);
    window->resize(800, 600);
    window->move(-3200, -3200);
    window->show();
    QCoreApplication::processEvents();
    viewport = window->findChild<W3DViewport *>("viewport");
    QVERIFY(viewport);
    float nearFog, farFog;
    QVERIFY2(viewport->sceneFogRange(nearFog, farFog), "Native Direct3D scene did not initialize");
    auto *part = new SphereRenderObjClass;
    part->Set_Name("NATIVE_PART");
    part->Set_Extent(Vector3(10.0f, 10.0f, 10.0f));
    WW3DAssetManager::Get_Instance()->Add_Prototype(new SpherePrototypeClass(part));
    part->Release_Ref();
    auto *attachment = new SphereRenderObjClass;
    attachment->Set_Name("NATIVE_EXTRA");
    attachment->Set_Extent(Vector3(4.0f, 4.0f, 4.0f));
    WW3DAssetManager::Get_Instance()->Add_Prototype(new SpherePrototypeClass(attachment));
    attachment->Release_Ref();
    const QString path = fixtures.filePath("native.w3d");
    QVERIFY(writeFixture(path));
    QFile wav(fixtures.filePath("NATIVE_MOVE.wav"));
    QVERIFY(wav.open(QIODevice::WriteOnly));
    const QByteArray soundBytes = soundFixture();
    QCOMPARE(wav.write(soundBytes), soundBytes.size());
    wav.close();
    QVERIFY(window->openFilePath(path));
    QVERIFY(selectObject(*window, "NATIVE_LOD"));
    QVERIFY(viewport->currentRenderObject());
}

void ViewerNativeWorkflowTests::init()
{
    viewport->clearAnimation();
    viewport->clearLineup();
    viewport->setBackgroundBitmap({});
    viewport->setFogEnabled(false);
    viewport->setObjectRotationFlags(0);
    viewport->setLightRotationFlags(0);
    viewport->setLodAutoSwitchingEnabled(false);
}

void ViewerNativeWorkflowTests::aggregateEditingAndLineup()
{
    QVERIFY(selectObject(*window, "NATIVE_LOD"));
    QVERIFY(runDialog(*window, "actionLodMakeAggregate", [](QDialog &dialog) {
        auto *name = dialog.findChild<QLineEdit *>("nameLineEdit");
        if (!name) return false;
        name->setText("NATIVE_AGG");
        return acceptDialog(dialog);
    }));
    QVERIFY(selectObject(*window, "NATIVE_AGG"));
    auto *manager = WW3DAssetManager::Get_Instance();
    QVERIFY(dynamic_cast<AggregatePrototypeClass *>(manager->Find_Prototype("NATIVE_AGG")));

    const auto editBone = [&](bool save) {
        return runDialog(*window, "actionAggregateBoneManagement", [&](QDialog &dialog) {
            auto *bones = dialog.findChild<QTreeWidget *>("boneTree");
            auto *objects = dialog.findChild<QComboBox *>("objectCombo");
            auto *attach = dialog.findChild<QPushButton *>("attachButton");
            if (!bones || !objects || !attach || bones->topLevelItemCount() != 1) return false;
            bones->setCurrentItem(bones->topLevelItem(0));
            const int index = objects->findText("NATIVE_EXTRA");
            if (index < 0) return false;
            objects->setCurrentIndex(index);
            attach->click();
            if (save) return acceptDialog(dialog);
            dialog.reject();
            return true;
        });
    };
    const int originalCount = viewport->currentRenderObject()->Get_Num_Sub_Objects();
    QVERIFY(editBone(true));
    QCOMPARE(viewport->currentRenderObject()->Get_Num_Sub_Objects(), originalCount + 1);
    auto *saved = dynamic_cast<AggregatePrototypeClass *>(manager->Find_Prototype("NATIVE_AGG"));
    QVERIFY(saved);
    QCOMPARE(QByteArray(saved->Get_Definition()->Get_Base_Model_Name()), QByteArray("NATIVE_LOD"));
    RenderObjClass *reloaded = manager->Create_Render_Obj("NATIVE_AGG");
    QVERIFY(reloaded);
    QCOMPARE(reloaded->Get_Num_Sub_Objects(), originalCount + 1);
    reloaded->Release_Ref();
    QVERIFY(editBone(false)); // Removing the attachment and cancelling restores the live preview.
    QCOMPARE(viewport->currentRenderObject()->Get_Num_Sub_Objects(), originalCount + 1);
    QVERIFY(runDialog(*window, "actionAggregateRename", [](QDialog &dialog) {
        auto *name = dialog.findChild<QLineEdit *>("nameLineEdit");
        if (!name) return false;
        name->setText("RENAMED_AGG");
        return acceptDialog(dialog);
    }));
    QVERIFY(!manager->Find_Prototype("NATIVE_AGG"));
    QVERIFY(selectObject(*window, "RENAMED_AGG"));
    RenderObjClass *current = viewport->currentRenderObject();
    QVERIFY(current && current->Peek_Scene());
    SceneClass *scene = current->Peek_Scene();
    QCOMPARE(lineupPositions(*scene).size(), 1);
    for (int count = 2; count <= 3; ++count) {
        QVERIFY(runDialog(*window, "actionAddToLineup", [](QDialog &dialog) {
            auto *objects = dialog.findChild<QComboBox *>("objectComboBox");
            if (!objects) return false;
            const int index = objects->findText("NATIVE_LOD");
            if (index < 0) return false;
            objects->setCurrentIndex(index);
            return acceptDialog(dialog);
        }));
        const auto positions = lineupPositions(*scene);
        QCOMPARE(positions.size(), count);
        for (int first = 0; first < positions.size(); ++first)
            for (int second = first + 1; second < positions.size(); ++second)
                QVERIFY(std::abs(positions[first].Y - positions[second].Y) > 20.0f);
    }
    viewport->clearLineup();
    QCOMPARE(lineupPositions(*scene).size(), 1);
    QCOMPARE(viewport->currentRenderObject(), current);
}

void ViewerNativeWorkflowTests::visibleSettingsRoundTrip_data()
{
    QTest::addColumn<bool>("bitmap");
    QTest::newRow("solid-background") << false;
    QTest::newRow("bitmap-background") << true;
}

void ViewerNativeWorkflowTests::visibleSettingsRoundTrip()
{
    QFETCH(bool, bitmap);
    QVERIFY(selectObject(*window, "NATIVE_LOD"));
    viewport->setRenderObject(nullptr);
    const Vector3 ambient(0.12f, 0.23f, 0.34f);
    W3DViewport::SceneLightState light;
    light.diffuse = Vector3(0.45f, 0.56f, 0.67f);
    light.specular = Vector3(0.78f, 0.89f, 0.91f);
    light.orientation = Quaternion(0.1f, 0.2f, 0.3f, 0.9f);
    light.orientation.Normalize();
    light.distance = 50.0f;
    light.intensity = 0.65f;
    light.attenuationStart = 15.0f;
    light.attenuationEnd = 180.0f;
    light.attenuationEnabled = true;
    light.orientationExplicit = light.distanceExplicit = true;
    viewport->setAmbientLight(ambient);
    viewport->setSceneLightState(light);
    viewport->setBackgroundColor(Vector3(0.1f, 0.4f, 0.7f));
    viewport->setFogEnabled(true);
    QString bitmapPath;
    if (bitmap) {
        bitmapPath = fixtures.filePath("background.tga");
        QByteArray tga(18, '\0');
        tga[2] = 2; tga[12] = 2; tga[14] = 2; tga[16] = 24; tga[17] = 0x20;
        tga.append(QByteArray::fromHex("20408040802080204090a0b0"));
        QFile file(bitmapPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(tga), tga.size());
        file.close();
        viewport->setBackgroundBitmap(bitmapPath);
    }
    const QByteArray before = capturePixels(*viewport, fixtures.filePath("before-"));
    QVERIFY(!before.isEmpty());
    const QString settingsPath = fixtures.filePath("visible.dat");
    QVERIFY(runDialog(*window, "actionSaveSettings", [&](QDialog &dialog) {
        auto *path = dialog.findChild<QLineEdit *>("pathLineEdit");
        auto *lighting = dialog.findChild<QCheckBox *>("lightingCheckBox");
        auto *background = dialog.findChild<QCheckBox *>("backgroundCheckBox");
        if (!path || !lighting || !background) return false;
        path->setText(settingsPath);
        lighting->setChecked(true);
        background->setChecked(true);
        return acceptDialog(dialog);
    }));
    viewport->setAmbientLight(Vector3(0.9f, 0.8f, 0.7f));
    viewport->setSceneLightState(W3DViewport::SceneLightState{});
    viewport->setBackgroundBitmap({});
    viewport->setBackgroundColor(Vector3(0.9f, 0.1f, 0.2f));
    viewport->setFogEnabled(false);
    const QByteArray changed = capturePixels(*viewport, fixtures.filePath("changed-"));
    QVERIFY(!changed.isEmpty());
    QVERIFY(changed != before);
    QVERIFY(window->loadSettingsPath(settingsPath));
    QVERIFY(closeEnough(viewport->ambientLight(), ambient));
    const auto restored = viewport->sceneLightState();
    QVERIFY(closeEnough(restored.diffuse, light.diffuse));
    QVERIFY(closeEnough(restored.specular, light.specular));
    QVERIFY(closeEnough(restored.orientation.X, light.orientation.X));
    QVERIFY(closeEnough(restored.orientation.Y, light.orientation.Y));
    QVERIFY(closeEnough(restored.orientation.Z, light.orientation.Z));
    QVERIFY(closeEnough(restored.orientation.W, light.orientation.W));
    QVERIFY(closeEnough(restored.distance, light.distance));
    QVERIFY(closeEnough(restored.intensity, light.intensity));
    QVERIFY(closeEnough(restored.attenuationStart, light.attenuationStart));
    QVERIFY(closeEnough(restored.attenuationEnd, light.attenuationEnd));
    QCOMPARE(restored.attenuationEnabled, light.attenuationEnabled);
    QCOMPARE(viewport->backgroundBitmap(), bitmapPath);
    QVERIFY(closeEnough(viewport->backgroundColor(), Vector3(0.1f, 0.4f, 0.7f)));
    QVERIFY(viewport->isFogEnabled());
    QCOMPARE(capturePixels(*viewport, fixtures.filePath("restored-")), before);
}

void ViewerNativeWorkflowTests::quickSettingsKeys()
{
    QApplication::setActiveWindow(window.get());
    viewport->setFocus();
    for (int slot = 1; slot <= 9; ++slot) {
        const QString path = QCoreApplication::applicationDirPath() +
            QString("/settings%1.dat").arg(slot);
        DisposableFile fixture(path);
        QVERIFY2(fixture.write(QString("[Settings]\nBackgroundR=%1\nBackgroundG=0.2\n"
                                       "BackgroundB=0.3\nAmbientLightR=0.1\nAmbientLightG=%1\n"
                                       "AmbientLightB=0.3\nFogEnabled=true\n").arg(slot / 10.0)
                                   .toUtf8()),
                 "A settings fixture already exists or cannot be created; it was left unchanged");
        viewport->setBackgroundColor(Vector3(0.0f, 0.0f, 0.0f));
        viewport->setAmbientLight(Vector3(0.0f, 0.0f, 0.0f));
        viewport->setFogEnabled(false);
        QTest::keyClick(viewport, static_cast<Qt::Key>(Qt::Key_0 + slot));
        QVERIFY(closeEnough(viewport->backgroundColor(), Vector3(slot / 10.0f, 0.2f, 0.3f)));
        QVERIFY(closeEnough(viewport->ambientLight(), Vector3(0.1f, slot / 10.0f, 0.3f)));
        QVERIFY(viewport->isFogEnabled());
    }
}

void ViewerNativeWorkflowTests::manualAutomaticAndScreenAreaLod()
{
    QVERIFY(selectObject(*window, "NATIVE_LOD"));
    auto *object = dynamic_cast<HLodClass *>(viewport->currentRenderObject());
    QVERIFY(object);
    QCOMPARE(object->Get_LOD_Count(), 3);
    viewport->setLodAutoSwitchingEnabled(false);
    object->Set_LOD_Level(2);
    for (int expected : {1, 0}) {
        auto *previous = window->findChild<QAction *>("actionLodPrevious");
        QVERIFY(previous);
        previous->trigger();
        QCOMPARE(object->Get_LOD_Level(), expected);
    }
    auto *next = window->findChild<QAction *>("actionLodNext");
    QVERIFY(next);
    next->trigger();
    QCOMPARE(object->Get_LOD_Level(), 1);
    viewport->setCameraDistance(120.0f);
    const float area = viewport->currentScreenSize();
    QVERIFY(area > 0.0f);
    auto *record = window->findChild<QAction *>("actionLodRecordScreenArea");
    QVERIFY(record);
    record->trigger();
    QVERIFY(closeEnough(object->Get_Max_Screen_Size(1), area));
    RenderObjClass *reloaded = WW3DAssetManager::Get_Instance()->Create_Render_Obj("NATIVE_LOD");
    QVERIFY(reloaded);
    QVERIFY(closeEnough(static_cast<HLodClass *>(reloaded)->Get_Max_Screen_Size(1), area));
    reloaded->Release_Ref();
    object->Set_LOD_Level(0);
    viewport->setCameraDistance(30.0f);
    QVERIFY(QMetaObject::invokeMethod(viewport, "renderFrame", Qt::DirectConnection));
    QCOMPARE(object->Get_LOD_Level(), 0);
    viewport->setLodAutoSwitchingEnabled(true);
    QVERIFY(QMetaObject::invokeMethod(viewport, "renderFrame", Qt::DirectConnection));
    QCOMPARE(object->Get_LOD_Level(), 2);
    viewport->setCameraDistance(3000.0f);
    QVERIFY(QMetaObject::invokeMethod(viewport, "renderFrame", Qt::DirectConnection));
    QCOMPARE(object->Get_LOD_Level(), 0);
    viewport->setLodAutoSwitchingEnabled(false);
    QVERIFY(viewport->setNullLodIncluded(true));
    QCOMPARE(object->Get_LOD_Count(), 4);
    QVERIFY(viewport->isNullLodIncluded());
    QVERIFY(viewport->setNullLodIncluded(false));
    QCOMPARE(object->Get_LOD_Count(), 3);
}

void ViewerNativeWorkflowTests::soundCreationEditingAndAttenuation()
{
    auto *audio = WWAudioClass::Get_Instance();
    QVERIFY(audio);
    QCOMPARE(QString::fromLatin1(audio->Get_3D_Driver_Name().Peek_Buffer()),
             QString("OpenAL 3D Audio"));
    QVERIFY(audio->Get_3D_Sample_Count() > 0);
    QVERIFY(runDialog(*window, "actionCreateSoundObject", [&](QDialog &dialog) {
        auto *name = dialog.findChild<QLineEdit *>("nameEdit");
        auto *file = dialog.findChild<QLineEdit *>("fileEdit");
        auto *spatial = dialog.findChild<QRadioButton *>("radio3d");
        auto *loops = dialog.findChild<QCheckBox *>("infiniteLoops");
        auto *volume = dialog.findChild<QSlider *>("volumeSlider");
        auto *priority = dialog.findChild<QSlider *>("prioritySlider");
        auto *drop = dialog.findChild<QDoubleSpinBox *>("dropOffEdit");
        auto *maximum = dialog.findChild<QDoubleSpinBox *>("maxVolEdit");
        if (!name || !file || !spatial || !loops || !volume || !priority || !drop || !maximum)
            return false;
        name->setText("NATIVE_SOUND");
        file->setText("NATIVE_MOVE.wav");
        spatial->setChecked(true);
        loops->setChecked(true);
        volume->setValue(75);
        priority->setValue(60);
        drop->setValue(100.0);
        maximum->setValue(10.0);
        return acceptDialog(dialog);
    }));
    auto *object = dynamic_cast<SoundRenderObjClass *>(viewport->currentRenderObject());
    QVERIFY(object);
    auto *sound = object->Peek_Sound();
    QVERIFY(sound && sound->As_Sound3DClass());
    QVERIFY(closeEnough(sound->Get_Volume(), 0.75f));
    QCOMPARE(sound->Get_Loop_Count(), 0);
    QVERIFY(closeEnough(sound->Peek_Priority(), 0.6f));
    QVERIFY(closeEnough(sound->As_Sound3DClass()->Get_DropOff_Radius(), 100.0f));
    QVERIFY(closeEnough(sound->As_Sound3DClass()->Get_Max_Vol_Radius(), 10.0f));
    viewport->setCameraDistance(5.0f);
    QTest::qWait(50);
    QVERIFY(sound->Is_Playing());
    SoundHandleClass *handle = SoundInspection::handle(*sound);
    QVERIFY(handle);
    const float nearVolume = handle->Get_Sample_Volume();
    QVERIFY(nearVolume > 0.0f);
    viewport->setCameraDistance(95.0f);
    QTest::qWait(50);
    handle = SoundInspection::handle(*sound);
    QVERIFY(handle);
    const float edgeVolume = handle->Get_Sample_Volume();
    QVERIFY2(edgeVolume > 0.0f && edgeVolume < nearVolume * 0.5f,
             "Moving the viewport camera to the edge of the sound radius did not attenuate playback");
    viewport->setCameraDistance(150.0f);
    QTest::qWait(50);
    QVERIFY(sound->Is_Sound_Culled());
    viewport->setCameraDistance(5.0f);
    QTest::qWait(50);
    QVERIFY(!sound->Is_Sound_Culled());
    handle = SoundInspection::handle(*sound);
    QVERIFY(handle);
    QVERIFY(closeEnough(handle->Get_Sample_Volume(), nearVolume));
    QVERIFY(runDialog(*window, "actionEditSoundObject", [](QDialog &dialog) {
        auto *name = dialog.findChild<QLineEdit *>("nameEdit");
        auto *flat = dialog.findChild<QRadioButton *>("radio2d");
        auto *volume = dialog.findChild<QSlider *>("volumeSlider");
        auto *trigger = dialog.findChild<QDoubleSpinBox *>("triggerRadiusEdit");
        if (!name || !flat || !volume || !trigger) return false;
        name->setText("EDITED_SOUND");
        flat->setChecked(true);
        volume->setValue(35);
        trigger->setValue(250.0);
        return acceptDialog(dialog);
    }));
    auto *manager = WW3DAssetManager::Get_Instance();
    QVERIFY(!manager->Find_Prototype("NATIVE_SOUND"));
    QVERIFY(manager->Find_Prototype("EDITED_SOUND"));
    object = dynamic_cast<SoundRenderObjClass *>(viewport->currentRenderObject());
    QVERIFY(object);
    sound = object->Peek_Sound();
    QVERIFY(sound && !sound->As_Sound3DClass());
    QVERIFY(closeEnough(sound->Get_Volume(), 0.35f));
    QCOMPARE(sound->Get_Loop_Count(), 0);
    viewport->setCameraDistance(5.0f);
    QTest::qWait(50);
    QVERIFY(sound->Is_Playing());
    // Exercise the disk loader and clone after their source definitions are gone.
    const QByteArray path = QFile::encodeName(fixtures.filePath("sound.w3d"));
    {
        RawFileClass file(path.constData());
        QVERIFY(file.Open(FileClass::WRITE));
        ChunkSaveClass save(&file);
        auto *prototype = dynamic_cast<SoundRenderObjPrototypeClass *>(
            manager->Find_Prototype("EDITED_SOUND"));
        QVERIFY(prototype);
        QCOMPARE(prototype->Peek_Definition()->Save_W3D(save), WW3D_ERROR_OK);
    }
    manager->Remove_Prototype("EDITED_SOUND");
    QVERIFY(manager->Load_3D_Assets(path.constData()));
    auto *reloaded = dynamic_cast<SoundRenderObjClass *>(
        manager->Create_Render_Obj("EDITED_SOUND"));
    QVERIFY(reloaded);
    auto *clone = static_cast<SoundRenderObjClass *>(reloaded->Clone());
    auto *cloneSound = clone->Peek_Sound();
    const bool independentDefinition = cloneSound && cloneSound->Get_Definition()
        && cloneSound->Get_Definition() != reloaded->Peek_Sound()->Get_Definition();
    reloaded->Release_Ref();
    manager->Remove_Prototype("EDITED_SOUND");
    viewport->setRenderObject(clone);
    clone->Release_Ref();
    QVERIFY(independentDefinition);
    QTest::qWait(50);
    QVERIFY(cloneSound->Is_Playing());
    QVERIFY(closeEnough(cloneSound->Get_Volume(), 0.35f));
}

void ViewerNativeWorkflowTests::animationTriggeredSound()
{
    auto *audio = WWAudioClass::Get_Instance();
    QVERIFY(selectObject(*window, "NATIVE_RIG.NATIVE_MOVE"));
    QVERIFY(viewport->hasAnimation());
    const auto playingAnimationSound = [&]() -> AudibleSoundClass * {
        for (int index = 0; index < audio->Get_2D_Sample_Count(); ++index) {
            auto *sound = audio->Peek_2D_Sample(index);
            if (sound && sound->Get_Filename() &&
                QString::fromLatin1(sound->Get_Filename()).endsWith("NATIVE_MOVE.wav",
                                                                   Qt::CaseInsensitive) &&
                sound->Is_Playing())
                return sound;
        }
        return nullptr;
    };
    QVERIFY2(playingAnimationSound(), "Selecting the animation did not play its companion WAV");
    auto *pause = window->findChild<QAction *>("actionToolbarAnimationPause");
    auto *play = window->findChild<QAction *>("actionToolbarAnimationPlay");
    QVERIFY(pause && play);
    pause->trigger();
    QCOMPARE(viewport->animationState(), W3DViewport::AnimationState::Paused);
    playingAnimationSound()->Stop();
    play->trigger();
    QCOMPARE(viewport->animationState(), W3DViewport::AnimationState::Playing);
    QVERIFY2(playingAnimationSound(), "Resuming the animation did not restart its companion WAV");
}

void ViewerNativeWorkflowTests::cleanupTestCase()
{
    window.reset();
    QVERIFY(QDir::setCurrent(originalDirectory));
}

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    WWMath::Init();
    int result;
    std::unique_ptr<WWAudioClass> audio(WWAudioClass::Create_Instance());
    if (audio) audio->Initialize();
    {
        ViewerAssetManager manager;
        manager.Set_WW3D_Load_On_Demand(false);
        ViewerNativeWorkflowTests tests;
        result = QTest::qExec(&tests, argc, argv);
    }
    audio.reset();
    WWMath::Shutdown();
    return result;
}

#include "ViewerNativeWorkflowTests.moc"
