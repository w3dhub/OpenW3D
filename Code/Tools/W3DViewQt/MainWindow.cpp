#include "MainWindow.h"

#include "RenderObjUtils.h"
#include "W3DExportUtils.h"
#include "W3DViewport.h"
#include "ui_MainWindow.h"

#include "agg_def.h"
#include "assetmgr.h"
#include "AudibleSound.h"
#include "bmp2d.h"
#include "chunkio.h"
#include "dx8wrapper.h"
#include "ffactory.h"
#include "hanim.h"
#include "hmorphanim.h"
#include "hlod.h"
#include "htree.h"
#include "matrix3d.h"
#include "part_emt.h"
#include "part_ldr.h"
#include "quat.h"
#include "rawfile.h"
#include "refcount.h"
#include "rendobj.h"
#include "ringobj.h"
#include "shader.h"
#include "soundrobj.h"
#include "textfile.h"
#include "texture.h"
#include "sphereobj.h"
#include "vector3.h"
#include "v3_rnd.h"
#include "ww3d.h"
#include "WWAudio.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QMimeData>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QSet>
#include <QSignalBlocker>
#include <QSettings>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QDropEvent>
#include <QStringList>
#include <QUrl>
#include <QVector>
#include <QVariant>
#include <QClipboard>
#include <algorithm>
#include <limits>
#include <memory>

#include "CameraDistanceDialog.h"
#include "CameraSettingsDialog.h"
#include "AddToLineupDialog.h"
#include "AdvancedAnimationDialog.h"
#include "AggregateNameDialog.h"
#include "AnimationPropertiesDialog.h"
#include "AnimationSettingsDialog.h"
#include "AnimatedSoundOptionsDialog.h"
#include "BackgroundBitmapDialog.h"
#include "BackgroundObjectDialog.h"
#include "BoneManagementDialog.h"
#include "EmitterEditDialog.h"
#include "ExportDirectoryDialog.h"
#include "ColorLightDialog.h"
#include "GammaDialog.h"
#include "HierarchyPropertiesDialog.h"
#include "MeshPropertiesDialog.h"
#include "ResolutionDialog.h"
#include "RingEditDialog.h"
#include "ScaleDialog.h"
#include "SaveSettingsDialog.h"
#include "SceneLightDialog.h"
#include "SphereEditDialog.h"
#include "SoundEditDialog.h"
#include "TexturePathDialog.h"
#include "RecentFiles.h"
#include "ShortcutHelpers.h"

namespace {
constexpr int kRoleType = Qt::UserRole + 1;
constexpr int kRoleName = Qt::UserRole + 2;
constexpr int kRoleHierarchyName = Qt::UserRole + 3;
constexpr int kRolePointer = Qt::UserRole + 4;
constexpr int kRoleClassId = Qt::UserRole + 5;
constexpr double kPi = 3.14159265358979323846;
constexpr double kDegToRad = kPi / 180.0;
constexpr double kRadToDeg = 180.0 / kPi;
constexpr int kMaxRecentFiles = 9;

QString NormalizeOptionalPath(const QString &path)
{
    const QString trimmed = path.trimmed();
    return trimmed.isEmpty() ? QString() : QDir::cleanPath(trimmed);
}

QString SelectExportPath(QWidget *parent,
                         const QString &title,
                         const QString &fixedFilename,
                         const QString &preferredDirectory)
{
    QString initialDirectory = preferredDirectory;
    if (initialDirectory.isEmpty() || !QDir(initialDirectory).exists()) {
        initialDirectory = QDir::currentPath();
    }

    ExportDirectoryDialog dialog(fixedFilename, initialDirectory, parent);
    dialog.setWindowTitle(title);
    if (dialog.exec() != QDialog::Accepted) {
        return {};
    }
    return dialog.selectedPath();
}

enum class AssetNodeType {
    None = 0,
    Group = 1,
    RenderObject = 2,
    Animation = 3,
    Material = 4,
};

struct RenderObjInfo {
    bool isAggregate = false;
    bool isRealLod = false;
    QString hierarchyName;
};

enum class LodNamingType {
    Commando = 0,
    G = 1,
};

RenderObjInfo InspectRenderObj(const char *name)
{
    RenderObjInfo info;
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return info;
    }

    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name);
    if (!render_obj) {
        return info;
    }

    PrototypeClass *prototype = asset_manager->Find_Prototype(name);
    info.isAggregate = dynamic_cast<AggregatePrototypeClass *>(prototype) != nullptr ||
                       render_obj->Get_Base_Model_Name() != nullptr;
    if (render_obj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        auto *hlod = static_cast<HLodClass *>(render_obj);
        info.isRealLod = hlod->Get_LOD_Count() > 1;
    }

    const HTreeClass *tree = render_obj->Get_HTree();
    if (tree && tree->Get_Name()) {
        info.hierarchyName = QString::fromLatin1(tree->Get_Name());
    }

    render_obj->Release_Ref();
    return info;
}

struct RenderObjectReleaser
{
    void operator()(RenderObjClass *render_object) const
    {
        if (render_object) {
            render_object->Release_Ref();
        }
    }
};

bool ConvertDistLodPrototype(WW3DAssetManager *asset_manager, const QString &name)
{
    if (!asset_manager || name.isEmpty()) {
        return false;
    }

    const QByteArray name_bytes = name.toLatin1();
    PrototypeClass *source_prototype = asset_manager->Find_Prototype(name_bytes.constData());
    if (!source_prototype) {
        return false;
    }
    if (source_prototype->Get_Class_ID() == RenderObjClass::CLASSID_HLOD) {
        return true;
    }
    if (source_prototype->Get_Class_ID() != RenderObjClass::CLASSID_DISTLOD) {
        return false;
    }

    std::unique_ptr<RenderObjClass, RenderObjectReleaser> render_object(
        asset_manager->Create_Render_Obj(name_bytes.constData()));
    if (!render_object) {
        return false;
    }

    const int object_class_id = render_object->Class_ID();
    if (object_class_id != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    std::unique_ptr<HLodDefClass> definition(
        new HLodDefClass(*static_cast<HLodClass *>(render_object.get())));

    // The replacement prototype owns only copied definition data. Drop the
    // temporary instance before deleting the prototype that created it.
    render_object.reset();

    std::unique_ptr<HLodPrototypeClass> replacement(
        new HLodPrototypeClass(definition.release()));
    asset_manager->Remove_Prototype(name_bytes.constData());
    asset_manager->Add_Prototype(replacement.release());
    return true;
}

bool IsLodNameValid(const QString &name, LodNamingType &type)
{
    if (name.size() < 2) {
        return false;
    }

    const QChar last = name.at(name.size() - 1);
    const QChar second_last = name.at(name.size() - 2);
    if ((second_last == 'L' || second_last == 'l') && last.isDigit()) {
        type = LodNamingType::Commando;
        return true;
    }

    if (last.isLetter()) {
        type = LodNamingType::G;
        return true;
    }

    return false;
}

bool IsModelPartOfLod(const QString &name, const QString &base, LodNamingType type)
{
    if (!name.startsWith(base)) {
        return false;
    }

    const QString extension = name.mid(base.size());
    if (type == LodNamingType::Commando) {
        return extension.size() == 2 &&
            (extension.at(0) == 'L' || extension.at(0) == 'l') &&
            extension.at(1).isDigit();
    }

    return extension.size() == 1 && extension.at(0).isLetter();
}

HLodPrototypeClass *GenerateLodPrototype(const QString &base_name, LodNamingType type)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return nullptr;
    }

    RenderObjIterator *iterator = asset_manager->Create_Render_Obj_Iterator();
    if (!iterator) {
        return nullptr;
    }

    int lod_count = 0;
    int starting_index = std::numeric_limits<int>::max();
    QChar starting_char = 'Z';

    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const char *item_name = iterator->Current_Item_Name();
        if (!item_name || !item_name[0]) {
            continue;
        }

        if (!asset_manager->Render_Obj_Exists(item_name)) {
            continue;
        }

        if (iterator->Current_Item_Class_ID() != RenderObjClass::CLASSID_HLOD) {
            continue;
        }

        const QString qname = QString::fromLatin1(item_name);
        if (!IsModelPartOfLod(qname, base_name, type)) {
            continue;
        }

        ++lod_count;
        const QChar last = qname.at(qname.size() - 1);
        if (type == LodNamingType::Commando) {
            starting_index = std::min(starting_index, last.digitValue());
        } else {
            const QChar upper = last.toUpper();
            if (upper < starting_char) {
                starting_char = upper;
            }
        }
    }

    asset_manager->Release_Render_Obj_Iterator(iterator);

    if (lod_count <= 0) {
        return nullptr;
    }

    if (type == LodNamingType::Commando && starting_index == std::numeric_limits<int>::max()) {
        return nullptr;
    }

    QVector<RenderObjClass *> lod_array(lod_count, nullptr);
    for (int lod_index = 0; lod_index < lod_count; ++lod_index) {
        QString lod_name;
        if (type == LodNamingType::Commando) {
            lod_name = QString("%1L%2").arg(base_name).arg(starting_index + lod_index);
        } else {
            lod_name = base_name + QChar(starting_char.unicode() + lod_index);
        }

        const QByteArray name_bytes = lod_name.toLatin1();
        RenderObjClass *lod_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
        if (!lod_obj) {
            for (auto *item : lod_array) {
                if (item) {
                    item->Release_Ref();
                }
            }
            return nullptr;
        }

        lod_array[lod_count - (lod_index + 1)] = lod_obj;
    }

    const QByteArray base_bytes = base_name.toLatin1();
    auto *new_lod = new HLodClass(base_bytes.constData(), lod_array.data(), lod_count);
    auto *definition = new HLodDefClass(*new_lod);
    auto *prototype = new HLodPrototypeClass(definition);

    new_lod->Release_Ref();
    for (auto *item : lod_array) {
        if (item) {
            item->Release_Ref();
        }
    }

    return prototype;
}

void CollectHierarchyItems(QStandardItem *parent,
                           const QString &hierarchyName,
                           QVector<QStandardItem *> &items)
{
    if (!parent || hierarchyName.isEmpty()) {
        return;
    }

    const int count = parent->rowCount();
    for (int index = 0; index < count; ++index) {
        auto *child = parent->child(index);
        if (!child) {
            continue;
        }

        const QString itemHierarchy = child->data(kRoleHierarchyName).toString();
        if (itemHierarchy == hierarchyName) {
            items.push_back(child);
        }
    }
}

void CollectAllChildren(QStandardItem *parent, QVector<QStandardItem *> &items)
{
    if (!parent) {
        return;
    }

    const int count = parent->rowCount();
    for (int index = 0; index < count; ++index) {
        auto *child = parent->child(index);
        if (child) {
            items.push_back(child);
        }
    }
}

void AdjustLightIntensity(Vector3 &color, float inc)
{
    color.X = std::clamp(color.X + inc, 0.0f, 1.0f);
    color.Y = std::clamp(color.Y + inc, 0.0f, 1.0f);
    color.Z = std::clamp(color.Z + inc, 0.0f, 1.0f);
}

void SortAnimationChildren(QStandardItem *parent)
{
    if (!parent) {
        return;
    }

    const int count = parent->rowCount();
    for (int index = 0; index < count; ++index) {
        auto *child = parent->child(index);
        if (child) {
            child->sortChildren(0, Qt::AscendingOrder);
        }
    }
}

void SetHighestLod(RenderObjClass *render_obj)
{
    if (!render_obj) {
        return;
    }

    const int count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = render_obj->Get_Sub_Object(index);
        if (sub_obj) {
            SetHighestLod(sub_obj);
            sub_obj->Release_Ref();
        }
    }

    if (render_obj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        auto *hlod = static_cast<HLodClass *>(render_obj);
        const int max_level = hlod->Get_LOD_Count() - 1;
        if (max_level >= 0) {
            hlod->Set_LOD_Level(max_level);
        }
    }
}

bool GetSelectedRenderObject(QTreeView *tree, QString &name, int &class_id)
{
    if (!tree) {
        return false;
    }

    const QModelIndex current = tree->currentIndex();
    if (!current.isValid()) {
        return false;
    }

    if (current.data(kRoleType).toInt() != static_cast<int>(AssetNodeType::RenderObject)) {
        return false;
    }

    name = current.data(kRoleName).toString();
    class_id = current.data(kRoleClassId).toInt();
    return !name.isEmpty();
}

bool GetSelectedRenderObjectName(QTreeView *tree, QString &name)
{
    if (!tree) {
        return false;
    }

    QModelIndex current = tree->currentIndex();
    if (!current.isValid()) {
        return false;
    }

    const int type_value = current.data(kRoleType).toInt();
    if (type_value == static_cast<int>(AssetNodeType::RenderObject)) {
        name = current.data(kRoleName).toString();
        return !name.isEmpty();
    }

    if (type_value == static_cast<int>(AssetNodeType::Animation)) {
        QModelIndex render_index = current.parent();
        while (render_index.isValid() &&
               render_index.data(kRoleType).toInt() != static_cast<int>(AssetNodeType::RenderObject)) {
            render_index = render_index.parent();
        }
        if (!render_index.isValid()) {
            return false;
        }
        name = render_index.data(kRoleName).toString();
        return !name.isEmpty();
    }

    return false;
}

QString GetSelectedHierarchyName(QTreeView *tree)
{
    if (!tree) {
        return QString();
    }

    QModelIndex current = tree->currentIndex();
    while (current.isValid()) {
        const QString hierarchy = current.data(kRoleHierarchyName).toString();
        if (!hierarchy.isEmpty()) {
            return hierarchy;
        }
        current = current.parent();
    }

    return QString();
}

QModelIndex FindRenderObjectIndex(QStandardItemModel *model, const QString &name, int class_id)
{
    if (!model || name.isEmpty() || model->rowCount() <= 0) {
        return QModelIndex();
    }

    const QModelIndex start = model->index(0, 0);
    const QModelIndexList matches = model->match(start,
                                                 kRoleName,
                                                 name,
                                                 -1,
                                                 Qt::MatchExactly | Qt::MatchRecursive);
    for (const QModelIndex &match : matches) {
        if (!match.isValid()) {
            continue;
        }
        if (match.data(kRoleType).toInt() != static_cast<int>(AssetNodeType::RenderObject)) {
            continue;
        }
        if (class_id >= 0 && match.data(kRoleClassId).toInt() != class_id) {
            continue;
        }
        return match;
    }

    return QModelIndex();
}

void ExpandParentChain(QTreeView *tree, QModelIndex index)
{
    if (!tree) {
        return;
    }

    while (index.isValid()) {
        tree->expand(index);
        index = index.parent();
    }
}

QString ResolveGroupLabel(QStandardItemModel *model, const QModelIndex &index, bool is_group)
{
    if (!model || !index.isValid()) {
        return QString();
    }

    auto *item = model->itemFromIndex(index);
    if (!item) {
        return QString();
    }

    if (is_group) {
        return item->text();
    }

    if (auto *parent = item->parent()) {
        return parent->text();
    }

    return QString();
}

QString FindHierarchyAssetPath(const QString &directory, const QString &hierarchy)
{
    if (directory.isEmpty() || hierarchy.isEmpty()) {
        return QString();
    }

    QDir dir(directory);
    QString base = hierarchy;
    if (base.endsWith(".w3d", Qt::CaseInsensitive)) {
        base.chop(4);
    }

    const QString direct_path = dir.filePath(base + ".w3d");
    if (QFileInfo::exists(direct_path)) {
        return direct_path;
    }

    const auto entries = dir.entryInfoList(QStringList() << "*.w3d" << "*.W3D", QDir::Files);
    for (const auto &info : entries) {
        if (info.completeBaseName().compare(base, Qt::CaseInsensitive) == 0) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

void LoadMissingHierarchyAssets(WW3DAssetManager *asset_manager, const QString &directory)
{
    if (!asset_manager || directory.isEmpty()) {
        return;
    }

    QSet<QString> loaded_hierarchies;
    RenderObjIterator *render_iter = asset_manager->Create_Render_Obj_Iterator();
    if (render_iter) {
        for (render_iter->First(); !render_iter->Is_Done(); render_iter->Next()) {
            const char *name = render_iter->Current_Item_Name();
            if (!name || !name[0]) {
                continue;
            }

            const RenderObjInfo info = InspectRenderObj(name);
            if (!info.hierarchyName.isEmpty()) {
                loaded_hierarchies.insert(info.hierarchyName.toUpper());
            }
        }
        asset_manager->Release_Render_Obj_Iterator(render_iter);
    }

    QSet<QString> anim_hierarchies;
    AssetIterator *anim_iter = asset_manager->Create_HAnim_Iterator();
    if (anim_iter) {
        for (anim_iter->First(); !anim_iter->Is_Done(); anim_iter->Next()) {
            const char *anim_name = anim_iter->Current_Item_Name();
            if (!anim_name || !anim_name[0]) {
                continue;
            }

            HAnimClass *anim = asset_manager->Get_HAnim(anim_name);
            if (!anim) {
                continue;
            }

            const char *hier_name = anim->Get_HName();
            if (hier_name && hier_name[0]) {
                anim_hierarchies.insert(QString::fromLatin1(hier_name).toUpper());
            }
            anim->Release_Ref();
        }
        delete anim_iter;
    }

    for (const auto &hierarchy : anim_hierarchies) {
        if (loaded_hierarchies.contains(hierarchy)) {
            continue;
        }

        const QString path = FindHierarchyAssetPath(directory, hierarchy);
        if (path.isEmpty()) {
            continue;
        }

        const QByteArray path_bytes = QDir::toNativeSeparators(path).toLocal8Bit();
        if (asset_manager->Load_3D_Assets(path_bytes.constData())) {
            loaded_hierarchies.insert(hierarchy);
        }
    }
}

bool ImportFacialAnimation(const QString &hierarchy, const QString &path)
{
    if (hierarchy.isEmpty() || path.isEmpty()) {
        return false;
    }

    const QByteArray file_native = QDir::toNativeSeparators(path).toLocal8Bit();
    TextFileClass anim_desc_file(file_native.constData());
    if (!anim_desc_file.Open()) {
        return false;
    }

    HMorphAnimClass *new_anim = new HMorphAnimClass;
    const QByteArray hierarchy_bytes = hierarchy.toLatin1();
    if (!new_anim->Import(hierarchy_bytes.constData(), anim_desc_file)) {
        anim_desc_file.Close();
        new_anim->Release_Ref();
        return false;
    }

    const QString anim_name = QFileInfo(path).completeBaseName().toUpper();
    const QString new_name = QString("%1.%2").arg(hierarchy, anim_name);
    const QByteArray new_name_bytes = new_name.toLatin1();
    new_anim->Set_Name(new_name_bytes.constData());

    if (auto *asset_manager = WW3DAssetManager::Get_Instance()) {
        asset_manager->Add_Anim(new_anim);
    }

    const QString output_path = QDir(QFileInfo(path).absolutePath()).filePath(anim_name + ".w3d");
    const QByteArray output_native = QDir::toNativeSeparators(output_path).toLocal8Bit();
    RawFileClass animation_file(output_native.constData());
    if (animation_file.Create() == (int)true &&
        animation_file.Open(FileClass::WRITE) == (int)true) {
        ChunkSaveClass csave(&animation_file);
        new_anim->Save_W3D(csave);
        animation_file.Close();
    }

    anim_desc_file.Close();
    new_anim->Release_Ref();
    return true;
}

ParticleEmitterDefClass CreateDefaultEmitterDefinition()
{
    ParticlePropertyStruct<Vector3> color;
    color.Start = Vector3(1, 1, 1);
    color.Rand.Set(0, 0, 0);
    color.NumKeyFrames = 0;
    color.KeyTimes = nullptr;
    color.Values = nullptr;

    ParticlePropertyStruct<float> opacity;
    opacity.Start = 1.0f;
    opacity.Rand = 0.0f;
    opacity.NumKeyFrames = 0;
    opacity.KeyTimes = nullptr;
    opacity.Values = nullptr;

    ParticlePropertyStruct<float> size;
    size.Start = 0.1f;
    size.Rand = 0.0f;
    size.NumKeyFrames = 0;
    size.KeyTimes = nullptr;
    size.Values = nullptr;

    ParticlePropertyStruct<float> rotation;
    rotation.Start = 0.0f;
    rotation.Rand = 0.0f;
    rotation.NumKeyFrames = 0;
    rotation.KeyTimes = nullptr;
    rotation.Values = nullptr;

    ParticlePropertyStruct<float> frames;
    frames.Start = 0.0f;
    frames.Rand = 0.0f;
    frames.NumKeyFrames = 0;
    frames.KeyTimes = nullptr;
    frames.Values = nullptr;

    ParticlePropertyStruct<float> blur_times;
    blur_times.Start = 0.0f;
    blur_times.Rand = 0.0f;
    blur_times.NumKeyFrames = 0;
    blur_times.KeyTimes = nullptr;
    blur_times.Values = nullptr;

    auto *emitter = new ParticleEmitterClass(
        10.0f,
        1,
        new Vector3SolidBoxRandomizer(Vector3(0.1f, 0.1f, 0.1f)),
        Vector3(0, 0, 1),
        new Vector3SolidBoxRandomizer(Vector3(0, 0, 0.1f)),
        0.0f,
        0.0f,
        color,
        opacity,
        size,
        rotation,
        0.0f,
        frames,
        blur_times,
        Vector3(0, 0, 0),
        1.0f,
        nullptr,
        ShaderClass::_PresetAdditiveSpriteShader,
        0);

    ParticleEmitterDefClass *definition = emitter->Build_Definition();
    ParticleEmitterDefClass copy;
    if (definition) {
        copy = *definition;
        delete definition;
    }
    emitter->Release_Ref();
    return copy;
}

bool UpdateEmitterPrototype(const ParticleEmitterDefClass &definition,
                            const QString &old_name,
                            QString *errorMessage = nullptr)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        if (errorMessage) {
            *errorMessage = "WW3D asset manager is not available.";
        }
        return false;
    }

    const char *definition_name = definition.Get_Name();
    const QString new_name = definition_name ? QString::fromLatin1(definition_name) : QString();
    if (new_name.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Emitter name is required.";
        }
        return false;
    }

    const QByteArray new_bytes = new_name.toLatin1();
    const bool replaces_registered_name = !old_name.isEmpty() &&
        old_name.compare(new_name, Qt::CaseInsensitive) == 0;
    if (asset_manager->Find_Prototype(new_bytes.constData()) && !replaces_registered_name) {
        if (errorMessage) {
            *errorMessage = QString("An asset named '%1' already exists.").arg(new_name);
        }
        return false;
    }

    auto definition_copy = std::make_unique<ParticleEmitterDefClass>(definition);
    auto prototype = std::make_unique<ParticleEmitterPrototypeClass>(definition_copy.release());

    if (!old_name.isEmpty()) {
        const QByteArray old_bytes = old_name.toLatin1();
        asset_manager->Remove_Prototype(old_bytes.constData());
    }

    asset_manager->Add_Prototype(prototype.release());
    return true;
}

} // namespace

W3DViewMainWindow::W3DViewMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::W3DViewMainWindow)
{
    // Asset parsing must not depend on whether the native Direct3D viewport
    // has already initialized. This also keeps startup-file loading reliable
    // when the render device is temporarily unavailable.
    if (auto *asset_manager = WW3DAssetManager::Get_Instance()) {
        asset_manager->Register_Prototype_Loader(&_ParticleEmitterLoader);
        asset_manager->Register_Prototype_Loader(&_RingLoader);
        asset_manager->Register_Prototype_Loader(&_SphereLoader);
        asset_manager->Register_Prototype_Loader(&_SoundRenderObjLoader);
    }

    _ui->setupUi(this);
    setAcceptDrops(true);

    _fileMenu = _ui->fileMenu;
    _emittersEditMenu = _ui->emittersEditMenu;
    _objectMenuAction = _ui->objectMenu->menuAction();
    _mainToolbar = _ui->MainToolbar;
    _objectToolbar = _ui->ObjectToolbar;
    _animationToolbar = _ui->AnimationToolbar;
    _toolbarMainAction = _ui->actionToolbarMain;
    _toolbarObjectAction = _ui->actionToolbarObject;
    _toolbarAnimationAction = _ui->actionToolbarAnimation;
    _newAction = _ui->actionNew;
    _openAction = _ui->actionOpen;
    _recentFilesPlaceholderAction = _ui->actionRecentFilesPlaceholder;
    _texturePathsAction = _ui->actionTexturePaths;
    _autoExpandTreeAction = _ui->actionAutoExpandAssetTree;
    _loadSettingsAction = _ui->actionLoadSettings;
    _saveSettingsAction = _ui->actionSaveSettings;
    _enableGammaAction = _ui->actionEnableGammaCorrection;
    _mungeSortAction = _ui->actionMungeSortOnLoad;
    _exportAggregateAction = _ui->actionExportAggregate;
    _exportEmitterAction = _ui->actionExportEmitter;
    _exportLodAction = _ui->actionExportLod;
    _exportPrimitiveAction = _ui->actionExportPrimitive;
    _exportSoundObjectAction = _ui->actionExportSoundObject;
    _editSoundObjectAction = _ui->actionEditSoundObject;
    _editEmitterAction = _ui->actionEditEmitter;
    _scaleEmitterAction = _ui->actionScaleEmitter;
    _editPrimitiveAction = _ui->actionEditPrimitive;
    _listMissingTexturesAction = _ui->actionListMissingTextures;
    _copyAssetsAction = _ui->actionCopyAssets;
    _addToLineupAction = _ui->actionAddToLineup;
    _aboutAction = _ui->actionAbout;
    _wireframeAction = _ui->actionWireframe;
    _sortingAction = _ui->actionSorting;
    _restrictAnimsAction = _ui->actionRestrictAnims;
    _statusBarAction = _ui->actionStatusBar;
    _fogAction = _ui->actionFog;
    _gammaAction = _ui->actionGamma;
    _invertBackfaceCullingAction = _ui->actionInvertBackfaceCulling;
    _backgroundObjectAction = _ui->actionBackgroundObject;
    _captureScreenshotAction = _ui->actionCaptureScreenshot;
    _makeMovieAction = _ui->actionMakeMovie;
    _slideshowPrevAction = _ui->actionSlideshowPrev;
    _slideshowNextAction = _ui->actionSlideshowNext;
    _objectRotateXAction = _ui->actionObjectRotateX;
    _objectRotateYAction = _ui->actionObjectRotateY;
    _objectRotateZAction = _ui->actionObjectRotateZ;
    _objectResetAction = _ui->actionObjectReset;
    _objectAlternateAction = _ui->actionObjectAlternateMaterials;
    _objectPropertiesAction = _ui->actionObjectProperties;
    _cameraFrontAction = _ui->actionCameraFront;
    _cameraBackAction = _ui->actionCameraBack;
    _cameraLeftAction = _ui->actionCameraLeft;
    _cameraRightAction = _ui->actionCameraRight;
    _cameraTopAction = _ui->actionCameraTop;
    _cameraBottomAction = _ui->actionCameraBottom;
    _cameraRotateXAction = _ui->actionCameraRotateX;
    _cameraRotateYAction = _ui->actionCameraRotateY;
    _cameraRotateZAction = _ui->actionCameraRotateZ;
    _cameraCopyScreenAction = _ui->actionCameraCopyScreen;
    _cameraAnimateAction = _ui->actionCameraAnimate;
    _cameraResetOnDisplayAction = _ui->actionCameraResetOnDisplay;
    _cameraResetAction = _ui->actionCameraReset;
    _cameraBonePosXAction = _ui->actionCameraBonePosX;
    _cameraSettingsAction = _ui->actionCameraSettings;
    _cameraDistanceAction = _ui->actionCameraDistance;
    _npatchesGapAction = _ui->actionNpatchesGap;
    _lightRotateYAction = _ui->actionLightRotateY;
    _lightRotateZAction = _ui->actionLightRotateZ;
    _exposePrelitAction = _ui->actionExposePrelit;
    _prelitVertexAction = _ui->actionPrelitVertex;
    _prelitMultipassAction = _ui->actionPrelitMultipass;
    _prelitMultitexAction = _ui->actionPrelitMultitex;
    applyMainToolbarIcons();

    connect(_newAction, &QAction::triggered, this, &W3DViewMainWindow::newFile);
    connect(_openAction, &QAction::triggered, this, &W3DViewMainWindow::openFile);
    connect(_mungeSortAction, &QAction::triggered, this, &W3DViewMainWindow::toggleMungeSortOnLoad);
    connect(_enableGammaAction, &QAction::triggered, this, &W3DViewMainWindow::toggleGammaCorrection);
    connect(_saveSettingsAction, &QAction::triggered, this, &W3DViewMainWindow::saveSettingsFile);
    connect(_loadSettingsAction, &QAction::triggered, this, &W3DViewMainWindow::loadSettingsFile);
    connect(_ui->actionImportFacialAnims, &QAction::triggered,
            this, &W3DViewMainWindow::importFacialAnims);
    connect(_exportAggregateAction, &QAction::triggered, this, &W3DViewMainWindow::exportAggregate);
    connect(_exportEmitterAction, &QAction::triggered, this, &W3DViewMainWindow::exportEmitter);
    connect(_exportLodAction, &QAction::triggered, this, &W3DViewMainWindow::exportLod);
    connect(_exportPrimitiveAction, &QAction::triggered, this, &W3DViewMainWindow::exportPrimitive);
    connect(_exportSoundObjectAction, &QAction::triggered, this, &W3DViewMainWindow::exportSoundObject);
    connect(_ui->actionFileTexturePath, &QAction::triggered,
            this, &W3DViewMainWindow::openTexturePathsDialog);
    connect(_ui->actionAnimatedSoundOptions, &QAction::triggered,
            this, &W3DViewMainWindow::openAnimatedSoundOptions);
    updateRecentFilesMenu();
    connect(_ui->actionExit, &QAction::triggered, this, &QWidget::close);

    connect(_texturePathsAction, &QAction::triggered, this, &W3DViewMainWindow::openTexturePathsDialog);
    connect(_autoExpandTreeAction, &QAction::toggled, this, &W3DViewMainWindow::toggleAutoExpandAssetTree);

    connect(_toolbarMainAction, &QAction::toggled, this, &W3DViewMainWindow::toggleMainToolbar);
    connect(_toolbarObjectAction, &QAction::toggled, this, &W3DViewMainWindow::toggleObjectToolbar);
    connect(_toolbarAnimationAction, &QAction::toggled, this, &W3DViewMainWindow::toggleAnimationToolbar);
    connect(_animationToolbar, &QToolBar::visibilityChanged, this, [this](bool visible) {
        if (_toolbarAnimationAction) {
            const QSignalBlocker blocker(_toolbarAnimationAction);
            _toolbarAnimationAction->setChecked(visible);
        }
        if (!_changingAnimationToolbarForSelection) {
            _showAnimationToolbar = visible;
        }
    });
    connect(_statusBarAction, &QAction::triggered, this, &W3DViewMainWindow::toggleStatusBar);
    connect(_slideshowPrevAction, &QAction::triggered, this, &W3DViewMainWindow::selectPrevAsset);
    connect(_slideshowNextAction, &QAction::triggered, this, &W3DViewMainWindow::selectNextAsset);
    connect(_wireframeAction, &QAction::triggered, this, &W3DViewMainWindow::toggleWireframe);
    connect(_sortingAction, &QAction::triggered, this, &W3DViewMainWindow::toggleSorting);
    connect(_invertBackfaceCullingAction, &QAction::triggered,
            this, &W3DViewMainWindow::toggleBackfaceCulling);
    connect(_gammaAction, &QAction::triggered, this, &W3DViewMainWindow::openGammaDialog);
    connect(_ui->actionChangeResolution,
            &QAction::triggered,
            this,
            &W3DViewMainWindow::changeResolution);
    _npatchesGroup = new QActionGroup(this);
    _npatchesGroup->setObjectName("npatchesGroup");
    _npatchesGroup->setExclusive(true);
    for (int level = 1; level <= 8; ++level) {
        auto *action = _ui->npatchesMenu->addAction(QString::number(level));
        action->setObjectName(QString("actionNpatchesLevel%1").arg(level));
        action->setCheckable(true);
        action->setData(level);
        _npatchesGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, level]() { setNpatchesLevel(level); });
    }
    connect(_npatchesGapAction, &QAction::triggered, this, &W3DViewMainWindow::toggleNpatchesGap);

    connect(_objectRotateXAction, &QAction::triggered, this, &W3DViewMainWindow::toggleObjectRotateX);
    _objectRotateYAction->setShortcuts(
        QList<QKeySequence>{QKeySequence(Qt::Key_Up), QKeySequence(Qt::CTRL | Qt::Key_Y)});
    connect(_objectRotateYAction, &QAction::triggered, this, &W3DViewMainWindow::toggleObjectRotateY);
    _objectRotateZAction->setShortcuts(
        QList<QKeySequence>{QKeySequence(Qt::Key_Right), QKeySequence(Qt::CTRL | Qt::Key_Z)});
    connect(_objectRotateZAction, &QAction::triggered, this, &W3DViewMainWindow::toggleObjectRotateZ);
    connect(_objectPropertiesAction, &QAction::triggered, this, &W3DViewMainWindow::showObjectProperties);
    connect(_restrictAnimsAction, &QAction::triggered, this, &W3DViewMainWindow::toggleRestrictAnims);
    connect(_objectResetAction, &QAction::triggered, this, &W3DViewMainWindow::resetObject);
    connect(_objectAlternateAction, &QAction::triggered, this, &W3DViewMainWindow::toggleAlternateMaterials);

    _animationMenu = new QMenu("&Animation", this);
    _animationMenu->setObjectName("animationMenu");
    _animationPlayAction = _ui->actionToolbarAnimationPlay;
    _animationPauseAction = _ui->actionToolbarAnimationPause;
    _animationStopAction = _ui->actionToolbarAnimationStop;
    _animationStepBackAction = _ui->actionToolbarAnimationStepBack;
    _animationStepForwardAction = _ui->actionToolbarAnimationStepForward;
    _animationMenu->addAction(_animationPlayAction);
    _animationMenu->addAction(_animationPauseAction);
    _animationMenu->addAction(_animationStopAction);
    connect(_animationPlayAction, &QAction::triggered, this, &W3DViewMainWindow::startAnimation);
    connect(_animationPauseAction, &QAction::triggered, this, &W3DViewMainWindow::pauseAnimation);
    connect(_animationStopAction, &QAction::triggered, this, &W3DViewMainWindow::stopAnimation);
    _animationMenu->addSeparator();
    _animationMenu->addAction(_animationStepBackAction);
    _animationMenu->addAction(_animationStepForwardAction);
    connect(_animationStepBackAction, &QAction::triggered, this, &W3DViewMainWindow::stepAnimationBackward);
    connect(_animationStepForwardAction, &QAction::triggered, this, &W3DViewMainWindow::stepAnimationForward);
    _animationMenu->addSeparator();
    auto *animation_settings_action = _animationMenu->addAction("Se&ttings");
    animation_settings_action->setObjectName("actionAnimationSettings");
    connect(animation_settings_action, &QAction::triggered, this, &W3DViewMainWindow::openAnimationSettings);
    _animationMenu->addSeparator();
    auto *animation_advanced_action = _animationMenu->addAction("Ad&vanced...");
    animation_advanced_action->setObjectName("actionAnimationAdvanced");
    connect(animation_advanced_action, &QAction::triggered, this, &W3DViewMainWindow::openAdvancedAnimation);
    connect(_animationMenu, &QMenu::aboutToShow, this, &W3DViewMainWindow::refreshAnimationMenu);

    _hierarchyMenu = new QMenu("&Hierarchy", this);
    _hierarchyMenu->setObjectName("hierarchyMenu");
    auto *hierarchy_generate_action = _hierarchyMenu->addAction("&Generate LOD...");
    hierarchy_generate_action->setObjectName("actionHierarchyGenerateLod");
    connect(hierarchy_generate_action, &QAction::triggered, this, &W3DViewMainWindow::generateLod);
    auto *hierarchy_aggregate_action = _hierarchyMenu->addAction("&Make Aggregate...");
    hierarchy_aggregate_action->setObjectName("actionHierarchyMakeAggregate");
    connect(hierarchy_aggregate_action, &QAction::triggered, this, &W3DViewMainWindow::makeAggregate);

    _aggregateMenu = new QMenu("&Aggregate", this);
    _aggregateMenu->setObjectName("aggregateMenu");
    auto *aggregate_rename_action = _aggregateMenu->addAction("R&ename Aggregate...");
    aggregate_rename_action->setObjectName("actionAggregateRename");
    connect(aggregate_rename_action, &QAction::triggered, this, &W3DViewMainWindow::renameAggregate);
    _aggregateMenu->addSeparator();
    auto *aggregate_bone_action = _aggregateMenu->addAction("&Bone Management...");
    aggregate_bone_action->setObjectName("actionAggregateBoneManagement");
    connect(aggregate_bone_action, &QAction::triggered, this, &W3DViewMainWindow::openBoneManagement);
    auto *aggregate_auto_assign_action = _aggregateMenu->addAction("&Auto Assign Bone Models");
    aggregate_auto_assign_action->setObjectName("actionAggregateAutoAssignBones");
    connect(aggregate_auto_assign_action, &QAction::triggered, this, &W3DViewMainWindow::autoAssignBoneModels);
    _aggregateMenu->addSeparator();
    _aggregateBindSubobjectAction = _aggregateMenu->addAction("Bind &Subobject LOD");
    _aggregateBindSubobjectAction->setObjectName("actionAggregateBindSubobjectLod");
    _aggregateBindSubobjectAction->setCheckable(true);
    connect(_aggregateBindSubobjectAction, &QAction::triggered, this, &W3DViewMainWindow::bindSubobjectLod);
    auto *aggregate_generate_action = _aggregateMenu->addAction("&Generate LOD...");
    aggregate_generate_action->setObjectName("actionAggregateGenerateLod");
    connect(aggregate_generate_action, &QAction::triggered, this, &W3DViewMainWindow::generateLod);
    connect(_aggregateMenu, &QMenu::aboutToShow, this, &W3DViewMainWindow::refreshAggregateMenu);

    _lodMenu = new QMenu("&LOD", this);
    _lodMenu->setObjectName("lodMenu");
    _lodRecordAction = _lodMenu->addAction("&Record Screen Area");
    _lodRecordAction->setObjectName("actionLodRecordScreenArea");
    connect(_lodRecordAction, &QAction::triggered, this, &W3DViewMainWindow::recordLodScreenArea);
    _lodIncludeNullAction = _lodMenu->addAction("Include &NULL Object");
    _lodIncludeNullAction->setObjectName("actionLodIncludeNull");
    _lodIncludeNullAction->setCheckable(true);
    connect(_lodIncludeNullAction, &QAction::triggered, this, &W3DViewMainWindow::toggleLodIncludeNull);
    _lodMenu->addSeparator();
    _lodPrevAction = _lodMenu->addAction("&Prev Level");
    _lodPrevAction->setObjectName("actionLodPrevious");
    connect(_lodPrevAction, &QAction::triggered, this, &W3DViewMainWindow::selectPrevLod);
    _lodNextAction = _lodMenu->addAction("&Next Level");
    _lodNextAction->setObjectName("actionLodNext");
    connect(_lodNextAction, &QAction::triggered, this, &W3DViewMainWindow::selectNextLod);
    _lodAutoSwitchAction = _lodMenu->addAction("&Auto Switching");
    _lodAutoSwitchAction->setObjectName("actionLodAutoSwitch");
    _lodAutoSwitchAction->setCheckable(true);
    connect(_lodAutoSwitchAction, &QAction::triggered, this, &W3DViewMainWindow::toggleLodAutoSwitch);
    _lodMenu->addSeparator();
    auto *lod_make_aggregate_action = _lodMenu->addAction("&Make Aggregate...");
    lod_make_aggregate_action->setObjectName("actionLodMakeAggregate");
    connect(lod_make_aggregate_action, &QAction::triggered, this, &W3DViewMainWindow::makeAggregate);
    connect(_lodMenu, &QMenu::aboutToShow, this, &W3DViewMainWindow::refreshLodMenu);

    connect(_ui->actionCreateEmitter, &QAction::triggered,
            this, &W3DViewMainWindow::createEmitter);
    connect(_scaleEmitterAction, &QAction::triggered, this, &W3DViewMainWindow::scaleEmitter);
    connect(_editEmitterAction, &QAction::triggered, this, &W3DViewMainWindow::editEmitter);
    connect(_emittersEditMenu, &QMenu::aboutToShow, this, &W3DViewMainWindow::updateEmittersEditMenu);

    connect(_ui->actionCreateSphere, &QAction::triggered,
            this, &W3DViewMainWindow::createSphere);
    connect(_ui->actionCreateRing, &QAction::triggered,
            this, &W3DViewMainWindow::createRing);
    connect(_editPrimitiveAction, &QAction::triggered,
            this, &W3DViewMainWindow::editPrimitive);

    connect(_ui->actionCreateSoundObject, &QAction::triggered,
            this, &W3DViewMainWindow::createSoundObject);
    connect(_editSoundObjectAction, &QAction::triggered, this, &W3DViewMainWindow::editSoundObject);

    connect(_lightRotateYAction, &QAction::triggered, this, &W3DViewMainWindow::toggleLightRotateY);
    connect(_lightRotateZAction, &QAction::triggered, this, &W3DViewMainWindow::toggleLightRotateZ);
    connect(_ui->actionAmbientLight, &QAction::triggered,
            this, &W3DViewMainWindow::setAmbientLight);
    connect(_ui->actionSceneLight, &QAction::triggered,
            this, &W3DViewMainWindow::setSceneLight);
    _ui->actionIncreaseAmbientLight->setShortcuts(
        QList<QKeySequence>{QKeySequence(Qt::Key_Plus), QKeySequence(Qt::Key_Equal)});
    connect(_ui->actionIncreaseAmbientLight, &QAction::triggered,
            this, &W3DViewMainWindow::increaseAmbientLight);
    connect(_ui->actionDecreaseAmbientLight, &QAction::triggered,
            this, &W3DViewMainWindow::decreaseAmbientLight);
    _ui->actionIncreaseSceneLight->setShortcuts(
        QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_Plus),
                            QKeySequence(Qt::CTRL | Qt::Key_Equal)});
    connect(_ui->actionIncreaseSceneLight, &QAction::triggered,
            this, &W3DViewMainWindow::increaseSceneLight);
    connect(_ui->actionDecreaseSceneLight, &QAction::triggered,
            this, &W3DViewMainWindow::decreaseSceneLight);
    connect(_exposePrelitAction, &QAction::triggered, this, &W3DViewMainWindow::toggleExposePrelit);
    connect(_ui->actionKillSceneLight, &QAction::triggered,
            this, &W3DViewMainWindow::killSceneLight);
    _prelitGroup = new QActionGroup(this);
    _prelitGroup->setObjectName("prelitGroup");
    _prelitGroup->addAction(_prelitVertexAction);
    connect(_prelitVertexAction, &QAction::triggered, this, &W3DViewMainWindow::setPrelitVertex);
    _prelitGroup->addAction(_prelitMultipassAction);
    connect(_prelitMultipassAction, &QAction::triggered, this, &W3DViewMainWindow::setPrelitMultipass);
    _prelitGroup->addAction(_prelitMultitexAction);
    connect(_prelitMultitexAction, &QAction::triggered, this, &W3DViewMainWindow::setPrelitMultitex);

    connect(_cameraFrontAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraFront);
    connect(_cameraBackAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraBack);
    connect(_cameraLeftAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraLeft);
    connect(_cameraRightAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraRight);
    connect(_cameraTopAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraTop);
    connect(_cameraBottomAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraBottom);
    connect(_cameraRotateXAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraRotateX);
    connect(_cameraRotateYAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraRotateY);
    connect(_cameraRotateZAction, &QAction::triggered, this, &W3DViewMainWindow::setCameraRotateZ);
    connect(_cameraCopyScreenAction, &QAction::triggered, this, &W3DViewMainWindow::copyScreenSize);
    connect(_cameraAnimateAction, &QAction::triggered, this, &W3DViewMainWindow::toggleCameraAnimate);
    connect(_cameraBonePosXAction, &QAction::triggered, this, &W3DViewMainWindow::toggleCameraBonePosX);
    connect(_cameraSettingsAction, &QAction::triggered, this, &W3DViewMainWindow::openCameraSettings);
    connect(_cameraDistanceAction, &QAction::triggered, this, &W3DViewMainWindow::openCameraDistance);
    connect(_cameraResetOnDisplayAction, &QAction::triggered, this, &W3DViewMainWindow::toggleCameraResetOnDisplay);
    connect(_cameraResetAction, &QAction::triggered, this, &W3DViewMainWindow::resetCamera);

    connect(_ui->actionBackgroundColor, &QAction::triggered,
            this, &W3DViewMainWindow::setBackgroundColor);
    connect(_ui->actionBackgroundBitmap, &QAction::triggered,
            this, &W3DViewMainWindow::setBackgroundBitmap);
    connect(_backgroundObjectAction, &QAction::triggered, this,
            &W3DViewMainWindow::openBackgroundObjectDialog);
    connect(_fogAction, &QAction::triggered, this, &W3DViewMainWindow::toggleFog);

    connect(_makeMovieAction, &QAction::triggered, this, &W3DViewMainWindow::makeMovie);
    connect(_captureScreenshotAction, &QAction::triggered, this,
            &W3DViewMainWindow::captureScreenshot);

    connect(_aboutAction, &QAction::triggered, this, &W3DViewMainWindow::showAbout);

    auto *make_aggregate_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_A)});
    if (make_aggregate_shortcut != nullptr) {
        make_aggregate_shortcut->setObjectName("shortcutMakeAggregate");
        connect(make_aggregate_shortcut, &QAction::triggered, this, &W3DViewMainWindow::makeAggregate);
    }
    auto *advanced_animation_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_V)});
    if (advanced_animation_shortcut != nullptr) {
        advanced_animation_shortcut->setObjectName("shortcutAdvancedAnimation");
        connect(advanced_animation_shortcut, &QAction::triggered,
                this, &W3DViewMainWindow::openAdvancedAnimation);
    }
    auto *lod_record_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::Key_Space)});
    if (lod_record_shortcut != nullptr) {
        lod_record_shortcut->setObjectName("shortcutLodRecordScreenArea");
        connect(lod_record_shortcut, &QAction::triggered, this, &W3DViewMainWindow::recordLodScreenArea);
    }
    auto *lod_prev_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::Key_BracketLeft)});
    if (lod_prev_shortcut != nullptr) {
        lod_prev_shortcut->setObjectName("shortcutLodPrevious");
        connect(lod_prev_shortcut, &QAction::triggered, this, &W3DViewMainWindow::selectPrevLod);
    }
    auto *lod_next_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::Key_BracketRight)});
    if (lod_next_shortcut != nullptr) {
        lod_next_shortcut->setObjectName("shortcutLodNext");
        connect(lod_next_shortcut, &QAction::triggered, this, &W3DViewMainWindow::selectNextLod);
    }
    auto *object_rotate_y_back_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::Key_Down)});
    if (object_rotate_y_back_shortcut != nullptr) {
        object_rotate_y_back_shortcut->setObjectName("shortcutObjectRotateYBack");
        connect(object_rotate_y_back_shortcut, &QAction::triggered,
                this, &W3DViewMainWindow::toggleObjectRotateYBack);
    }
    auto *object_rotate_z_back_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::Key_Left)});
    if (object_rotate_z_back_shortcut != nullptr) {
        object_rotate_z_back_shortcut->setObjectName("shortcutObjectRotateZBack");
        connect(object_rotate_z_back_shortcut, &QAction::triggered,
                this, &W3DViewMainWindow::toggleObjectRotateZBack);
    }
    auto *light_rotate_y_back_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_Down)});
    if (light_rotate_y_back_shortcut != nullptr) {
        light_rotate_y_back_shortcut->setObjectName("shortcutLightRotateYBack");
        connect(light_rotate_y_back_shortcut, &QAction::triggered,
                this, &W3DViewMainWindow::toggleLightRotateYBack);
    }
    auto *light_rotate_z_back_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_Left)});
    if (light_rotate_z_back_shortcut != nullptr) {
        light_rotate_z_back_shortcut->setObjectName("shortcutLightRotateZBack");
        connect(light_rotate_z_back_shortcut, &QAction::triggered,
                this, &W3DViewMainWindow::toggleLightRotateZBack);
    }
    for (int slot = 1; slot <= 9; ++slot) {
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_1 + (slot - 1));
        auto *settings_shortcut =
            qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(key)});
        if (settings_shortcut != nullptr) {
            settings_shortcut->setObjectName(QString("shortcutQuickSettings%1").arg(slot));
            connect(settings_shortcut, &QAction::triggered, this, [this, slot]() {
                loadQuickSettings(slot);
            });
        }
    }
    auto *next_pane_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::Key_F6)});
    if (next_pane_shortcut != nullptr) {
        next_pane_shortcut->setObjectName("shortcutNextPane");
        connect(next_pane_shortcut, &QAction::triggered, this, [this]() { cyclePaneFocus(false); });
    }
    auto *prev_pane_shortcut =
        qtcommon::CreateWindowShortcutAction(this, QList<QKeySequence>{QKeySequence(Qt::SHIFT | Qt::Key_F6)});
    if (prev_pane_shortcut != nullptr) {
        prev_pane_shortcut->setObjectName("shortcutPreviousPane");
        connect(prev_pane_shortcut, &QAction::triggered, this, [this]() { cyclePaneFocus(true); });
    }

    connect(_listMissingTexturesAction, &QAction::triggered, this,
            &W3DViewMainWindow::listMissingTextures);
    connect(_copyAssetsAction, &QAction::triggered, this, &W3DViewMainWindow::copyAssets);
    connect(_addToLineupAction, &QAction::triggered, this, &W3DViewMainWindow::addToLineup);
    statusBar()->showMessage("Ready");

    _treeView = _ui->assetTreeView;
    _treeModel = new QStandardItemModel(_treeView);
    _treeModel->setHorizontalHeaderLabels(QStringList() << "Assets");
    _treeView->setModel(_treeModel);
    _treeView->setHeaderHidden(false);
    _treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &W3DViewMainWindow::onCurrentChanged);
    connect(_treeView, &QTreeView::customContextMenuRequested,
            this, &W3DViewMainWindow::showTreeContextMenu);

    _viewport = _ui->viewport;
    connect(_viewport,
            &W3DViewport::animationStateChanged,
            this,
            &W3DViewMainWindow::refreshAnimationMenu);
    connect(_viewport,
            &W3DViewport::objectCameraReset,
            this,
            &W3DViewMainWindow::loadDefaultSettings);
    refreshAnimationMenu();

    _statusPolysLabel = _ui->statusPolysLabel;
    _statusParticlesLabel = _ui->statusParticlesLabel;
    _statusCameraLabel = _ui->statusCameraLabel;
    _statusFramesLabel = _ui->statusFramesLabel;
    _statusFpsLabel = _ui->statusFpsLabel;
    _statusResolutionLabel = _ui->statusResolutionLabel;

    if (statusBar()) {
        statusBar()->addPermanentWidget(_ui->permanentStatusPanel, 1);
    }

    _statusTimer = new QTimer(this);
    _statusTimer->setInterval(250);
    connect(_statusTimer, &QTimer::timeout, this, &W3DViewMainWindow::updateStatusBar);
    _statusTimer->start();

    _ui->mainSplitter->setStretchFactor(0, 0);
    _ui->mainSplitter->setStretchFactor(1, 1);
    _ui->mainSplitter->setSizes({240, 800});

    loadAppSettings();
    if (_animationToolbar) {
        _changingAnimationToolbarForSelection = true;
        _animationToolbar->hide();
        _changingAnimationToolbarForSelection = false;
    }
    loadDefaultSettings();
    if (_restrictAnimsAction) {
        _restrictAnimsAction->setChecked(_restrictAnims);
    }
    if (_sortingAction) {
        _sortingAction->setChecked(_sortingEnabled);
    }
    if (_invertBackfaceCullingAction) {
        _invertBackfaceCullingAction->setChecked(ShaderClass::Is_Backface_Culling_Inverted());
    }
    if (_wireframeAction && _viewport) {
        _wireframeAction->setChecked(_viewport->isWireframeEnabled());
    }
    if (_toolbarMainAction && _mainToolbar) {
        const QSignalBlocker blocker(_toolbarMainAction);
        _toolbarMainAction->setChecked(!_mainToolbar->isHidden());
    }
    if (_toolbarObjectAction && _objectToolbar) {
        const QSignalBlocker blocker(_toolbarObjectAction);
        _toolbarObjectAction->setChecked(!_objectToolbar->isHidden());
    }
    if (_toolbarAnimationAction && _animationToolbar) {
        const QSignalBlocker blocker(_toolbarAnimationAction);
        _toolbarAnimationAction->setChecked(!_animationToolbar->isHidden());
    }
    if (_statusBarAction) {
        _statusBarAction->setChecked(statusBar() && !statusBar()->isHidden());
    }
    if (_fogAction && _viewport) {
        _fogAction->setChecked(_viewport->isFogEnabled());
    }
    if (_cameraResetOnDisplayAction) {
        _cameraResetOnDisplayAction->setChecked(_autoResetCamera);
    }
    if (_cameraAnimateAction) {
        _cameraAnimateAction->setChecked(_animateCamera);
    }
    if (_cameraBonePosXAction && _viewport) {
        _cameraBonePosXAction->setChecked(_viewport->isCameraBonePosX());
    }
    if (_exposePrelitAction) {
        _exposePrelitAction->setChecked(WW3D::Expose_Prelit());
    }
    if (_prelitGroup) {
        const WW3D::PrelitModeEnum mode = WW3D::Get_Prelit_Mode();
        if (_prelitVertexAction && mode == WW3D::PRELIT_MODE_VERTEX) {
            _prelitVertexAction->setChecked(true);
        } else if (_prelitMultipassAction && mode == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS) {
            _prelitMultipassAction->setChecked(true);
        } else if (_prelitMultitexAction && mode == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE) {
            _prelitMultitexAction->setChecked(true);
        }
    }
    rebuildAssetTree();
}

W3DViewMainWindow::~W3DViewMainWindow()
{
    stopAnimationSound();
    if (_viewport) {
        // QObject deletes child actions and the central viewport only after this
        // destructor body. Their relative child order is not an API guarantee,
        // so the viewport must not emit menu-refresh signals after actions have
        // begun disappearing in QMainWindow's base destructor.
        disconnect(_viewport, nullptr, this, nullptr);

        // Release asset-manager-owned animation/render references while the
        // complete window object is still alive. QObject deletes child widgets
        // only after this destructor body has finished.
        _viewport->clearAnimation();
        _viewport->setRenderObject(nullptr);
    }
    delete _ui;
}

bool W3DViewMainWindow::openFilePath(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, "W3DViewQt", QString("File not found:\n%1").arg(path));
        return false;
    }
    return loadAssetsFromFile(info.absoluteFilePath());
}

bool W3DViewMainWindow::loadSettingsPath(const QString &path)
{
    const QFileInfo info(path);
    if (!_viewport || !info.exists() || !info.isFile()) {
        return false;
    }

    QSettings settings(info.absoluteFilePath(), QSettings::IniFormat);
    settings.sync();
    settings.allKeys();
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    applySettings(settings);
    return settings.status() == QSettings::NoError;
}

void W3DViewMainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("Window/Geometry", saveGeometry());
    settings.setValue("Window/State", saveState());
    QMainWindow::closeEvent(event);
    if (event && event->isAccepted()) {
        QCoreApplication::quit();
    }
}

void W3DViewMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event) {
        return;
    }

    const QMimeData *mime = event->mimeData();
    if (!mime || !mime->hasUrls()) {
        return;
    }

    const auto urls = mime->urls();
    for (const auto &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        if (path.endsWith(".w3d", Qt::CaseInsensitive)) {
            event->acceptProposedAction();
            return;
        }
    }
}

void W3DViewMainWindow::dropEvent(QDropEvent *event)
{
    if (!event) {
        return;
    }

    const QMimeData *mime = event->mimeData();
    if (!mime || !mime->hasUrls()) {
        return;
    }

    bool loaded_any = false;
    const auto urls = mime->urls();
    for (const auto &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        if (!path.endsWith(".w3d", Qt::CaseInsensitive)) {
            continue;
        }
        if (loadAssetsFromFile(path)) {
            loaded_any = true;
        }
    }

    if (loaded_any) {
        event->acceptProposedAction();
    }
}

void W3DViewMainWindow::openFile()
{
    const QStringList paths = QFileDialog::getOpenFileNames(
        this,
        "Open W3D Assets",
        _lastOpenedPath,
        "W3D Assets (*.w3d);;All Files (*.*)");

    if (paths.isEmpty()) {
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    for (const QString &path : paths) {
        loadAssetsFromFile(path);
    }
    QGuiApplication::restoreOverrideCursor();
}

void W3DViewMainWindow::openRecentFile()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }

    const QString path = action->data().toString();
    if (path.isEmpty()) {
        return;
    }

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, "W3DViewQt", QString("File not found:\n%1").arg(path));
        QSettings settings;
        const QStringList files = qtcommon::RemoveRecentFile(
            qtcommon::ReadRecentFiles(settings, QStringLiteral("recentFiles"), kMaxRecentFiles),
            path);
        qtcommon::WriteRecentFiles(settings, files, QStringLiteral("recentFiles"), kMaxRecentFiles);
        updateRecentFilesMenu();
        return;
    }

    if (!loadAssetsFromFile(path)) {
        QSettings settings;
        const QStringList files = qtcommon::RemoveRecentFile(
            qtcommon::ReadRecentFiles(settings, QStringLiteral("recentFiles"), kMaxRecentFiles),
            path);
        qtcommon::WriteRecentFiles(settings, files, QStringLiteral("recentFiles"), kMaxRecentFiles);
        updateRecentFilesMenu();
    }
}

void W3DViewMainWindow::newFile()
{
    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(nullptr);
        _viewport->requestOneTimeCameraReset();
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (asset_manager) {
        asset_manager->Free_Assets();
        asset_manager->Load_Procedural_Textures();
    }

    // MFC preserves its last-open directory when creating a new document.
    // Keeping it also makes the next Open dialog start in the same place.
    _loadedFiles.clear();
    setWindowTitle("W3DViewQt");
    rebuildAssetTree();
    statusBar()->showMessage("Cleared assets.");
}

void W3DViewMainWindow::openTexturePathsDialog()
{
    TexturePathDialog dialog(_texturePath1, _texturePath2, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    setTexturePaths(dialog.path1(), dialog.path2());
}

void W3DViewMainWindow::loadSettingsFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Load Settings",
        _lastOpenedPath,
        "W3D Settings (*.dat *.ini);;All Files (*.*)");

    if (path.isEmpty() || !_viewport) {
        return;
    }

    if (!loadSettingsPath(path)) {
        QMessageBox::warning(this, "Load Settings", "Unable to read the selected settings file.");
        return;
    }
    statusBar()->showMessage(QString("Loaded settings: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::saveSettingsFile()
{
    if (!_viewport) {
        return;
    }

    SaveSettingsDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString path = dialog.selectedPath();
    QSettings settings(path, QSettings::IniFormat);
    writeSettings(settings, dialog.saveLighting(), dialog.saveBackground());
    settings.sync();

    if (settings.status() != QSettings::NoError) {
        QMessageBox::warning(this, "Save Settings", "Unable to write the selected settings file.");
        return;
    }

    statusBar()->showMessage(QString("Saved settings: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::loadQuickSettings(int slot)
{
    if (slot < 1 || slot > 9 || !_viewport) {
        return;
    }

    const QString path =
        QDir(QCoreApplication::applicationDirPath()).filePath(QString("settings%1.dat").arg(slot));
    if (!QFileInfo::exists(path)) {
        return;
    }

    loadSettingsPath(path);
}

void W3DViewMainWindow::cyclePaneFocus(bool reverse)
{
    if (!_treeView || !_viewport) {
        return;
    }

    QWidget *focused = QApplication::focusWidget();
    const bool tree_has_focus = focused == _treeView || _treeView->isAncestorOf(focused);
    const bool viewport_has_focus = focused == _viewport || _viewport->isAncestorOf(focused);
    if (tree_has_focus) {
        _viewport->setFocus();
        return;
    }
    if (viewport_has_focus) {
        _treeView->setFocus();
        return;
    }

    if (reverse) {
        _viewport->setFocus();
    } else {
        _treeView->setFocus();
    }
}

void W3DViewMainWindow::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    updateSpecialMenu(current);
    if (!_viewport) {
        return;
    }

    const int type_value = current.data(kRoleType).toInt();
    const bool is_render_object = type_value == static_cast<int>(AssetNodeType::RenderObject);
    const int class_id = current.data(kRoleClassId).toInt();
    const bool is_sound = is_render_object && class_id == RenderObjClass::CLASSID_SOUND;
    const bool is_emitter = is_render_object && class_id == RenderObjClass::CLASSID_PARTICLEEMITTER;
    const bool is_primitive = is_render_object &&
        (class_id == RenderObjClass::CLASSID_SPHERE || class_id == RenderObjClass::CLASSID_RING);
    const bool is_animation = type_value == static_cast<int>(AssetNodeType::Animation);
    if (_selectionIsAnimation != is_animation && _animationToolbar) {
        _changingAnimationToolbarForSelection = true;
        if (_selectionIsAnimation) {
            _showAnimationToolbar = _animationToolbar->isVisible();
            _animationToolbar->hide();
        }
        _selectionIsAnimation = is_animation;
        if (_selectionIsAnimation && _showAnimationToolbar) {
            _animationToolbar->show();
        }
        _changingAnimationToolbarForSelection = false;
    }
    RenderObjInfo selected_info;
    if (is_render_object) {
        const QString selected_name = current.data(kRoleName).toString();
        if (!selected_name.isEmpty()) {
            const QByteArray selected_name_bytes = selected_name.toLatin1();
            selected_info = InspectRenderObj(selected_name_bytes.constData());
        }
    }
    const bool is_aggregate = is_render_object && selected_info.isAggregate;
    const bool is_lod = is_render_object && selected_info.isRealLod && !selected_info.isAggregate;
    const bool has_hierarchy = is_animation ||
        (is_render_object && !selected_info.hierarchyName.isEmpty());
    bool can_lineup = false;
    if (is_render_object && _viewport) {
        can_lineup = _viewport->canLineUpClass(class_id);
    }
    if (_copyAssetsAction) {
        _copyAssetsAction->setEnabled(is_render_object);
    }
    if (_addToLineupAction) {
        _addToLineupAction->setEnabled(can_lineup);
    }
    if (_editSoundObjectAction) {
        _editSoundObjectAction->setEnabled(is_sound);
    }
    if (_exportSoundObjectAction) {
        _exportSoundObjectAction->setEnabled(is_sound);
    }
    if (_exportEmitterAction) {
        _exportEmitterAction->setEnabled(is_emitter);
    }
    if (_exportAggregateAction) {
        _exportAggregateAction->setEnabled(is_aggregate);
    }
    if (_exportLodAction) {
        _exportLodAction->setEnabled(is_lod);
    }
    if (_exportPrimitiveAction) {
        _exportPrimitiveAction->setEnabled(is_primitive);
    }
    if (_editEmitterAction) {
        _editEmitterAction->setEnabled(is_emitter);
    }
    if (_scaleEmitterAction) {
        _scaleEmitterAction->setEnabled(is_emitter);
    }
    if (_editPrimitiveAction) {
        _editPrimitiveAction->setEnabled(is_primitive);
    }
    if (_objectPropertiesAction) {
        _objectPropertiesAction->setEnabled(is_render_object || is_animation);
    }
    if (_makeMovieAction) {
        _makeMovieAction->setEnabled(is_animation);
    }
    _ui->actionImportFacialAnims->setEnabled(has_hierarchy);
    if (type_value == static_cast<int>(AssetNodeType::RenderObject)) {
        _viewport->clearAnimation();

        const QString name = current.data(kRoleName).toString();
        if (name.isEmpty()) {
            return;
        }

        auto *asset_manager = WW3DAssetManager::Get_Instance();
        if (!asset_manager) {
            return;
        }

        const QByteArray name_bytes = name.toLatin1();
        RenderObjClass *object = asset_manager->Create_Render_Obj(name_bytes.constData());
        if (!object) {
            statusBar()->showMessage(QString("Failed to create render object: %1").arg(name));
            return;
        }

        SetHighestLod(object);
        _viewport->setRenderObject(object);
        object->Release_Ref();
        statusBar()->showMessage(QString("Showing: %1").arg(name));
        updateEmittersEditMenu();
        return;
    }

    if (type_value == static_cast<int>(AssetNodeType::Animation)) {
        _viewport->clearAnimation();
        auto *asset_manager = WW3DAssetManager::Get_Instance();
        if (!asset_manager) {
            return;
        }

        const QString animation_name = current.data(kRoleName).toString();
        if (animation_name.isEmpty()) {
            return;
        }

        QModelIndex render_index = current.parent();
        while (render_index.isValid() &&
               render_index.data(kRoleType).toInt() != static_cast<int>(AssetNodeType::RenderObject)) {
            render_index = render_index.parent();
        }

        if (!render_index.isValid()) {
            return;
        }

        const QString render_name = render_index.data(kRoleName).toString();
        if (render_name.isEmpty()) {
            return;
        }

        const QByteArray render_bytes = render_name.toLatin1();
        RenderObjClass *object = asset_manager->Create_Render_Obj(render_bytes.constData());
        if (!object) {
            statusBar()->showMessage(QString("Failed to create render object: %1").arg(render_name));
            return;
        }

        const QByteArray anim_bytes = animation_name.toLatin1();
        HAnimClass *animation = asset_manager->Get_HAnim(anim_bytes.constData());
        if (!animation) {
            object->Release_Ref();
            statusBar()->showMessage(QString("Failed to load animation: %1").arg(animation_name));
            return;
        }

        SetHighestLod(object);
        _viewport->setRenderObject(object);
        _viewport->setAnimation(animation);
        playAnimationSound();
        object->Release_Ref();
        animation->Release_Ref();
        statusBar()->showMessage(
            QString("Playing: %1 (%2)").arg(animation_name, render_name));
        updateEmittersEditMenu();
        return;
    }

    if (type_value == static_cast<int>(AssetNodeType::Material)) {
        _viewport->clearAnimation();

        const QString name = current.data(kRoleName).toString();
        const quintptr texture_ptr = current.data(kRolePointer).value<quintptr>();
        auto *texture = reinterpret_cast<TextureClass *>(texture_ptr);
        if (!texture) {
            statusBar()->showMessage(QString("Missing texture: %1").arg(name));
            return;
        }

        auto *bitmap = new Bitmap2DObjClass(texture, 0.5f, 0.5f, true, false, false, true);
        _viewport->setRenderObject(bitmap);
        bitmap->Release_Ref();
        statusBar()->showMessage(QString("Showing material: %1").arg(name));
        updateEmittersEditMenu();
        return;
    }

    _viewport->clearAnimation();
    _viewport->setRenderObject(nullptr);
    updateEmittersEditMenu();
    statusBar()->showMessage("No asset selected.");
}

void W3DViewMainWindow::updateSpecialMenu(const QModelIndex &current)
{
    if (!_objectMenuAction || !menuBar()) {
        return;
    }

    QMenu *desired_menu = nullptr;
    const int type_value = current.data(kRoleType).toInt();
    const bool is_group = type_value == static_cast<int>(AssetNodeType::Group);

    auto matches_group = [](const QString &text, const QString &label) {
        return text == label || text.startsWith(label + " (");
    };

    if (type_value == static_cast<int>(AssetNodeType::Animation)) {
        desired_menu = _animationMenu;
    } else if (type_value == static_cast<int>(AssetNodeType::RenderObject) || is_group) {
        const QString group_label = ResolveGroupLabel(_treeModel, current, is_group);
        if (matches_group(group_label, "H-LOD")) {
            desired_menu = _lodMenu;
        } else if (matches_group(group_label, "Hierarchy")) {
            desired_menu = _hierarchyMenu;
        } else if (matches_group(group_label, "Aggregate")) {
            desired_menu = _aggregateMenu;
        }
    }

    QAction *desired_action = desired_menu ? desired_menu->menuAction() : nullptr;
    if (_specialMenuAction && _specialMenuAction != desired_action) {
        menuBar()->removeAction(_specialMenuAction);
    }
    if (desired_action && !menuBar()->actions().contains(desired_action)) {
        menuBar()->insertMenu(_ui->lightingMenu->menuAction(), desired_menu);
    }
    _specialMenuAction = desired_action;
}

void W3DViewMainWindow::updateEmittersEditMenu()
{
    if (!_emittersEditMenu) {
        return;
    }

    _emittersEditMenu->clear();

    QStringList names;
    if (_viewport) {
        if (auto *render_obj = _viewport->currentRenderObject()) {
            CollectEmitterNames(*render_obj, names);
        }
    }

    if (names.isEmpty()) {
        auto *empty_action = _emittersEditMenu->addAction("(No Emitters)");
        empty_action->setEnabled(false);
        return;
    }

    names.sort(Qt::CaseInsensitive);
    for (const auto &name : names) {
        auto *action = _emittersEditMenu->addAction(name);
        connect(action, &QAction::triggered, this, [this, name]() { editEmitterByName(name); });
    }
}

void W3DViewMainWindow::refreshAnimationMenu()
{
    const bool has_anim = _viewport && _viewport->hasAnimation();
    const W3DViewport::AnimationState state =
        has_anim ? _viewport->animationState() : W3DViewport::AnimationState::Stopped;
    if (_animationPlayAction) {
        _animationPlayAction->setEnabled(has_anim);
        _animationPlayAction->setChecked(has_anim && state == W3DViewport::AnimationState::Playing);
    }
    if (_animationPauseAction) {
        _animationPauseAction->setEnabled(has_anim);
        _animationPauseAction->setChecked(has_anim && state == W3DViewport::AnimationState::Paused);
    }
    if (_animationStopAction) {
        _animationStopAction->setEnabled(has_anim);
    }
    if (_animationStepBackAction) {
        _animationStepBackAction->setEnabled(has_anim);
    }
    if (_animationStepForwardAction) {
        _animationStepForwardAction->setEnabled(has_anim);
    }
}

void W3DViewMainWindow::refreshAggregateMenu()
{
    if (!_aggregateBindSubobjectAction) {
        return;
    }
    const bool bound = _viewport && _viewport->isSubobjectLodBound();
    _aggregateBindSubobjectAction->setChecked(bound);
}

void W3DViewMainWindow::refreshLodMenu()
{
    if (!_viewport) {
        if (_lodPrevAction) {
            _lodPrevAction->setEnabled(false);
        }
        if (_lodNextAction) {
            _lodNextAction->setEnabled(false);
        }
        return;
    }

    if (_lodIncludeNullAction) {
        _lodIncludeNullAction->setChecked(_viewport->isNullLodIncluded());
    }
    if (_lodAutoSwitchAction) {
        _lodAutoSwitchAction->setChecked(_viewport->isLodAutoSwitchingEnabled());
    }

    int level = 0;
    int count = 0;
    const bool has_lod = _viewport->currentLodInfo(level, count);
    if (_lodPrevAction) {
        _lodPrevAction->setEnabled(has_lod && level > 0);
    }
    if (_lodNextAction) {
        _lodNextAction->setEnabled(has_lod && (level + 1) < count);
    }
}

bool W3DViewMainWindow::commitEmitterDefinition(const ParticleEmitterDefClass &definition,
                                                const QString &registeredName,
                                                bool reloadCurrentObject,
                                                bool attachedToAggregate)
{
    const char *definition_name = definition.Get_Name();
    const QString updated_name = definition_name ? QString::fromLatin1(definition_name) : QString();
    QString displayed_name;
    if (reloadCurrentObject && _viewport) {
        if (RenderObjClass *displayed_object = _viewport->currentRenderObject()) {
            const char *name = displayed_object->Get_Name();
            if (name) {
                displayed_name = QString::fromLatin1(name);
            }
        }
    }

    const bool is_rename = !registeredName.isEmpty() &&
        registeredName.compare(updated_name, Qt::CaseInsensitive) != 0;
    if (attachedToAggregate && is_rename) {
        QMessageBox::warning(
            this,
            "Apply Emitter",
            "This emitter is attached to an aggregate. Rename it only after removing or rebinding "
            "the aggregate reference; other property changes can still be applied here.");
        return false;
    }

    QString error_message;
    if (!UpdateEmitterPrototype(definition, registeredName, &error_message)) {
        QMessageBox::warning(this,
                             "Apply Emitter",
                             error_message.isEmpty() ? "Failed to register emitter prototype."
                                                     : error_message);
        return false;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (reloadCurrentObject && _viewport && asset_manager && !displayed_name.isEmpty()) {
        const QByteArray displayed_bytes = displayed_name.toLatin1();
        if (RenderObjClass *object = asset_manager->Create_Render_Obj(displayed_bytes.constData())) {
            _viewport->clearAnimation();
            _viewport->setRenderObject(object);
            object->Release_Ref();
        } else {
            QMessageBox::warning(this,
                                 "Apply Emitter",
                                 "The emitter was registered, but the displayed object could not "
                                 "be reloaded.");
        }
    } else if (_viewport && asset_manager && !updated_name.isEmpty()) {
        const QByteArray updated_bytes = updated_name.toLatin1();
        if (RenderObjClass *object = asset_manager->Create_Render_Obj(updated_bytes.constData())) {
            _viewport->clearAnimation();
            _viewport->setRenderObject(object);
            object->Release_Ref();
        } else {
            QMessageBox::warning(this,
                                 "Apply Emitter",
                                 "The emitter was registered, but its preview could not be reloaded.");
        }
    }

    rebuildAssetTree();
    if (!reloadCurrentObject && _treeModel && _treeView) {
        const QModelIndex index = FindRenderObjectIndex(_treeModel,
                                                        updated_name,
                                                        RenderObjClass::CLASSID_PARTICLEEMITTER);
        if (index.isValid()) {
            ExpandParentChain(_treeView, index.parent());
            _treeView->setCurrentIndex(index);
            _treeView->scrollTo(index);
        }
    }

    statusBar()->showMessage(QString("Applied emitter: %1").arg(updated_name));
    return true;
}

void W3DViewMainWindow::editEmitterByName(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Edit Emitter", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Edit Emitter", "Failed to load emitter.");
        return;
    }

    if (render_obj->Class_ID() != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        render_obj->Release_Ref();
        QMessageBox::warning(this, "Edit Emitter", "Selected object is not an emitter.");
        return;
    }

    auto *emitter = static_cast<ParticleEmitterClass *>(render_obj);
    ParticleEmitterDefClass *definition = emitter->Build_Definition();
    emitter->Release_Ref();
    if (!definition) {
        QMessageBox::warning(this, "Edit Emitter", "Failed to load emitter definition.");
        return;
    }

    bool attached_to_aggregate = false;
    if (_viewport) {
        if (RenderObjClass *displayed_object = _viewport->currentRenderObject()) {
            const char *displayed_name = displayed_object->Get_Name();
            if (displayed_name && displayed_name[0]) {
                PrototypeClass *displayed_prototype = asset_manager->Find_Prototype(displayed_name);
                attached_to_aggregate =
                    dynamic_cast<AggregatePrototypeClass *>(displayed_prototype) != nullptr;
            }
        }
    }

    EmitterEditDialog dialog(*definition, this);
    delete definition;
    dialog.setApplyHandler(
        [this, attached_to_aggregate](const ParticleEmitterDefClass &updated,
                                     const QString &registeredName) {
            return commitEmitterDefinition(updated,
                                           registeredName,
                                           true,
                                           attached_to_aggregate);
        },
        dialog.originalName());
    dialog.exec();
}

void W3DViewMainWindow::updateStatusBar()
{
    if (!_viewport) {
        return;
    }

    int polys = 0;
    int particles = 0;
    if (auto *render_obj = _viewport->currentRenderObject()) {
        polys = render_obj->Get_Num_Polys();
        particles = CountParticles(render_obj);
    }

    if (_statusPolysLabel) {
        _statusPolysLabel->setText(QString("Polys %1").arg(polys));
    }
    if (_statusParticlesLabel) {
        _statusParticlesLabel->setText(QString("Particles %1").arg(particles));
    }
    if (_statusCameraLabel) {
        _statusCameraLabel->setText(QString("Camera %1").arg(_viewport->cameraDistance(), 0, 'f', 3));
    }

    int current_frame = 0;
    int total_frames = 0;
    float fps = 0.0f;
    const bool has_anim = _viewport->animationStatus(current_frame, total_frames, fps);
    if (_statusFramesLabel) {
        const int max_frame = total_frames > 0 ? total_frames - 1 : 0;
        const int display_frame = has_anim ? current_frame : 0;
        const float display_fps = has_anim ? fps : 0.0f;
        _statusFramesLabel->setText(
            QString("Frame %1/%2 at %3 fps")
                .arg(display_frame)
                .arg(max_frame)
                .arg(display_fps, 0, 'f', 2));
    }

    if (_statusFpsLabel) {
        const float frame_ms = _viewport->averageFrameMilliseconds();
        if (frame_ms > 0.0f) {
            _statusFpsLabel->setText(QString("Clocks: %1").arg(frame_ms, 0, 'f', 2));
        } else {
            _statusFpsLabel->setText(QString());
        }
    }

    if (_statusResolutionLabel) {
        _statusResolutionLabel->setText(QString(" %1 x %2 ").arg(_viewport->width()).arg(_viewport->height()));
    }
}

void W3DViewMainWindow::toggleWireframe(bool enabled)
{
    if (_viewport) {
        _viewport->setWireframeEnabled(enabled);
    }
}

void W3DViewMainWindow::toggleSorting(bool enabled)
{
    WW3D::_Invalidate_Mesh_Cache();
    WW3D::Enable_Sorting(enabled);
    _sortingEnabled = enabled;

    QSettings settings;
    settings.setValue("Config/EnableSorting", enabled);
}

void W3DViewMainWindow::toggleAutoExpandAssetTree(bool enabled)
{
    _autoExpandAssetTree = enabled;

    QSettings settings;
    settings.setValue("Config/AutoExpandAssetTree", enabled);

    if (!_treeView || !_treeModel) {
        return;
    }

    auto *root = _treeModel->invisibleRootItem();
    if (!root) {
        return;
    }

    const int count = root->rowCount();
    for (int index = 0; index < count; ++index) {
        auto *item = root->child(index);
        if (!item) {
            continue;
        }
        _treeView->setExpanded(item->index(), enabled);
    }
}

void W3DViewMainWindow::toggleBackfaceCulling(bool inverted)
{
    ShaderClass::Invert_Backface_Culling(inverted);
    QSettings settings;
    settings.setValue("Config/InvertBackfaceCulling", inverted);
}

void W3DViewMainWindow::toggleRestrictAnims(bool enabled)
{
    if (_restrictAnims == enabled) {
        return;
    }

    _restrictAnims = enabled;
    rebuildAssetTree();
}

void W3DViewMainWindow::toggleStatusBar(bool visible)
{
    if (statusBar()) {
        statusBar()->setVisible(visible);
    }
}

void W3DViewMainWindow::toggleMainToolbar(bool visible)
{
    if (_mainToolbar) {
        _mainToolbar->setVisible(visible);
    }
}

void W3DViewMainWindow::toggleObjectToolbar(bool visible)
{
    if (_objectToolbar) {
        _objectToolbar->setVisible(visible);
    }
}

void W3DViewMainWindow::toggleAnimationToolbar(bool visible)
{
    _showAnimationToolbar = visible;
    if (_animationToolbar) {
        _animationToolbar->setVisible(visible);
    }
}

void W3DViewMainWindow::setAmbientLight()
{
    if (!_viewport) {
        return;
    }

    ColorLightDialog dialog(
        "Ambient Light",
        _viewport->ambientLight(),
        [this](const Vector3 &color) {
            if (_viewport) {
                _viewport->setAmbientLight(color);
            }
        },
        this);
    dialog.exec();
}

void W3DViewMainWindow::setSceneLight()
{
    if (!_viewport) {
        return;
    }

    SceneLightDialog dialog(*_viewport, this);
    dialog.exec();
}

void W3DViewMainWindow::increaseAmbientLight()
{
    if (!_viewport) {
        return;
    }

    Vector3 color = _viewport->ambientLight();
    AdjustLightIntensity(color, 0.05f);
    _viewport->setAmbientLight(color);
}

void W3DViewMainWindow::decreaseAmbientLight()
{
    if (!_viewport) {
        return;
    }

    Vector3 color = _viewport->ambientLight();
    AdjustLightIntensity(color, -0.05f);
    _viewport->setAmbientLight(color);
}

void W3DViewMainWindow::increaseSceneLight()
{
    if (!_viewport) {
        return;
    }

    Vector3 diffuse = _viewport->sceneLightDiffuse();
    Vector3 specular = _viewport->sceneLightSpecular();
    AdjustLightIntensity(diffuse, 0.05f);
    AdjustLightIntensity(specular, 0.05f);
    _viewport->setSceneLightDiffuse(diffuse);
    _viewport->setSceneLightSpecular(specular);
}

void W3DViewMainWindow::decreaseSceneLight()
{
    if (!_viewport) {
        return;
    }

    Vector3 diffuse = _viewport->sceneLightDiffuse();
    Vector3 specular = _viewport->sceneLightSpecular();
    AdjustLightIntensity(diffuse, -0.05f);
    AdjustLightIntensity(specular, -0.05f);
    _viewport->setSceneLightDiffuse(diffuse);
    _viewport->setSceneLightSpecular(specular);
}

void W3DViewMainWindow::killSceneLight()
{
    if (!_viewport) {
        return;
    }

    _viewport->setSceneLightColor(Vector3(0.0f, 0.0f, 0.0f));
}

void W3DViewMainWindow::toggleLightRotateY(bool enabled)
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->lightRotationFlags();
    if (enabled) {
        flags |= W3DViewport::RotateY;
        flags &= ~W3DViewport::RotateYBack;
    } else {
        flags &= ~W3DViewport::RotateY;
    }
    _viewport->setLightRotationFlags(flags);
}

void W3DViewMainWindow::toggleLightRotateYBack()
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->lightRotationFlags();
    flags ^= W3DViewport::RotateYBack;
    flags &= ~W3DViewport::RotateY;
    _viewport->setLightRotationFlags(flags);
    if (_lightRotateYAction) {
        const QSignalBlocker blocker(_lightRotateYAction);
        _lightRotateYAction->setChecked((flags & W3DViewport::RotateY) != 0);
    }
}

void W3DViewMainWindow::toggleLightRotateZ(bool enabled)
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->lightRotationFlags();
    if (enabled) {
        flags |= W3DViewport::RotateZ;
        flags &= ~W3DViewport::RotateZBack;
    } else {
        flags &= ~W3DViewport::RotateZ;
    }
    _viewport->setLightRotationFlags(flags);
}

void W3DViewMainWindow::toggleLightRotateZBack()
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->lightRotationFlags();
    flags ^= W3DViewport::RotateZBack;
    flags &= ~W3DViewport::RotateZ;
    _viewport->setLightRotationFlags(flags);
    if (_lightRotateZAction) {
        const QSignalBlocker blocker(_lightRotateZAction);
        _lightRotateZAction->setChecked((flags & W3DViewport::RotateZ) != 0);
    }
}

void W3DViewMainWindow::toggleExposePrelit(bool enabled)
{
    WW3D::Expose_Prelit(enabled);
}

void W3DViewMainWindow::setPrelitVertex()
{
    if (WW3D::Get_Prelit_Mode() == WW3D::PRELIT_MODE_VERTEX) {
        return;
    }

    WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_VERTEX);
    reloadLightmapModels();
    reloadDisplayedObject();
}

void W3DViewMainWindow::setPrelitMultipass()
{
    if (WW3D::Get_Prelit_Mode() == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS) {
        return;
    }

    WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
    reloadLightmapModels();
    reloadDisplayedObject();
}

void W3DViewMainWindow::setPrelitMultitex()
{
    if (WW3D::Get_Prelit_Mode() == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE) {
        return;
    }

    WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
    reloadLightmapModels();
    reloadDisplayedObject();
}

void W3DViewMainWindow::setBackgroundColor()
{
    if (!_viewport) {
        return;
    }

    ColorLightDialog dialog(
        "Background Color",
        _viewport->backgroundColor(),
        [this](const Vector3 &color) {
            if (_viewport) {
                _viewport->setBackgroundColor(color);
            }
        },
        this);
    dialog.exec();
}

void W3DViewMainWindow::setBackgroundBitmap()
{
    if (!_viewport) {
        return;
    }

    BackgroundBitmapDialog dialog(_viewport->backgroundBitmap(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString path = dialog.selectedPath();
    _viewport->setBackgroundBitmap(path);
    statusBar()->showMessage(path.isEmpty()
                                 ? QStringLiteral("Background bitmap cleared")
                                 : QString("Background bitmap: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::toggleFog(bool enabled)
{
    if (_viewport) {
        _viewport->setFogEnabled(enabled);
    }
}

void W3DViewMainWindow::setCameraFront()
{
    if (_viewport) {
        _viewport->setCameraPosition(W3DViewport::CameraPosition::Front);
    }
}

void W3DViewMainWindow::setCameraBack()
{
    if (_viewport) {
        _viewport->setCameraPosition(W3DViewport::CameraPosition::Back);
    }
}

void W3DViewMainWindow::setCameraLeft()
{
    if (_viewport) {
        _viewport->setCameraPosition(W3DViewport::CameraPosition::Left);
    }
}

void W3DViewMainWindow::setCameraRight()
{
    if (_viewport) {
        _viewport->setCameraPosition(W3DViewport::CameraPosition::Right);
    }
}

void W3DViewMainWindow::setCameraTop()
{
    if (_viewport) {
        _viewport->setCameraPosition(W3DViewport::CameraPosition::Top);
    }
}

void W3DViewMainWindow::setCameraBottom()
{
    if (_viewport) {
        _viewport->setCameraPosition(W3DViewport::CameraPosition::Bottom);
    }
}

void W3DViewMainWindow::resetCamera()
{
    if (_viewport) {
        _viewport->resetCamera();
    }
}

void W3DViewMainWindow::setCameraRotateX(bool enabled)
{
    if (!_viewport) {
        return;
    }

    if (enabled) {
        _viewport->setAllowedCameraRotation(W3DViewport::CameraRotation::OnlyX);
        if (_cameraRotateYAction) {
            _cameraRotateYAction->setChecked(false);
        }
        if (_cameraRotateZAction) {
            _cameraRotateZAction->setChecked(false);
        }
    } else if (_viewport->allowedCameraRotation() == W3DViewport::CameraRotation::OnlyX) {
        _viewport->setAllowedCameraRotation(W3DViewport::CameraRotation::Free);
    }
}

void W3DViewMainWindow::setCameraRotateY(bool enabled)
{
    if (!_viewport) {
        return;
    }

    if (enabled) {
        _viewport->setAllowedCameraRotation(W3DViewport::CameraRotation::OnlyY);
        if (_cameraRotateXAction) {
            _cameraRotateXAction->setChecked(false);
        }
        if (_cameraRotateZAction) {
            _cameraRotateZAction->setChecked(false);
        }
    } else if (_viewport->allowedCameraRotation() == W3DViewport::CameraRotation::OnlyY) {
        _viewport->setAllowedCameraRotation(W3DViewport::CameraRotation::Free);
    }
}

void W3DViewMainWindow::setCameraRotateZ(bool enabled)
{
    if (!_viewport) {
        return;
    }

    if (enabled) {
        _viewport->setAllowedCameraRotation(W3DViewport::CameraRotation::OnlyZ);
        if (_cameraRotateXAction) {
            _cameraRotateXAction->setChecked(false);
        }
        if (_cameraRotateYAction) {
            _cameraRotateYAction->setChecked(false);
        }
    } else if (_viewport->allowedCameraRotation() == W3DViewport::CameraRotation::OnlyZ) {
        _viewport->setAllowedCameraRotation(W3DViewport::CameraRotation::Free);
    }
}

void W3DViewMainWindow::toggleCameraAnimate(bool enabled)
{
    _animateCamera = enabled;
    if (_viewport) {
        _viewport->setCameraAnimationEnabled(enabled);
        if (!enabled) {
            _viewport->resetCamera();
        }
    }

    QSettings settings;
    settings.setValue("Config/AnimateCamera", enabled);
}

void W3DViewMainWindow::toggleCameraResetOnDisplay(bool enabled)
{
    _autoResetCamera = enabled;
    if (_viewport) {
        _viewport->setAutoResetEnabled(enabled);
    }

    QSettings settings;
    settings.setValue("Config/ResetCamera", enabled);
}

void W3DViewMainWindow::toggleCameraBonePosX(bool enabled)
{
    if (_viewport) {
        _viewport->setCameraBonePosX(enabled);
    }
}

void W3DViewMainWindow::openCameraSettings()
{
    if (!_viewport) {
        return;
    }

    CameraSettingsDialog dialog(_viewport, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const bool manual_fov = dialog.isManualFovEnabled();
    const bool manual_clip = dialog.isManualClipPlanesEnabled();

    if (manual_fov) {
        _viewport->setManualFovEnabled(true);
        _viewport->setCameraFovDegrees(dialog.hfovDegrees(), dialog.vfovDegrees());
    } else {
        _viewport->setManualFovEnabled(false);
        _viewport->resetFov();
    }

    _viewport->setManualClipPlanesEnabled(manual_clip);
    _viewport->setCameraClipPlanes(dialog.nearClip(), dialog.farClip());

    double hfov_deg = 0.0;
    double vfov_deg = 0.0;
    _viewport->cameraFovDegrees(hfov_deg, vfov_deg);
    float znear = 0.0f;
    float zfar = 0.0f;
    _viewport->cameraClipPlanes(znear, zfar);

    QSettings settings;
    settings.setValue("Config/UseManualFOV", manual_fov);
    settings.setValue("Config/UseManualClipPlanes", manual_clip);
    settings.setValue("Config/hfov", hfov_deg * kDegToRad);
    settings.setValue("Config/vfov", vfov_deg * kDegToRad);
    settings.setValue("Config/znear", znear);
    settings.setValue("Config/zfar", zfar);
    _viewport->resetCamera();
}

void W3DViewMainWindow::openCameraDistance()
{
    if (!_viewport) {
        return;
    }

    CameraDistanceDialog dialog(_viewport->cameraDistance(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    _viewport->setCameraDistance(dialog.distance());
}

void W3DViewMainWindow::copyScreenSize()
{
    if (!_viewport) {
        return;
    }

    const float size = _viewport->currentScreenSize();
    if (size <= 0.0f) {
        statusBar()->showMessage("No render object to measure.");
        return;
    }

    const QString text = QString("MaxScreenSize=%1").arg(size, 0, 'f', 6);
    if (auto *clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(text);
        statusBar()->showMessage("Copied screen size to clipboard.");
    }
}

void W3DViewMainWindow::changeResolution()
{
    if (!_viewport) {
        return;
    }

    QSettings settings;
    const ResolutionDialog::Mode preferred_mode(
        settings.value("Config/DeviceWidth", 0).toInt(),
        settings.value("Config/DeviceHeight", 0).toInt(),
        settings.value("Config/DeviceBitsPerPix", 32).toInt());
    ResolutionDialog dialog(preferred_mode, isFullScreen(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const int width = dialog.selectedWidth();
    const int height = dialog.selectedHeight();
    const int bits_per_pixel = dialog.selectedBitsPerPixel();
    if (width <= 0 || height <= 0 || bits_per_pixel <= 0) {
        QMessageBox::warning(this, "Resolution", "Select a valid display resolution.");
        return;
    }

    const bool fullscreen = dialog.fullscreen();
    const bool was_fullscreen = isFullScreen();

    // Leaving borderless mode exposes the restored viewport size first, while
    // the viewport still retains its previous fullscreen render preference.
    // Entering does the inverse so a failed device reset cannot strand the Qt
    // window in a state the renderer did not accept.
    if (was_fullscreen && !fullscreen) {
        showNormal();
    }

    if (!_viewport->applyResolution(width, height, bits_per_pixel, fullscreen)) {
        if (was_fullscreen && !fullscreen) {
            showFullScreen();
        }
        QMessageBox::warning(this, "Resolution", "The selected display mode could not be applied.");
        return;
    }

    if (!was_fullscreen && fullscreen) {
        showFullScreen();
    }

    int applied_width = 0;
    int applied_height = 0;
    int applied_bits_per_pixel = 0;
    bool applied_windowed = true;
    WW3D::Get_Device_Resolution(
        applied_width, applied_height, applied_bits_per_pixel, applied_windowed);

    // Keep the selected mode as the next borderless preference. In windowed
    // mode the active render surface follows the widget and is intentionally
    // allowed to differ from this stored preference.
    settings.setValue("Config/DeviceWidth", width);
    settings.setValue("Config/DeviceHeight", height);
    settings.setValue("Config/DeviceBitsPerPix", bits_per_pixel);
    settings.setValue("Config/Windowed", fullscreen ? 0 : 1);
    statusBar()->showMessage(QString("Display mode: %1 x %2, %3 bpp%4")
                                 .arg(applied_width)
                                 .arg(applied_height)
                                 .arg(applied_bits_per_pixel)
                                 .arg(fullscreen ? " borderless fullscreen" : " windowed"));
}

void W3DViewMainWindow::openGammaDialog()
{
    if (_enableGammaAction && !_enableGammaAction->isChecked()) {
        QMessageBox::warning(this, "Gamma", "Gamma is disabled.\nEnable it in the File menu.");
        return;
    }

    GammaDialog dialog(this);
    dialog.exec();
}

void W3DViewMainWindow::toggleGammaCorrection(bool enabled)
{
    QSettings settings;
    settings.setValue("Config/EnableGamma", enabled ? 1 : 0);

    if (enabled) {
        int gamma = settings.value("Config/Gamma", 10).toInt();
        if (gamma < 10) {
            gamma = 10;
        }
        if (gamma > 30) {
            gamma = 30;
        }
        DX8Wrapper::Set_Gamma(gamma / 10.0f, 0.0f, 1.0f);
    } else {
        DX8Wrapper::Set_Gamma(1.0f, 0.0f, 1.0f);
    }
}

void W3DViewMainWindow::toggleMungeSortOnLoad(bool enabled)
{
    WW3D::Enable_Munge_Sort_On_Load(enabled);
    QSettings settings;
    settings.setValue("Config/MungeSortOnLoad", enabled ? 1 : 0);
}

void W3DViewMainWindow::openBackgroundObjectDialog()
{
    if (!_viewport) {
        return;
    }

    BackgroundObjectDialog dialog(_viewport->backgroundObjectName(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString name = dialog.selectedName();
    _viewport->setBackgroundObjectName(name);
    if (name.isEmpty()) {
        statusBar()->showMessage("Background object cleared.");
    } else {
        statusBar()->showMessage(QString("Background object: %1").arg(name));
    }
}

void W3DViewMainWindow::captureScreenshot()
{
    if (!_viewport) {
        return;
    }

    const QString base = QDir(QCoreApplication::applicationDirPath()).filePath("ScreenShot");
    const int screenshot_number = _viewport->captureScreenshot(base);
    if (screenshot_number <= 0) {
        statusBar()->showMessage("Screen capture failed.");
        return;
    }

    const QString filename = QString("%1%2.tga")
                                 .arg(base)
                                 .arg(screenshot_number, 2, 10, QLatin1Char('0'));
    statusBar()->showMessage(QString("Saved screenshot: %1").arg(filename));
}

void W3DViewMainWindow::makeMovie()
{
    if (!_viewport || !_treeView) {
        return;
    }

    const QModelIndex current = _treeView->currentIndex();
    if (!current.isValid() ||
        current.data(kRoleType).toInt() != static_cast<int>(AssetNodeType::Animation)) {
        QMessageBox::information(this, "Make Movie", "Select an animation to capture.");
        return;
    }

    if (!_viewport->hasAnimation()) {
        QMessageBox::information(this, "Make Movie", "No animation is available for capture.");
        return;
    }

    const QString previous_directory = QDir::currentPath();
    if (!QDir::setCurrent(QCoreApplication::applicationDirPath())) {
        QMessageBox::warning(this,
                             "Make Movie",
                             "Unable to use the application directory for movie capture.");
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::BlankCursor);
    QString error;
    const bool ok = _viewport->captureMovie(QStringLiteral("Grab"), 30.0f, &error);
    QGuiApplication::restoreOverrideCursor();

    const bool directory_restored = QDir::setCurrent(previous_directory);
    if (!directory_restored) {
        if (!error.isEmpty()) {
            error += '\n';
        }
        error += "The previous working directory could not be restored.";
    }

    if (!ok || !directory_restored) {
        const QString message = error.isEmpty() ? "Movie capture failed." : error;
        QMessageBox::warning(this, "Make Movie", message);
        return;
    }

    statusBar()->showMessage("Movie capture complete.");
}

void W3DViewMainWindow::selectPrevAsset()
{
    if (!_treeView) {
        return;
    }

    const QModelIndex current = _treeView->currentIndex();
    if (!current.isValid()) {
        return;
    }

    const QModelIndex prev = _treeModel->index(current.row() - 1, current.column(), current.parent());
    if (prev.isValid()) {
        _treeView->setCurrentIndex(prev);
    }
}

void W3DViewMainWindow::selectNextAsset()
{
    if (!_treeView) {
        return;
    }

    const QModelIndex current = _treeView->currentIndex();
    if (!current.isValid()) {
        return;
    }

    const QModelIndex next = _treeModel->index(current.row() + 1, current.column(), current.parent());
    if (next.isValid()) {
        _treeView->setCurrentIndex(next);
    }
}

void W3DViewMainWindow::showTreeContextMenu(const QPoint &pos)
{
    if (!_treeView || !_viewport) {
        return;
    }

    const QModelIndex index = _treeView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const int type_value = index.data(kRoleType).toInt();
    const bool is_group = type_value == static_cast<int>(AssetNodeType::Group);
    if (!is_group) {
        _treeView->setCurrentIndex(index);
    }
    auto matches_group = [](const QString &text, const QString &label) {
        return text == label || text.startsWith(label + " (");
    };

    if (type_value == static_cast<int>(AssetNodeType::Animation)) {
        QMenu menu(this);
        auto *play_action = menu.addAction("Play");
        connect(play_action, &QAction::triggered, this, &W3DViewMainWindow::startAnimation);
        auto *pause_action = menu.addAction("Pause");
        connect(pause_action, &QAction::triggered, this, &W3DViewMainWindow::pauseAnimation);
        auto *stop_action = menu.addAction("Stop");
        connect(stop_action, &QAction::triggered, this, &W3DViewMainWindow::stopAnimation);
        menu.addSeparator();
        auto *step_back_action = menu.addAction("Step Back");
        connect(step_back_action, &QAction::triggered, this, &W3DViewMainWindow::stepAnimationBackward);
        auto *step_forward_action = menu.addAction("Step Forward");
        connect(step_forward_action, &QAction::triggered, this, &W3DViewMainWindow::stepAnimationForward);
        menu.addSeparator();
        auto *settings_action = menu.addAction("Settings");
        connect(settings_action, &QAction::triggered, this, &W3DViewMainWindow::openAnimationSettings);
        menu.addSeparator();
        auto *advanced_action = menu.addAction("Advanced...");
        connect(advanced_action, &QAction::triggered, this, &W3DViewMainWindow::openAdvancedAnimation);

        const bool has_anim = _viewport->hasAnimation();
        play_action->setEnabled(has_anim);
        pause_action->setEnabled(has_anim);
        stop_action->setEnabled(has_anim);
        step_back_action->setEnabled(has_anim);
        step_forward_action->setEnabled(has_anim);

        menu.exec(_treeView->viewport()->mapToGlobal(pos));
        return;
    }

    if (type_value != static_cast<int>(AssetNodeType::RenderObject) && !is_group) {
        return;
    }

    const QString group_label = ResolveGroupLabel(_treeModel, index, is_group);

    if (matches_group(group_label, "H-LOD")) {
        QMenu menu(this);
        auto *record_action = menu.addAction("Record Screen Area");
        connect(record_action, &QAction::triggered, this, &W3DViewMainWindow::recordLodScreenArea);

        auto *include_null_action = menu.addAction("Include NULL Object");
        include_null_action->setCheckable(true);
        include_null_action->setChecked(_viewport->isNullLodIncluded());
        connect(include_null_action, &QAction::triggered, this, &W3DViewMainWindow::toggleLodIncludeNull);

        menu.addSeparator();

        auto *prev_action = menu.addAction("Prev Level");
        connect(prev_action, &QAction::triggered, this, &W3DViewMainWindow::selectPrevLod);
        auto *next_action = menu.addAction("Next Level");
        connect(next_action, &QAction::triggered, this, &W3DViewMainWindow::selectNextLod);

        int level = 0;
        int count = 0;
        if (_viewport->currentLodInfo(level, count)) {
            prev_action->setEnabled(level > 0);
            next_action->setEnabled(level + 1 < count);
        }

        menu.addSeparator();

        auto *auto_switch_action = menu.addAction("Auto Switching");
        auto_switch_action->setCheckable(true);
        auto_switch_action->setChecked(_viewport->isLodAutoSwitchingEnabled());
        connect(auto_switch_action, &QAction::triggered, this, &W3DViewMainWindow::toggleLodAutoSwitch);

        menu.addSeparator();
        auto *make_aggregate_action = menu.addAction("Make Aggregate...");
        connect(make_aggregate_action, &QAction::triggered, this, &W3DViewMainWindow::makeAggregate);

        menu.exec(_treeView->viewport()->mapToGlobal(pos));
        return;
    }

    if (matches_group(group_label, "Hierarchy")) {
        QMenu menu(this);
        auto *generate_lod_action = menu.addAction("Generate LOD...");
        connect(generate_lod_action, &QAction::triggered, this, &W3DViewMainWindow::generateLod);
        auto *make_aggregate_action = menu.addAction("Make Aggregate...");
        connect(make_aggregate_action, &QAction::triggered, this, &W3DViewMainWindow::makeAggregate);
        menu.exec(_treeView->viewport()->mapToGlobal(pos));
        return;
    }

    if (matches_group(group_label, "Aggregate")) {
        QMenu menu(this);
        auto *rename_action = menu.addAction("Rename Aggregate...");
        connect(rename_action, &QAction::triggered, this, &W3DViewMainWindow::renameAggregate);
        menu.addSeparator();
        auto *bone_action = menu.addAction("Bone Management...");
        connect(bone_action, &QAction::triggered, this, &W3DViewMainWindow::openBoneManagement);
        auto *auto_assign_action = menu.addAction("Auto Assign Bone Models");
        connect(auto_assign_action, &QAction::triggered, this, &W3DViewMainWindow::autoAssignBoneModels);
        menu.addSeparator();
        auto *bind_action = menu.addAction("Bind Subobject LOD");
        bind_action->setCheckable(true);
        bind_action->setChecked(_viewport->isSubobjectLodBound());
        connect(bind_action, &QAction::triggered, this, &W3DViewMainWindow::bindSubobjectLod);
        auto *generate_lod_action = menu.addAction("Generate LOD...");
        connect(generate_lod_action, &QAction::triggered, this, &W3DViewMainWindow::generateLod);
        menu.exec(_treeView->viewport()->mapToGlobal(pos));
        return;
    }
}

void W3DViewMainWindow::startAnimation()
{
    if (_viewport) {
        _viewport->setAnimationState(W3DViewport::AnimationState::Playing);
        refreshAnimationMenu();
        playAnimationSound();
        statusBar()->showMessage("Animation playing.");
    }
}

void W3DViewMainWindow::pauseAnimation()
{
    if (!_viewport) {
        return;
    }

    const auto state = _viewport->animationState();
    if (state == W3DViewport::AnimationState::Playing) {
        _viewport->setAnimationState(W3DViewport::AnimationState::Paused);
        statusBar()->showMessage("Animation paused.");
    } else if (state == W3DViewport::AnimationState::Paused) {
        _viewport->setAnimationState(W3DViewport::AnimationState::Playing);
        playAnimationSound();
        statusBar()->showMessage("Animation resumed.");
    }
    refreshAnimationMenu();
}

void W3DViewMainWindow::stopAnimation()
{
    if (_viewport) {
        _viewport->setAnimationState(W3DViewport::AnimationState::Stopped);
        refreshAnimationMenu();
        statusBar()->showMessage("Animation stopped.");
    }
}

void W3DViewMainWindow::stepAnimationForward()
{
    if (!_viewport) {
        return;
    }

    if (!_viewport->stepAnimation(1)) {
        statusBar()->showMessage("No animation to step.");
    }
}

void W3DViewMainWindow::stepAnimationBackward()
{
    if (!_viewport) {
        return;
    }

    if (!_viewport->stepAnimation(-1)) {
        statusBar()->showMessage("No animation to step.");
    }
}

void W3DViewMainWindow::openAnimationSettings()
{
    if (!_viewport) {
        return;
    }

    AnimationSettingsDialog dialog(*_viewport, this);
    dialog.exec();
}

void W3DViewMainWindow::openAdvancedAnimation()
{
    if (!_viewport || !_treeView) {
        return;
    }

    QString name;
    if (!GetSelectedRenderObjectName(_treeView, name)) {
        QMessageBox::information(this,
                                 "Advanced Animation",
                                 "Select a render object or animation before opening advanced controls.");
        return;
    }

    AdvancedAnimationDialog dialog(_viewport, name, this);
    if (dialog.exec() == QDialog::Accepted) {
        playAnimationSound();
        statusBar()->showMessage("Applied advanced animation mix.");
    }
}

void W3DViewMainWindow::generateLod()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Generate LOD", "Select a hierarchy to generate an LOD.");
        return;
    }
    Q_UNUSED(class_id);

    LodNamingType type = LodNamingType::Commando;
    if (!IsLodNameValid(name, type)) {
        QMessageBox::information(this,
                                 "Generate LOD",
                                 "Selected hierarchy name does not match LOD naming conventions.");
        return;
    }

    QString base_name = name;
    if (type == LodNamingType::Commando) {
        base_name.chop(2);
    } else {
        base_name.chop(1);
    }

    HLodPrototypeClass *prototype = GenerateLodPrototype(base_name, type);
    if (!prototype) {
        QMessageBox::warning(this, "Generate LOD", "Failed to generate LOD.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        delete prototype;
        QMessageBox::warning(this, "Generate LOD", "WW3D asset manager is not available.");
        return;
    }

    asset_manager->Add_Prototype(prototype);
    rebuildAssetTree();
    statusBar()->showMessage(QString("Generated LOD: %1").arg(prototype->Get_Name()));
}

void W3DViewMainWindow::makeAggregate()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Make Aggregate", "Select a hierarchy to make an aggregate.");
        return;
    }
    Q_UNUSED(class_id);

    AggregateNameDialog dialog("Make Aggregate", QString(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString aggregate_name = dialog.name();
    if (aggregate_name.isEmpty()) {
        QMessageBox::information(this, "Make Aggregate", "Aggregate name is required.");
        return;
    }
    if (aggregate_name.compare(name, Qt::CaseInsensitive) == 0) {
        QMessageBox::warning(this,
                             "Make Aggregate",
                             "The aggregate name must differ from its base-model name; using the "
                             "same name would create a self-recursive prototype.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Make Aggregate", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Make Aggregate", "Failed to load hierarchy.");
        return;
    }

    auto *definition = new AggregateDefClass(*render_obj);
    const QByteArray aggregate_bytes = aggregate_name.toLatin1();
    definition->Set_Name(aggregate_bytes.constData());
    auto *prototype = new AggregatePrototypeClass(definition);

    asset_manager->Remove_Prototype(definition->Get_Name());
    asset_manager->Add_Prototype(prototype);

    render_obj->Release_Ref();
    rebuildAssetTree();
    statusBar()->showMessage(QString("Created aggregate: %1").arg(aggregate_name));
}

void W3DViewMainWindow::renameAggregate()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Rename Aggregate", "Select an aggregate to rename.");
        return;
    }
    Q_UNUSED(class_id);

    const RenderObjInfo info = InspectRenderObj(name.toLatin1().constData());
    if (!info.isAggregate) {
        QMessageBox::information(this, "Rename Aggregate", "Selected object is not an aggregate.");
        return;
    }

    AggregateNameDialog dialog("Rename Aggregate", name, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString new_name = dialog.name();
    if (new_name.isEmpty()) {
        QMessageBox::information(this, "Rename Aggregate", "Aggregate name is required.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Rename Aggregate", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray old_name_bytes = name.toLatin1();
    auto *aggregate_prototype = dynamic_cast<AggregatePrototypeClass *>(
        asset_manager->Find_Prototype(old_name_bytes.constData()));
    AggregateDefClass *aggregate_definition =
        aggregate_prototype ? aggregate_prototype->Get_Definition() : nullptr;
    const char *base_model_name =
        aggregate_definition ? aggregate_definition->Get_Base_Model_Name() : nullptr;
    if (base_model_name &&
        new_name.compare(QString::fromLatin1(base_model_name), Qt::CaseInsensitive) == 0) {
        QMessageBox::warning(this,
                             "Rename Aggregate",
                             "The aggregate name must differ from its base-model name; using the "
                             "same name would create a self-recursive prototype.");
        return;
    }

    if (!RenameAggregatePrototype(name.toLatin1().constData(), new_name.toLatin1().constData())) {
        QMessageBox::warning(this, "Rename Aggregate", "Failed to rename aggregate.");
        return;
    }

    if (_viewport) {
        const QByteArray new_bytes = new_name.toLatin1();
        RenderObjClass *render_obj = asset_manager->Create_Render_Obj(new_bytes.constData());
        if (render_obj) {
            _viewport->clearAnimation();
            _viewport->setRenderObject(render_obj);
            render_obj->Release_Ref();
        }
    }

    rebuildAssetTree();
    statusBar()->showMessage(QString("Renamed aggregate: %1").arg(new_name));
}

void W3DViewMainWindow::openBoneManagement()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Bone Management", "Select an aggregate to edit bones.");
        return;
    }
    Q_UNUSED(class_id);

    const RenderObjInfo info = InspectRenderObj(name.toLatin1().constData());
    if (!info.isAggregate) {
        QMessageBox::information(this, "Bone Management", "Selected object is not an aggregate.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Bone Management", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Bone Management", "Failed to load aggregate.");
        return;
    }

    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(render_obj);
    }

    BoneManagementDialog dialog(render_obj, _viewport, this);
    const int result = dialog.exec();
    render_obj->Release_Ref();

    if (result == QDialog::Accepted) {
        statusBar()->showMessage("Updated aggregate bones.");
    }
}

void W3DViewMainWindow::autoAssignBoneModels()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Auto Assign Bones", "Select an aggregate to assign bones.");
        return;
    }
    Q_UNUSED(class_id);

    const RenderObjInfo info = InspectRenderObj(name.toLatin1().constData());
    if (!info.isAggregate) {
        QMessageBox::information(this, "Auto Assign Bones", "Selected object is not an aggregate.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Auto Assign Bones", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Auto Assign Bones", "Failed to load aggregate.");
        return;
    }

    bool updated = false;
    const int bone_count = render_obj->Get_Num_Bones();
    for (int index = 0; index < bone_count; ++index) {
        const char *bone_name = render_obj->Get_Bone_Name(index);
        if (!bone_name || !bone_name[0]) {
            continue;
        }

        if (!asset_manager->Render_Obj_Exists(bone_name)) {
            continue;
        }

        RenderObjClass *bone_obj = asset_manager->Create_Render_Obj(bone_name);
        if (!bone_obj) {
            continue;
        }

        render_obj->Add_Sub_Object_To_Bone(bone_obj, index);
        bone_obj->Release_Ref();
        updated = true;
    }

    if (updated) {
        UpdateAggregatePrototype(*render_obj);
    }

    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(render_obj);
    }

    render_obj->Release_Ref();
    statusBar()->showMessage(updated ? "Auto assigned bone models." : "No matching bone models found.");
}

void W3DViewMainWindow::bindSubobjectLod()
{
    if (!_viewport) {
        return;
    }

    const bool enabled = _viewport->toggleSubobjectLod();
    statusBar()->showMessage(enabled ? "Subobject LOD binding enabled." : "Subobject LOD binding disabled.");
}

void W3DViewMainWindow::createEmitter()
{
    ParticleEmitterDefClass definition = CreateDefaultEmitterDefinition();
    EmitterEditDialog dialog(definition, this);
    dialog.setApplyHandler(
        [this](const ParticleEmitterDefClass &updated, const QString &registeredName) {
            return commitEmitterDefinition(updated, registeredName, false, false);
        },
        QString(),
        true);
    dialog.exec();
}

void W3DViewMainWindow::scaleEmitter()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Scale Emitter", "Select an emitter to scale.");
        return;
    }
    if (class_id != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        QMessageBox::information(this, "Scale Emitter", "Selected object is not an emitter.");
        return;
    }

    ScaleDialog dialog(1.0, "Enter the scaling factor you want to apply to the current particle emitter.", this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Scale Emitter", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Scale Emitter", "Failed to load emitter.");
        return;
    }

    if (render_obj->Class_ID() != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        render_obj->Release_Ref();
        QMessageBox::warning(this, "Scale Emitter", "Selected object is not an emitter.");
        return;
    }

    auto *emitter = static_cast<ParticleEmitterClass *>(render_obj);
    emitter->Scale(static_cast<float>(dialog.scale()));

    ParticleEmitterDefClass *definition = emitter->Build_Definition();
    if (!definition) {
        emitter->Release_Ref();
        QMessageBox::warning(this, "Scale Emitter", "Failed to update emitter definition.");
        return;
    }

    UpdateEmitterPrototype(*definition, name);
    delete definition;

    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(emitter);
    }
    emitter->Release_Ref();
    statusBar()->showMessage("Scaled emitter.");
}

void W3DViewMainWindow::editEmitter()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Edit Emitter", "Select an emitter to edit.");
        return;
    }
    if (class_id != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        QMessageBox::information(this, "Edit Emitter", "Selected object is not an emitter.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Edit Emitter", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Edit Emitter", "Failed to load emitter.");
        return;
    }

    if (render_obj->Class_ID() != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        render_obj->Release_Ref();
        QMessageBox::warning(this, "Edit Emitter", "Selected object is not an emitter.");
        return;
    }

    auto *emitter = static_cast<ParticleEmitterClass *>(render_obj);
    ParticleEmitterDefClass *definition = emitter->Build_Definition();
    emitter->Release_Ref();
    if (!definition) {
        QMessageBox::warning(this, "Edit Emitter", "Failed to load emitter definition.");
        return;
    }

    EmitterEditDialog dialog(*definition, this);
    delete definition;
    dialog.setApplyHandler(
        [this](const ParticleEmitterDefClass &updated, const QString &registeredName) {
            return commitEmitterDefinition(updated, registeredName, false, false);
        },
        dialog.originalName());
    dialog.exec();
}

void W3DViewMainWindow::createSphere()
{
    if (!_viewport) {
        return;
    }

    SphereEditDialog dialog(nullptr, this);
    SphereRenderObjClass *preview = dialog.sphere();
    if (!preview) {
        QMessageBox::warning(this, "Create Sphere", "Failed to create sphere.");
        return;
    }
    _viewport->clearAnimation();
    _viewport->setRenderObject(preview);
    preview->Release_Ref();

    dialog.setApplyHandler(
        [this](SphereRenderObjClass &sphere, const QString &registered_name) {
            QString error_message;
            if (!UpdateSpherePrototype(sphere, registered_name, &error_message)) {
                QMessageBox::warning(this,
                                     "Apply Sphere",
                                     error_message.isEmpty()
                                         ? "Failed to register sphere prototype."
                                         : error_message);
                return false;
            }

            const QString name = QString::fromLatin1(sphere.Get_Name());
            const bool created = registered_name.isEmpty();
            rebuildAssetTree();
            if (_treeModel && _treeView) {
                const QModelIndex index = FindRenderObjectIndex(
                    _treeModel, name, RenderObjClass::CLASSID_SPHERE);
                if (index.isValid()) {
                    ExpandParentChain(_treeView, index.parent());
                    _treeView->setCurrentIndex(index);
                    _treeView->scrollTo(index);
                }
            }
            if (_viewport) {
                _viewport->clearAnimation();
                _viewport->setRenderObject(&sphere);
            }
            statusBar()->showMessage(created ? QString("Created sphere: %1").arg(name)
                                             : QString("Updated sphere: %1").arg(name));
            return true;
        },
        QString(),
        true);

    if (dialog.exec() != QDialog::Accepted) {
        onCurrentChanged(_treeView ? _treeView->currentIndex() : QModelIndex(), QModelIndex());
    }
}

void W3DViewMainWindow::createRing()
{
    if (!_viewport) {
        return;
    }

    RingEditDialog dialog(nullptr, this);
    RingRenderObjClass *preview = dialog.ring();
    if (!preview) {
        QMessageBox::warning(this, "Create Ring", "Failed to create ring.");
        return;
    }
    _viewport->clearAnimation();
    _viewport->setRenderObject(preview);
    preview->Release_Ref();

    dialog.setApplyHandler(
        [this](RingRenderObjClass &ring, const QString &registered_name) {
            QString error_message;
            if (!UpdateRingPrototype(ring, registered_name, &error_message)) {
                QMessageBox::warning(this,
                                     "Apply Ring",
                                     error_message.isEmpty()
                                         ? "Failed to register ring prototype."
                                         : error_message);
                return false;
            }

            const QString name = QString::fromLatin1(ring.Get_Name());
            const bool created = registered_name.isEmpty();
            rebuildAssetTree();
            if (_treeModel && _treeView) {
                const QModelIndex index = FindRenderObjectIndex(
                    _treeModel, name, RenderObjClass::CLASSID_RING);
                if (index.isValid()) {
                    ExpandParentChain(_treeView, index.parent());
                    _treeView->setCurrentIndex(index);
                    _treeView->scrollTo(index);
                }
            }
            if (_viewport) {
                _viewport->clearAnimation();
                _viewport->setRenderObject(&ring);
            }
            statusBar()->showMessage(created ? QString("Created ring: %1").arg(name)
                                             : QString("Updated ring: %1").arg(name));
            return true;
        },
        QString(),
        true);

    if (dialog.exec() != QDialog::Accepted) {
        onCurrentChanged(_treeView ? _treeView->currentIndex() : QModelIndex(), QModelIndex());
    }
}

void W3DViewMainWindow::editPrimitive()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Edit Primitive", "Select a primitive to edit.");
        return;
    }
    if (class_id != RenderObjClass::CLASSID_SPHERE &&
        class_id != RenderObjClass::CLASSID_RING) {
        QMessageBox::information(this, "Edit Primitive", "Selected object is not a primitive.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Edit Primitive", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Edit Primitive", "Failed to load primitive.");
        return;
    }

    if (class_id == RenderObjClass::CLASSID_SPHERE) {
        if (render_obj->Class_ID() != RenderObjClass::CLASSID_SPHERE) {
            render_obj->Release_Ref();
            QMessageBox::warning(this, "Edit Primitive", "Selected object is not a sphere.");
            return;
        }

        auto *sphere = static_cast<SphereRenderObjClass *>(render_obj);
        SphereEditDialog dialog(sphere, this);
        sphere->Release_Ref();
        SphereRenderObjClass *preview = dialog.sphere();
        if (!preview) {
            QMessageBox::warning(this, "Edit Primitive", "Failed to update sphere.");
            return;
        }
        if (_viewport) {
            _viewport->clearAnimation();
            _viewport->setRenderObject(preview);
        }
        preview->Release_Ref();

        dialog.setApplyHandler(
            [this](SphereRenderObjClass &updated, const QString &registered_name) {
                QString error_message;
                if (!UpdateSpherePrototype(updated, registered_name, &error_message)) {
                    QMessageBox::warning(this,
                                         "Apply Sphere",
                                         error_message.isEmpty()
                                             ? "Failed to register sphere prototype."
                                             : error_message);
                    return false;
                }

                const QString updated_name = QString::fromLatin1(updated.Get_Name());
                rebuildAssetTree();
                if (_treeModel && _treeView) {
                    const QModelIndex index = FindRenderObjectIndex(
                        _treeModel, updated_name, RenderObjClass::CLASSID_SPHERE);
                    if (index.isValid()) {
                        ExpandParentChain(_treeView, index.parent());
                        _treeView->setCurrentIndex(index);
                        _treeView->scrollTo(index);
                    }
                }
                if (_viewport) {
                    _viewport->clearAnimation();
                    _viewport->setRenderObject(&updated);
                }
                statusBar()->showMessage(QString("Updated sphere: %1").arg(updated_name));
                return true;
            },
            dialog.oldName());

        if (dialog.exec() != QDialog::Accepted) {
            onCurrentChanged(_treeView ? _treeView->currentIndex() : QModelIndex(), QModelIndex());
        }
        return;
    }

    if (render_obj->Class_ID() != RenderObjClass::CLASSID_RING) {
        render_obj->Release_Ref();
        QMessageBox::warning(this, "Edit Primitive", "Selected object is not a ring.");
        return;
    }

    auto *ring = static_cast<RingRenderObjClass *>(render_obj);
    RingEditDialog dialog(ring, this);
    ring->Release_Ref();
    RingRenderObjClass *preview = dialog.ring();
    if (!preview) {
        QMessageBox::warning(this, "Edit Primitive", "Failed to update ring.");
        return;
    }
    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(preview);
    }
    preview->Release_Ref();

    dialog.setApplyHandler(
        [this](RingRenderObjClass &updated, const QString &registered_name) {
            QString error_message;
            if (!UpdateRingPrototype(updated, registered_name, &error_message)) {
                QMessageBox::warning(this,
                                     "Apply Ring",
                                     error_message.isEmpty()
                                         ? "Failed to register ring prototype."
                                         : error_message);
                return false;
            }

            const QString updated_name = QString::fromLatin1(updated.Get_Name());
            rebuildAssetTree();
            if (_treeModel && _treeView) {
                const QModelIndex index = FindRenderObjectIndex(
                    _treeModel, updated_name, RenderObjClass::CLASSID_RING);
                if (index.isValid()) {
                    ExpandParentChain(_treeView, index.parent());
                    _treeView->setCurrentIndex(index);
                    _treeView->scrollTo(index);
                }
            }
            if (_viewport) {
                _viewport->clearAnimation();
                _viewport->setRenderObject(&updated);
            }
            statusBar()->showMessage(QString("Updated ring: %1").arg(updated_name));
            return true;
        },
        dialog.oldName());

    if (dialog.exec() != QDialog::Accepted) {
        onCurrentChanged(_treeView ? _treeView->currentIndex() : QModelIndex(), QModelIndex());
    }
}

void W3DViewMainWindow::createSoundObject()
{
    SoundEditDialog dialog(nullptr, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    SoundRenderObjClass *sound_obj = dialog.sound();
    if (!sound_obj) {
        QMessageBox::warning(this, "Create Sound Object", "Failed to create sound object.");
        return;
    }

    QString error_message;
    if (!UpdateSoundPrototype(*sound_obj, dialog.oldName(), &error_message)) {
        sound_obj->Release_Ref();
        QMessageBox::warning(this,
                             "Create Sound Object",
                             error_message.isEmpty() ? "Failed to register sound object."
                                                     : error_message);
        return;
    }

    const char *created_name_ptr = sound_obj->Get_Name();
    const QString created_name = created_name_ptr ? QString::fromLatin1(created_name_ptr) : QString();
    rebuildAssetTree();
    if (_treeModel && _treeView) {
        const QModelIndex index = FindRenderObjectIndex(
            _treeModel, created_name, RenderObjClass::CLASSID_SOUND);
        if (index.isValid()) {
            ExpandParentChain(_treeView, index.parent());
            _treeView->setCurrentIndex(index);
            _treeView->scrollTo(index);
        }
    }
    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(sound_obj);
    }
    statusBar()->showMessage(QString("Created sound object: %1").arg(created_name));
    sound_obj->Release_Ref();
}

void W3DViewMainWindow::editSoundObject()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Edit Sound Object", "Select a sound object to edit.");
        return;
    }
    if (class_id != RenderObjClass::CLASSID_SOUND) {
        QMessageBox::information(this, "Edit Sound Object", "Selected object is not a sound object.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Edit Sound Object", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Edit Sound Object", "Failed to load sound object.");
        return;
    }

    if (render_obj->Class_ID() != RenderObjClass::CLASSID_SOUND) {
        render_obj->Release_Ref();
        QMessageBox::warning(this, "Edit Sound Object", "Selected object is not a sound object.");
        return;
    }

    auto *sound_obj = static_cast<SoundRenderObjClass *>(render_obj);
    SoundEditDialog dialog(sound_obj, this);
    const int result = dialog.exec();
    sound_obj->Release_Ref();

    if (result != QDialog::Accepted) {
        return;
    }

    SoundRenderObjClass *updated = dialog.sound();
    if (!updated) {
        QMessageBox::warning(this, "Edit Sound Object", "Failed to update sound object.");
        return;
    }

    QString error_message;
    if (!UpdateSoundPrototype(*updated, dialog.oldName(), &error_message)) {
        updated->Release_Ref();
        QMessageBox::warning(this,
                             "Edit Sound Object",
                             error_message.isEmpty() ? "Failed to register sound object."
                                                     : error_message);
        return;
    }

    const char *updated_name_ptr = updated->Get_Name();
    const QString updated_name = updated_name_ptr ? QString::fromLatin1(updated_name_ptr) : QString();
    rebuildAssetTree();
    if (_treeModel && _treeView) {
        const QModelIndex index = FindRenderObjectIndex(
            _treeModel, updated_name, RenderObjClass::CLASSID_SOUND);
        if (index.isValid()) {
            ExpandParentChain(_treeView, index.parent());
            _treeView->setCurrentIndex(index);
            _treeView->scrollTo(index);
        }
    }
    if (_viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(updated);
    }
    statusBar()->showMessage(QString("Updated sound object: %1").arg(updated_name));
    updated->Release_Ref();
}

void W3DViewMainWindow::openAnimatedSoundOptions()
{
    QSettings settings;
    const QString definition_path = settings.value("Config/SoundDefLibPath").toString();
    const QString ini_path = settings.value("Config/AnimSoundINIPath").toString();
    const QString data_path = settings.value("Config/AnimSoundDataPath").toString();

    AnimatedSoundOptionsDialog dialog(definition_path, ini_path, data_path, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto normalize = [](const QString &value) -> QString {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty()) {
            return QString();
        }
        return QDir::toNativeSeparators(QDir::cleanPath(trimmed));
    };

    const QString new_definition = normalize(dialog.definitionLibraryPath());
    const QString new_ini = normalize(dialog.iniPath());
    const QString new_data = normalize(dialog.dataPath());

    settings.setValue("Config/SoundDefLibPath", new_definition);
    settings.setValue("Config/AnimSoundINIPath", new_ini);
    settings.setValue("Config/AnimSoundDataPath", new_data);

    AnimatedSoundOptionsDialog::LoadAnimatedSoundSettings();
    statusBar()->showMessage("Animated sound options updated.");
}

void W3DViewMainWindow::importFacialAnims()
{
    if (!_treeView) {
        return;
    }

    const QString hierarchy = GetSelectedHierarchyName(_treeView);
    if (hierarchy.isEmpty()) {
        QMessageBox::information(this, "Import Facial Anims", "Select a hierarchy before importing.");
        return;
    }

    const QString start_dir = _lastOpenedPath.isEmpty() ? QDir::currentPath() : _lastOpenedPath;
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Import Facial Anims",
        start_dir,
        "Animation Description (*.txt)");
    if (files.isEmpty()) {
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    int imported = 0;
    for (const QString &path : files) {
        if (ImportFacialAnimation(hierarchy, path)) {
            ++imported;
        }
    }
    QGuiApplication::restoreOverrideCursor();

    if (imported > 0) {
        rebuildAssetTree();
        statusBar()->showMessage(QString("Imported %1 facial animation(s).").arg(imported));
    } else {
        QMessageBox::warning(this, "Import Facial Anims", "No facial animations were imported.");
    }
}

bool W3DViewMainWindow::confirmExportTarget(const QString &path)
{
    const QFileInfo target_info(path);
    if (!target_info.exists()) {
        return true;
    }

    const auto normalized_path = [](const QString &candidate) {
        const QFileInfo info(candidate);
        const QString canonical = info.canonicalFilePath();
        return QDir::cleanPath(canonical.isEmpty() ? info.absoluteFilePath() : canonical);
    };

    const QString target = normalized_path(path);
    const bool replaces_loaded_file = std::any_of(
        _loadedFiles.cbegin(), _loadedFiles.cend(), [&](const QString &loaded_file) {
            return normalized_path(loaded_file).compare(target, Qt::CaseInsensitive) == 0;
        });
    if (replaces_loaded_file) {
        const QMessageBox::StandardButton answer = QMessageBox::warning(
            this,
            "Export W3D",
            "The selected target is one of the currently loaded W3D files.\n\n"
            "An export contains only the selected definition and will replace every "
            "other chunk in that file.\n\nReplace the loaded file anyway?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        return answer == QMessageBox::Yes;
    }

    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        "Export W3D",
        QString("The export target already exists:\n\n%1\n\nReplace it?")
            .arg(QDir::toNativeSeparators(target_info.absoluteFilePath())),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    return answer == QMessageBox::Yes;
}

void W3DViewMainWindow::exportAggregate()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Export Aggregate", "Select an aggregate to export.");
        return;
    }

    const RenderObjInfo info = InspectRenderObj(name.toLatin1().constData());
    if (!info.isAggregate) {
        QMessageBox::information(this, "Export Aggregate", "Selected object is not an aggregate.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Export Aggregate", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    auto *proto = dynamic_cast<AggregatePrototypeClass *>(
        asset_manager->Find_Prototype(name_bytes.constData()));
    if (!proto) {
        QMessageBox::warning(this,
                             "Export Aggregate",
                             "The selected prototype is not an aggregate.");
        return;
    }

    AggregateDefClass *definition = proto->Get_Definition();
    if (!definition) {
        QMessageBox::warning(this, "Export Aggregate", "Aggregate definition not available.");
        return;
    }

    const QString path =
        SelectExportPath(this, "Export Aggregate", name + ".w3d", _lastOpenedPath);
    if (path.isEmpty()) {
        return;
    }
    if (!confirmExportTarget(path)) {
        return;
    }

    QString error_message;
    const bool ok = W3DExportUtils::SaveChunkFileAtomically(
        path,
        W3D_CHUNK_AGGREGATE,
        [definition](ChunkSaveClass &save_chunk) {
            return definition->Save_W3D(save_chunk) == WW3D_ERROR_OK;
        },
        &error_message);
    if (!ok) {
        QMessageBox::warning(this,
                             "Export Aggregate",
                             QString("Failed to export aggregate.\n\n%1").arg(error_message));
        return;
    }

    statusBar()->showMessage(QString("Exported aggregate: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::exportEmitter()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Export Emitter", "Select an emitter to export.");
        return;
    }

    if (class_id != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        QMessageBox::information(this, "Export Emitter", "Selected object is not an emitter.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Export Emitter", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    auto *proto = dynamic_cast<ParticleEmitterPrototypeClass *>(
        asset_manager->Find_Prototype(name_bytes.constData()));
    if (!proto) {
        QMessageBox::warning(this,
                             "Export Emitter",
                             "The selected prototype is not an emitter.");
        return;
    }

    ParticleEmitterDefClass *definition = proto->Get_Definition();
    if (!definition) {
        QMessageBox::warning(this, "Export Emitter", "Emitter definition not available.");
        return;
    }

    const QString path =
        SelectExportPath(this, "Export Emitter", name + ".w3d", _lastOpenedPath);
    if (path.isEmpty()) {
        return;
    }
    if (!confirmExportTarget(path)) {
        return;
    }

    QString error_message;
    const bool ok = W3DExportUtils::SaveChunkFileAtomically(
        path,
        W3D_CHUNK_EMITTER,
        [definition](ChunkSaveClass &save_chunk) {
            return definition->Save_W3D(save_chunk) == WW3D_ERROR_OK;
        },
        &error_message);
    if (!ok) {
        QMessageBox::warning(this,
                             "Export Emitter",
                             QString("Failed to export emitter.\n\n%1").arg(error_message));
        return;
    }

    statusBar()->showMessage(QString("Exported emitter: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::exportLod()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Export LOD", "Select an LOD to export.");
        return;
    }

    if (class_id != RenderObjClass::CLASSID_HLOD) {
        QMessageBox::information(this, "Export LOD", "Selected object is not an LOD.");
        return;
    }

    const RenderObjInfo info = InspectRenderObj(name.toLatin1().constData());
    if (!info.isRealLod || info.isAggregate) {
        QMessageBox::information(this,
                                 "Export LOD",
                                 "Selected object is not a multilevel LOD.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Export LOD", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    auto *proto = dynamic_cast<HLodPrototypeClass *>(
        asset_manager->Find_Prototype(name_bytes.constData()));
    if (!proto) {
        QMessageBox::warning(this,
                             "Export LOD",
                             "The selected prototype is not an HLOD.");
        return;
    }

    HLodDefClass *definition = proto->Get_Definition();
    if (!definition) {
        QMessageBox::warning(this, "Export LOD", "LOD definition not available.");
        return;
    }

    const QString path =
        SelectExportPath(this, "Export LOD", name + ".w3d", _lastOpenedPath);
    if (path.isEmpty()) {
        return;
    }
    if (!confirmExportTarget(path)) {
        return;
    }

    QString error_message;
    const bool ok = W3DExportUtils::SaveChunkFileAtomically(
        path,
        W3D_CHUNK_HLOD,
        [definition](ChunkSaveClass &save_chunk) {
            return definition->Save(save_chunk) == WW3D_ERROR_OK;
        },
        &error_message);
    if (!ok) {
        QMessageBox::warning(this,
                             "Export LOD",
                             QString("Failed to export LOD.\n\n%1").arg(error_message));
        return;
    }

    statusBar()->showMessage(QString("Exported LOD: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::exportPrimitive()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Export Primitive", "Select a primitive to export.");
        return;
    }

    if (class_id != RenderObjClass::CLASSID_SPHERE &&
        class_id != RenderObjClass::CLASSID_RING) {
        QMessageBox::information(this, "Export Primitive", "Selected object is not a primitive.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Export Primitive", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    SpherePrototypeClass *sphere_proto = nullptr;
    RingPrototypeClass *ring_proto = nullptr;
    if (class_id == RenderObjClass::CLASSID_SPHERE) {
        sphere_proto = dynamic_cast<SpherePrototypeClass *>(
            asset_manager->Find_Prototype(name_bytes.constData()));
        if (!sphere_proto) {
            QMessageBox::warning(this,
                                 "Export Primitive",
                                 "The selected prototype is not a sphere.");
            return;
        }
    } else {
        ring_proto = dynamic_cast<RingPrototypeClass *>(
            asset_manager->Find_Prototype(name_bytes.constData()));
        if (!ring_proto) {
            QMessageBox::warning(this,
                                 "Export Primitive",
                                 "The selected prototype is not a ring.");
            return;
        }
    }

    const QString title = class_id == RenderObjClass::CLASSID_SPHERE ? "Export Sphere" : "Export Ring";
    const QString path = SelectExportPath(this, title, name + ".w3d", _lastOpenedPath);
    if (path.isEmpty()) {
        return;
    }
    if (!confirmExportTarget(path)) {
        return;
    }

    QString error_message;
    bool ok = false;
    if (sphere_proto) {
        ok = W3DExportUtils::SaveChunkFileAtomically(
            path,
            W3D_CHUNK_SPHERE,
            [sphere_proto](ChunkSaveClass &save_chunk) {
                return sphere_proto->Save(save_chunk);
            },
            &error_message);
    } else {
        ok = W3DExportUtils::SaveChunkFileAtomically(
            path,
            W3D_CHUNK_RING,
            [ring_proto](ChunkSaveClass &save_chunk) {
                return ring_proto->Save(save_chunk);
            },
            &error_message);
    }

    if (!ok) {
        QMessageBox::warning(this,
                             "Export Primitive",
                             QString("Failed to export primitive.\n\n%1").arg(error_message));
        return;
    }

    statusBar()->showMessage(QString("Exported primitive: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::exportSoundObject()
{
    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Export Sound Object", "Select a sound object to export.");
        return;
    }

    if (class_id != RenderObjClass::CLASSID_SOUND) {
        QMessageBox::information(this, "Export Sound Object", "Selected object is not a sound object.");
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Export Sound Object", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    auto *proto = dynamic_cast<SoundRenderObjPrototypeClass *>(
        asset_manager->Find_Prototype(name_bytes.constData()));
    if (!proto) {
        QMessageBox::warning(this,
                             "Export Sound Object",
                             "The selected prototype is not a sound object.");
        return;
    }

    SoundRenderObjDefClass *definition = proto->Peek_Definition();
    if (!definition) {
        QMessageBox::warning(this,
                             "Export Sound Object",
                             "Sound object definition not available.");
        return;
    }

    const QString path =
        SelectExportPath(this, "Export Sound Object", name + ".w3d", _lastOpenedPath);
    if (path.isEmpty()) {
        return;
    }
    if (!confirmExportTarget(path)) {
        return;
    }

    QString error_message;
    const bool ok = W3DExportUtils::SaveChunkFileAtomically(
        path,
        W3D_CHUNK_SOUNDROBJ,
        [definition](ChunkSaveClass &save_chunk) {
            return definition->Save_W3D(save_chunk) == WW3D_ERROR_OK;
        },
        &error_message);
    if (!ok) {
        QMessageBox::warning(this,
                             "Export Sound Object",
                             QString("Failed to export sound object.\n\n%1").arg(error_message));
        return;
    }

    statusBar()->showMessage(QString("Exported sound object: %1").arg(QFileInfo(path).fileName()));
}

void W3DViewMainWindow::listMissingTextures()
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Missing Textures", "WW3D asset manager is not available.");
        return;
    }

    QStringList missing;
    HashTemplateIterator<StringClass, TextureClass *> iterator(asset_manager->Texture_Hash());
    for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
        auto *texture = iterator.Peek_Value();
        if (!texture || !texture->Is_Missing_Texture()) {
            continue;
        }

        const char *name = iterator.Peek_Key();
        if (name && name[0]) {
            missing.append(QString::fromLatin1(name));
        }
    }

    if (missing.isEmpty()) {
        QMessageBox::information(this, "Texture Info", "No Missing Textures!");
        return;
    }

    QString message("Warning! The following textures are missing:\n\n");
    message += missing.join('\n');
    QMessageBox::warning(this, "Missing Textures", message);
}

void W3DViewMainWindow::copyAssets()
{
    if (!_treeView) {
        return;
    }

    const QModelIndex current = _treeView->currentIndex();
    if (!current.isValid() ||
        current.data(kRoleType).toInt() != static_cast<int>(AssetNodeType::RenderObject)) {
        QMessageBox::information(this, "Copy Asset Files", "Select a render object to copy assets.");
        return;
    }

    if (_loadedFiles.isEmpty()) {
        QMessageBox::warning(this, "Copy Asset Files", "No source directory is available.");
        return;
    }

    const QString dest_dir = QFileDialog::getExistingDirectory(
        this,
        "Copy Asset Files",
        _lastOpenedPath);
    if (dest_dir.isEmpty()) {
        return;
    }

    const QString name = current.data(kRoleName).toString();
    if (name.isEmpty()) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Copy Asset Files", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *object = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!object) {
        QMessageBox::information(this,
                                 "Copy Asset Files",
                                 QString("Unable to create render object '%1'.").arg(name));
        return;
    }

    DynamicVectorClass<StringClass> dependencies;
    object->Build_Dependency_List(dependencies);
    object->Release_Ref();

    if (dependencies.Count() == 0) {
        QMessageBox::information(this, "Copy Asset Files", "No dependent assets were found.");
        return;
    }

    QString source_path;
    for (auto it = _loadedFiles.crbegin(); it != _loadedFiles.crend(); ++it) {
        const QFileInfo loaded_info(*it);
        if (loaded_info.completeBaseName().compare(name, Qt::CaseInsensitive) == 0) {
            source_path = loaded_info.absolutePath();
            break;
        }
    }
    if (source_path.isEmpty()) {
        QMessageBox::warning(this,
                             "Copy Asset Files",
                             QString("The source W3D for '%1' could not be identified.").arg(name));
        return;
    }

    const QDir src_root(source_path);
    const QDir dest_root(dest_dir);
    QStringList failures;
    for (int index = 0; index < dependencies.Count(); ++index) {
        const char *dep_name = dependencies[index].Peek_Buffer();
        if (!dep_name || !dep_name[0]) {
            continue;
        }

        const QString filename = QString::fromLatin1(dep_name);
        const QString src_path = src_root.filePath(filename);
        const QString dest_path = dest_root.filePath(filename);

        if (!QFile::exists(src_path)) {
            failures.append(src_path);
            continue;
        }

        const QFileInfo dest_info(dest_path);
        QDir dest_dir = dest_info.dir();
        if (!dest_dir.exists() && !dest_dir.mkpath(".")) {
            failures.append(src_path);
            continue;
        }

        if (QFile::exists(dest_path)) {
            QFile::remove(dest_path);
        }

        if (!QFile::copy(src_path, dest_path)) {
            failures.append(src_path);
        }
    }

    if (!failures.isEmpty()) {
        QString message("Unable to copy the following files:\n\n");
        message += failures.join('\n');
        QMessageBox::warning(this, "Copy Failure", message);
        return;
    }

    statusBar()->showMessage(QString("Copied assets to %1").arg(dest_dir));
}

void W3DViewMainWindow::addToLineup()
{
    if (!_viewport) {
        return;
    }

    AddToLineupDialog dialog(_viewport, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString name = dialog.selectedName();
    if (name.isEmpty()) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Add To Lineup", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    RenderObjClass *object = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!object) {
        QMessageBox::information(this,
                                 "Add To Lineup",
                                 QString("Unable to create render object '%1'.").arg(name));
        return;
    }

    SetHighestLod(object);
    const bool added = _viewport->addToLineup(object);
    object->Release_Ref();

    if (!added) {
        QMessageBox::information(this, "Add To Lineup", "Selected object cannot be added to the lineup.");
        return;
    }

    statusBar()->showMessage(QString("Added to lineup: %1").arg(name));
}

void W3DViewMainWindow::showAbout()
{
    QMessageBox::about(this,
                       "About W3DViewQt",
                       "W3DViewQt\nQt-based W3D asset viewer.");
}

void W3DViewMainWindow::recordLodScreenArea()
{
    if (!_viewport) {
        return;
    }

    if (_viewport->recordLodScreenArea()) {
        statusBar()->showMessage("Recorded LOD screen area.");
    } else {
        statusBar()->showMessage("LOD screen area is not available.");
    }
}

void W3DViewMainWindow::toggleLodIncludeNull(bool enabled)
{
    if (!_viewport) {
        return;
    }

    if (_viewport->setNullLodIncluded(enabled)) {
        statusBar()->showMessage(enabled ? "Included NULL LOD." : "Removed NULL LOD.");
    } else {
        statusBar()->showMessage("NULL LOD is not available.");
    }
}

void W3DViewMainWindow::selectPrevLod()
{
    if (!_viewport) {
        return;
    }

    if (!_viewport->adjustLodLevel(-1)) {
        statusBar()->showMessage("No previous LOD level.");
    }
}

void W3DViewMainWindow::selectNextLod()
{
    if (!_viewport) {
        return;
    }

    if (!_viewport->adjustLodLevel(1)) {
        statusBar()->showMessage("No next LOD level.");
    }
}

void W3DViewMainWindow::toggleLodAutoSwitch(bool enabled)
{
    if (_viewport) {
        _viewport->setLodAutoSwitchingEnabled(enabled);
        statusBar()->showMessage(enabled ? "LOD auto switching enabled." : "LOD auto switching disabled.");
    }
}

void W3DViewMainWindow::toggleObjectRotateX(bool enabled)
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->objectRotationFlags();
    if (enabled) {
        flags |= W3DViewport::RotateX;
        flags &= ~W3DViewport::RotateXBack;
    } else {
        flags &= ~W3DViewport::RotateX;
    }
    _viewport->setObjectRotationFlags(flags);
}

void W3DViewMainWindow::toggleObjectRotateY(bool enabled)
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->objectRotationFlags();
    if (enabled) {
        flags |= W3DViewport::RotateY;
        flags &= ~W3DViewport::RotateYBack;
    } else {
        flags &= ~W3DViewport::RotateY;
    }
    _viewport->setObjectRotationFlags(flags);
}

void W3DViewMainWindow::toggleObjectRotateYBack()
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->objectRotationFlags();
    flags ^= W3DViewport::RotateYBack;
    flags &= ~W3DViewport::RotateY;
    _viewport->setObjectRotationFlags(flags);
    if (_objectRotateYAction) {
        const QSignalBlocker blocker(_objectRotateYAction);
        _objectRotateYAction->setChecked((flags & W3DViewport::RotateY) != 0);
    }
}

void W3DViewMainWindow::toggleObjectRotateZ(bool enabled)
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->objectRotationFlags();
    if (enabled) {
        flags |= W3DViewport::RotateZ;
        flags &= ~W3DViewport::RotateZBack;
    } else {
        flags &= ~W3DViewport::RotateZ;
    }
    _viewport->setObjectRotationFlags(flags);
}

void W3DViewMainWindow::toggleObjectRotateZBack()
{
    if (!_viewport) {
        return;
    }

    int flags = _viewport->objectRotationFlags();
    flags ^= W3DViewport::RotateZBack;
    flags &= ~W3DViewport::RotateZ;
    _viewport->setObjectRotationFlags(flags);
    if (_objectRotateZAction) {
        const QSignalBlocker blocker(_objectRotateZAction);
        _objectRotateZAction->setChecked((flags & W3DViewport::RotateZ) != 0);
    }
}

void W3DViewMainWindow::resetObject()
{
    if (_viewport) {
        _viewport->resetObjectTransform();
    }
}

void W3DViewMainWindow::toggleAlternateMaterials()
{
    if (_viewport) {
        _viewport->toggleAlternateMaterials();
    }
}

void W3DViewMainWindow::showObjectProperties()
{
    if (!_treeView) {
        return;
    }

    const QModelIndex current = _treeView->currentIndex();
    if (!current.isValid()) {
        QMessageBox::information(this, "Properties", "Select an asset to view properties.");
        return;
    }

    const int type_value = current.data(kRoleType).toInt();
    if (type_value == static_cast<int>(AssetNodeType::Animation)) {
        const QString animation_name = current.data(kRoleName).toString();
        if (animation_name.isEmpty()) {
            QMessageBox::information(this, "Properties", "Select an animation to view properties.");
            return;
        }

        AnimationPropertiesDialog dialog(animation_name, this);
        dialog.exec();
        return;
    }

    if (type_value != static_cast<int>(AssetNodeType::RenderObject)) {
        QMessageBox::information(this, "Properties", "Select a render object to view properties.");
        return;
    }

    QString name;
    int class_id = 0;
    if (!GetSelectedRenderObject(_treeView, name, class_id)) {
        QMessageBox::information(this, "Properties", "Select a render object to view properties.");
        return;
    }

    switch (class_id) {
    case RenderObjClass::CLASSID_MESH:
    {
        MeshPropertiesDialog dialog(name, this);
        dialog.exec();
        return;
    }
    case RenderObjClass::CLASSID_COLLECTION:
    case RenderObjClass::CLASSID_HMODEL:
    case RenderObjClass::CLASSID_DISTLOD:
    case RenderObjClass::CLASSID_HLOD:
    {
        HierarchyPropertiesDialog dialog(name, this);
        dialog.exec();
        return;
    }
    case RenderObjClass::CLASSID_SOUND:
        editSoundObject();
        return;
    case RenderObjClass::CLASSID_PARTICLEEMITTER:
        editEmitter();
        return;
    case RenderObjClass::CLASSID_SPHERE:
    case RenderObjClass::CLASSID_RING:
        editPrimitive();
        return;
    default:
        QMessageBox::information(this,
                                 "Properties",
                                 "Selected object type does not have a properties dialog.");
        return;
    }
}

void W3DViewMainWindow::setNpatchesLevel(int level)
{
    if (level < 1) {
        level = 1;
    }
    if (level > 8) {
        level = 8;
    }

    WW3D::Set_NPatches_Level(static_cast<unsigned int>(level));

    QSettings settings;
    settings.setValue("Config/NPatchesSubdivision", level);
}

void W3DViewMainWindow::toggleNpatchesGap(bool enabled)
{
    WW3D::Set_NPatches_Gap_Filling_Mode(
        enabled ? WW3D::NPATCHES_GAP_FILLING_ENABLED
                : WW3D::NPATCHES_GAP_FILLING_DISABLED);

    QSettings settings;
    settings.setValue("Config/NPatchesGapFilling", enabled ? 1 : 0);
}

void W3DViewMainWindow::reloadLightmapModels()
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager || !_treeModel) {
        return;
    }

    auto *root = _treeModel->invisibleRootItem();
    if (!root) {
        return;
    }

    auto matches_group = [](const QString &text, const QString &label) {
        return text == label || text.startsWith(label + " (");
    };

    auto remove_child_prototypes = [&](const QString &label) {
        QStandardItem *group = nullptr;
        const int root_count = root->rowCount();
        for (int i = 0; i < root_count; ++i) {
            auto *child = root->child(i);
            if (!child) {
                continue;
            }
            if (matches_group(child->text(), label)) {
                group = child;
                break;
            }
        }

        if (!group) {
            return;
        }

        const int count = group->rowCount();
        for (int index = 0; index < count; ++index) {
            auto *item = group->child(index);
            if (!item) {
                continue;
            }
            if (item->data(kRoleType).toInt() != static_cast<int>(AssetNodeType::RenderObject)) {
                continue;
            }
            const QString name = item->data(kRoleName).toString();
            if (name.isEmpty()) {
                continue;
            }
            const QByteArray name_bytes = name.toLatin1();
            asset_manager->Remove_Prototype(name_bytes.constData());
        }
    };

    remove_child_prototypes("Mesh");
    remove_child_prototypes("Hierarchy");
    remove_child_prototypes("Mesh Collection");
}

void W3DViewMainWindow::reloadDisplayedObject()
{
    if (!_treeView) {
        return;
    }

    const QModelIndex current = _treeView->currentIndex();
    if (current.isValid()) {
        onCurrentChanged(current, QModelIndex());
    }
}

bool W3DViewMainWindow::loadAssetsFromFile(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, "W3DViewQt", QString("File not found:\n%1").arg(path));
        return false;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "W3DViewQt", "WW3D asset manager is not available.");
        return false;
    }

    asset_manager->Load_Procedural_Textures();

    const QString directory = QFileInfo(path).absolutePath();
    if (!directory.isEmpty()) {
        QDir::setCurrent(directory);
        applyTexturePath(directory);
    }

    const QByteArray path_bytes = QDir::toNativeSeparators(path).toLocal8Bit();
    if (!asset_manager->Load_3D_Assets(path_bytes.constData())) {
        QMessageBox::warning(this, "W3DViewQt", "Failed to load W3D assets.");
        return false;
    }

    LoadMissingHierarchyAssets(asset_manager, directory);

    _lastOpenedPath = info.absolutePath();
    const QString absolute_path = info.absoluteFilePath();
    const auto already_loaded = std::find_if(
        _loadedFiles.cbegin(), _loadedFiles.cend(), [&absolute_path](const QString &loaded) {
            return loaded.compare(absolute_path, Qt::CaseInsensitive) == 0;
        });
    if (already_loaded == _loadedFiles.cend()) {
        _loadedFiles.append(absolute_path);
    }
    QSettings settings;
    settings.setValue("Config/LastOpenedPath", _lastOpenedPath);
    setWindowTitle(QString("W3DViewQt - %1").arg(info.fileName()));
    rebuildAssetTree();
    addRecentFile(info.absoluteFilePath());
    statusBar()->showMessage(QString("Loaded: %1").arg(info.fileName()));
    return true;
}

void W3DViewMainWindow::updateRecentFilesMenu()
{
    if (!_fileMenu || !_recentFilesPlaceholderAction) {
        return;
    }

    for (QAction *action : _recentFileActions) {
        _fileMenu->removeAction(action);
        action->setObjectName({});
        action->deleteLater();
    }
    _recentFileActions.clear();

    QSettings settings;
    const auto files =
        qtcommon::ReadRecentFiles(settings, QStringLiteral("recentFiles"), kMaxRecentFiles);
    if (settings.value(QStringLiteral("recentFiles")).toStringList() != files) {
        qtcommon::WriteRecentFiles(
            settings, files, QStringLiteral("recentFiles"), kMaxRecentFiles);
    }
    _recentFilesPlaceholderAction->setVisible(files.isEmpty());
    _recentFilesPlaceholderAction->setEnabled(false);
    if (files.isEmpty()) {
        return;
    }

    int index = 1;
    for (const QString &path : files) {
        const QFileInfo info(path);
        const int item_index = index++;
        const QString label = QString("&%1 %2").arg(item_index).arg(info.fileName());
        auto *action = new QAction(label, _fileMenu);
        action->setObjectName(QString("recentFileAction%1").arg(item_index));
        action->setData(path);
        action->setToolTip(path);
        connect(action, &QAction::triggered, this, &W3DViewMainWindow::openRecentFile);
        _fileMenu->insertAction(_recentFilesPlaceholderAction, action);
        _recentFileActions.append(action);
    }
}

void W3DViewMainWindow::addRecentFile(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    QSettings settings;
    const QStringList files = qtcommon::AddRecentFile(
        qtcommon::ReadRecentFiles(settings, QStringLiteral("recentFiles"), kMaxRecentFiles),
        path,
        kMaxRecentFiles);
    qtcommon::WriteRecentFiles(settings, files, QStringLiteral("recentFiles"), kMaxRecentFiles);
    updateRecentFilesMenu();
}

void W3DViewMainWindow::rebuildAssetTree()
{
    _treeModel->clear();
    _treeModel->setHorizontalHeaderLabels(QStringList() << "Assets");

    auto *root = _treeModel->invisibleRootItem();
    auto *materials_group = new QStandardItem("Materials");
    materials_group->setEditable(false);
    materials_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(materials_group);

    auto *mesh_group = new QStandardItem("Mesh");
    mesh_group->setEditable(false);
    mesh_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(mesh_group);

    auto *hierarchy_group = new QStandardItem("Hierarchy");
    hierarchy_group->setEditable(false);
    hierarchy_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(hierarchy_group);

    auto *hlod_group = new QStandardItem("H-LOD");
    hlod_group->setEditable(false);
    hlod_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(hlod_group);

    auto *collection_group = new QStandardItem("Mesh Collection");
    collection_group->setEditable(false);
    collection_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(collection_group);

    auto *aggregate_group = new QStandardItem("Aggregate");
    aggregate_group->setEditable(false);
    aggregate_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(aggregate_group);

    auto *emitter_group = new QStandardItem("Emitter");
    emitter_group->setEditable(false);
    emitter_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(emitter_group);

    auto *primitives_group = new QStandardItem("Primitives");
    primitives_group->setEditable(false);
    primitives_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(primitives_group);

    auto *sound_group = new QStandardItem("Sounds");
    sound_group->setEditable(false);
    sound_group->setData(static_cast<int>(AssetNodeType::Group), kRoleType);
    root->appendRow(sound_group);

    addMaterialItems(materials_group);
    addRenderObjectItems(mesh_group,
                         hierarchy_group,
                         hlod_group,
                         collection_group,
                         aggregate_group,
                         emitter_group,
                         primitives_group,
                         sound_group);
    addAnimationItems(hierarchy_group, hlod_group, aggregate_group);

    materials_group->sortChildren(0, Qt::AscendingOrder);
    mesh_group->sortChildren(0, Qt::AscendingOrder);
    hierarchy_group->sortChildren(0, Qt::AscendingOrder);
    hlod_group->sortChildren(0, Qt::AscendingOrder);
    collection_group->sortChildren(0, Qt::AscendingOrder);
    aggregate_group->sortChildren(0, Qt::AscendingOrder);
    emitter_group->sortChildren(0, Qt::AscendingOrder);
    primitives_group->sortChildren(0, Qt::AscendingOrder);
    sound_group->sortChildren(0, Qt::AscendingOrder);
    SortAnimationChildren(hierarchy_group);
    SortAnimationChildren(hlod_group);
    SortAnimationChildren(aggregate_group);

    _treeView->setExpanded(materials_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(mesh_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(hierarchy_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(hlod_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(collection_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(aggregate_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(emitter_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(primitives_group->index(), _autoExpandAssetTree);
    _treeView->setExpanded(sound_group->index(), _autoExpandAssetTree);
}

void W3DViewMainWindow::addMaterialItems(QStandardItem *parent)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    int count = 0;
    HashTemplateIterator<StringClass, TextureClass *> iterator(asset_manager->Texture_Hash());
    for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
        const char *name = iterator.Peek_Key();
        if (!name || !name[0]) {
            continue;
        }

        auto *texture = iterator.Peek_Value();
        auto *item = new QStandardItem(QString::fromLatin1(name));
        item->setEditable(false);
        item->setData(static_cast<int>(AssetNodeType::Material), kRoleType);
        item->setData(QString::fromLatin1(name), kRoleName);
        item->setData(QVariant::fromValue(reinterpret_cast<quintptr>(texture)), kRolePointer);
        parent->appendRow(item);
        ++count;
    }

    parent->setText(QString("Materials (%1)").arg(count));
}

void W3DViewMainWindow::addRenderObjectItems(QStandardItem *meshParent,
                                             QStandardItem *hierarchyParent,
                                             QStandardItem *hlodParent,
                                             QStandardItem *collectionParent,
                                             QStandardItem *aggregateParent,
                                             QStandardItem *emitterParent,
                                             QStandardItem *primitivesParent,
                                             QStandardItem *soundParent)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    RenderObjIterator *iterator = asset_manager->Create_Render_Obj_Iterator();
    if (!iterator) {
        return;
    }

    struct RenderObjectEntry
    {
        QString name;
        int class_id;
    };

    QVector<RenderObjectEntry> render_objects;
    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const char *name = iterator->Current_Item_Name();
        if (!name || !name[0] || !asset_manager->Render_Obj_Exists(name)) {
            continue;
        }

        render_objects.push_back(
            RenderObjectEntry{QString::fromLatin1(name), iterator->Current_Item_Class_ID()});
    }

    // Removing a prototype mutates the vector traversed by RenderObjIterator.
    // Snapshot first and release the iterator before converting old DISTLODs.
    asset_manager->Release_Render_Obj_Iterator(iterator);
    for (RenderObjectEntry &entry : render_objects) {
        if (entry.class_id == RenderObjClass::CLASSID_DISTLOD &&
            ConvertDistLodPrototype(asset_manager, entry.name)) {
            entry.class_id = RenderObjClass::CLASSID_HLOD;
        }
    }

    int mesh_count = 0;
    int hierarchy_count = 0;
    int hlod_count = 0;
    int collection_count = 0;
    int aggregate_count = 0;
    int emitter_count = 0;
    int primitive_count = 0;
    int sound_count = 0;
    for (const RenderObjectEntry &entry : render_objects) {
        const QByteArray name_bytes = entry.name.toLatin1();
        const char *name = name_bytes.constData();
        if (!asset_manager->Render_Obj_Exists(name)) {
            continue;
        }

        const int class_id = entry.class_id;
        QStandardItem *parent = nullptr;
        bool insert = false;

        switch (class_id) {
        case RenderObjClass::CLASSID_COLLECTION:
            insert = true;
            parent = collectionParent;
            break;
        case RenderObjClass::CLASSID_MESH:
            insert = true;
            parent = meshParent;
            break;
        case RenderObjClass::CLASSID_SOUND:
            insert = true;
            parent = soundParent;
            break;
        case RenderObjClass::CLASSID_PARTICLEEMITTER:
            insert = true;
            parent = emitterParent;
            break;
        case RenderObjClass::CLASSID_SPHERE:
        case RenderObjClass::CLASSID_RING:
            insert = true;
            parent = primitivesParent;
            break;
        case RenderObjClass::CLASSID_DISTLOD:
        case RenderObjClass::CLASSID_HLOD:
            insert = true;
            parent = hierarchyParent;
            break;
        case RenderObjClass::CLASSID_HMODEL:
            insert = true;
            parent = hierarchyParent;
            break;
        default:
            break;
        }

        if (!insert || !parent) {
            continue;
        }

        const RenderObjInfo info = InspectRenderObj(name);
        if ((class_id == RenderObjClass::CLASSID_DISTLOD ||
             class_id == RenderObjClass::CLASSID_HLOD) &&
            info.isRealLod) {
            parent = hlodParent;
        }

        if (info.isAggregate) {
            parent = aggregateParent;
        }

        auto *item = new QStandardItem(entry.name);
        item->setEditable(false);
        item->setData(static_cast<int>(AssetNodeType::RenderObject), kRoleType);
        item->setData(entry.name, kRoleName);
        item->setData(class_id, kRoleClassId);
        if (!info.hierarchyName.isEmpty()) {
            item->setData(info.hierarchyName, kRoleHierarchyName);
        }
        parent->appendRow(item);

        if (parent == meshParent) {
            ++mesh_count;
        } else if (parent == hierarchyParent) {
            ++hierarchy_count;
        } else if (parent == hlodParent) {
            ++hlod_count;
        } else if (parent == collectionParent) {
            ++collection_count;
        } else if (parent == aggregateParent) {
            ++aggregate_count;
        } else if (parent == emitterParent) {
            ++emitter_count;
        } else if (parent == primitivesParent) {
            ++primitive_count;
        } else if (parent == soundParent) {
            ++sound_count;
        }
    }

    meshParent->setText(QString("Mesh (%1)").arg(mesh_count));
    hierarchyParent->setText(QString("Hierarchy (%1)").arg(hierarchy_count));
    hlodParent->setText(QString("H-LOD (%1)").arg(hlod_count));
    collectionParent->setText(QString("Mesh Collection (%1)").arg(collection_count));
    aggregateParent->setText(QString("Aggregate (%1)").arg(aggregate_count));
    emitterParent->setText(QString("Emitter (%1)").arg(emitter_count));
    primitivesParent->setText(QString("Primitives (%1)").arg(primitive_count));
    soundParent->setText(QString("Sounds (%1)").arg(sound_count));
}

void W3DViewMainWindow::addAnimationItems(QStandardItem *hierarchyParent,
                                          QStandardItem *hlodParent,
                                          QStandardItem *aggregateParent)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    AssetIterator *iterator = asset_manager->Create_HAnim_Iterator();
    if (!iterator) {
        return;
    }

    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const char *anim_name = iterator->Current_Item_Name();
        if (!anim_name || !anim_name[0]) {
            continue;
        }

        HAnimClass *anim = asset_manager->Get_HAnim(anim_name);
        if (!anim) {
            continue;
        }

        const char *hier_name = anim->Get_HName();
        const QString hierarchy = hier_name ? QString::fromLatin1(hier_name) : QString();
        anim->Release_Ref();

        if (hierarchy.isEmpty()) {
            continue;
        }

        QVector<QStandardItem *> targets;
        if (_restrictAnims) {
            CollectHierarchyItems(hierarchyParent, hierarchy, targets);
            CollectHierarchyItems(hlodParent, hierarchy, targets);
            CollectHierarchyItems(aggregateParent, hierarchy, targets);
        } else {
            CollectAllChildren(hierarchyParent, targets);
            CollectAllChildren(hlodParent, targets);
            CollectAllChildren(aggregateParent, targets);
        }

        if (targets.isEmpty()) {
            continue;
        }

        const QString anim_text = QString::fromLatin1(anim_name);
        for (auto *target : targets) {
            if (!target) {
                continue;
            }

            auto *item = new QStandardItem(anim_text);
            item->setEditable(false);
            item->setData(static_cast<int>(AssetNodeType::Animation), kRoleType);
            item->setData(anim_text, kRoleName);
            target->appendRow(item);
        }
    }

    delete iterator;
}

void W3DViewMainWindow::loadAppSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("Window/Geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    const QByteArray window_state = settings.value("Window/State").toByteArray();
    if (!window_state.isEmpty()) {
        restoreState(window_state);
    }
    _lastOpenedPath = settings.value("Config/LastOpenedPath").toString();
    _texturePath1 = settings.value("Config/TexturePath1").toString();
    _texturePath2 = settings.value("Config/TexturePath2").toString();
    _sortingEnabled = settings.value("Config/EnableSorting", true).toBool();
    _animateCamera = settings.value("Config/AnimateCamera", false).toBool();
    _autoResetCamera = settings.value("Config/ResetCamera", true).toBool();
    _autoExpandAssetTree = settings.value("Config/AutoExpandAssetTree", true).toBool();
    const bool invert_culling = settings.value("Config/InvertBackfaceCulling", false).toBool();
    const bool manual_fov = settings.value("Config/UseManualFOV", false).toBool();
    const bool manual_clip = settings.value("Config/UseManualClipPlanes", false).toBool();
    const double hfov_rad = settings.value("Config/hfov", 0.0).toDouble();
    const double vfov_rad = settings.value("Config/vfov", 0.0).toDouble();
    const float znear = settings.value("Config/znear", 0.1).toFloat();
    const float zfar = settings.value("Config/zfar", 100.0).toFloat();
    const int device_width = settings.value("Config/DeviceWidth", 0).toInt();
    const int device_height = settings.value("Config/DeviceHeight", 0).toInt();
    const int device_bits_per_pixel = settings.value("Config/DeviceBitsPerPix", 32).toInt();
    const bool fullscreen = settings.value("Config/Windowed", 1).toInt() == 0;
    const bool gamma_enabled = settings.value("Config/EnableGamma", 0).toInt() != 0;
    const bool munge_sort = settings.value("Config/MungeSortOnLoad", 0).toInt() != 0;
    int npatches_level = settings.value("Config/NPatchesSubdivision", 4).toInt();
    if (npatches_level < 1) {
        npatches_level = 1;
    }
    if (npatches_level > 8) {
        npatches_level = 8;
    }
    const bool npatches_gap = settings.value("Config/NPatchesGapFilling", 0).toInt() != 0;
    WW3D::Enable_Sorting(_sortingEnabled);

    applyTexturePath(_texturePath1);
    applyTexturePath(_texturePath2);

    if (_viewport) {
        _viewport->setInitialDisplayMode(
            device_width, device_height, device_bits_per_pixel, fullscreen);
        _viewport->setCameraAnimationEnabled(_animateCamera);
        _viewport->setAutoResetEnabled(_autoResetCamera);
        _viewport->setManualFovEnabled(manual_fov);
        if (manual_fov && hfov_rad > 0.0 && vfov_rad > 0.0) {
            _viewport->setCameraFovDegrees(hfov_rad * kRadToDeg, vfov_rad * kRadToDeg);
        }
        _viewport->setManualClipPlanesEnabled(manual_clip);
        if (manual_clip) {
            _viewport->setCameraClipPlanes(znear, zfar);
        }
    }

    if (fullscreen) {
        setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }

    if (_enableGammaAction) {
        _enableGammaAction->setChecked(gamma_enabled);
    }
    if (_mungeSortAction) {
        _mungeSortAction->setChecked(munge_sort);
    }
    if (_autoExpandTreeAction) {
        _autoExpandTreeAction->setChecked(_autoExpandAssetTree);
    }
    if (gamma_enabled) {
        int gamma = settings.value("Config/Gamma", 10).toInt();
        if (gamma < 10) {
            gamma = 10;
        }
        if (gamma > 30) {
            gamma = 30;
        }
        DX8Wrapper::Set_Gamma(gamma / 10.0f, 0.0f, 1.0f);
    }

    ShaderClass::Invert_Backface_Culling(invert_culling);
    WW3D::Enable_Munge_Sort_On_Load(munge_sort);

    WW3D::Set_NPatches_Level(static_cast<unsigned int>(npatches_level));
    WW3D::Set_NPatches_Gap_Filling_Mode(
        npatches_gap ? WW3D::NPATCHES_GAP_FILLING_ENABLED
                     : WW3D::NPATCHES_GAP_FILLING_DISABLED);
    if (_npatchesGroup) {
        for (auto *action : _npatchesGroup->actions()) {
            if (action && action->data().toInt() == npatches_level) {
                action->setChecked(true);
                break;
            }
        }
    }
    if (_npatchesGapAction) {
        _npatchesGapAction->setChecked(npatches_gap);
    }
}

void W3DViewMainWindow::loadDefaultSettings()
{
    const QString default_path = QDir(QCoreApplication::applicationDirPath()).filePath("default.dat");
    if (!QFileInfo::exists(default_path)) {
        return;
    }

    loadSettingsPath(default_path);
}

void W3DViewMainWindow::playAnimationSound()
{
    const QString animation_name = _viewport ? _viewport->currentAnimationName() : QString();
    const qsizetype separator = animation_name.indexOf('.');
    if (separator < 0 || separator + 1 >= animation_name.size()) {
        return;
    }

    const QString sound_filename = animation_name.mid(separator + 1) + ".wav";
    stopAnimationSound();

    WWAudioClass *audio = WWAudioClass::Get_Instance();
    if (!audio) {
        return;
    }

    const QString native_filename = QDir::toNativeSeparators(sound_filename);
    const QByteArray filename_bytes = QFile::encodeName(native_filename);
    _animationSound = audio->Create_Sound_Effect(filename_bytes.constData());
    if (_animationSound && !_animationSound->Play()) {
        _animationSound->Release_Ref();
        _animationSound = nullptr;
    }
}

void W3DViewMainWindow::stopAnimationSound()
{
    if (!_animationSound) {
        return;
    }

    _animationSound->Stop();
    _animationSound->Release_Ref();
    _animationSound = nullptr;
}

void W3DViewMainWindow::applyMainToolbarIcons()
{
    const QPixmap strip(":/w3dview/main-toolbar.bmp");
    if (strip.isNull() || strip.width() < 16 || strip.height() < 15) {
        return;
    }

    const QList<QAction *> actions = {
        _newAction,
        _openAction,
        _exportEmitterAction,
        _exportAggregateAction,
        _exportLodAction,
        _exportPrimitiveAction,
        _exportSoundObjectAction,
        _listMissingTexturesAction,
        _copyAssetsAction,
        _addToLineupAction,
        _aboutAction,
    };
    const int action_count = static_cast<int>(actions.size());
    const int icon_width = strip.width() / action_count;
    for (int index = 0; index < action_count; ++index) {
        QAction *action = actions[index];
        if (!action) {
            continue;
        }
        QPixmap icon = strip.copy(index * icon_width, 0, icon_width, strip.height());
        const QColor mask_color = icon.toImage().pixelColor(0, 0);
        icon.setMask(icon.createMaskFromColor(mask_color, Qt::MaskInColor));
        action->setIcon(QIcon(icon));
    }
}

void W3DViewMainWindow::applyTexturePath(const QString &path)
{
    const QString cleaned = NormalizeOptionalPath(path);
    if (cleaned.isEmpty() || !_TheSimpleFileFactory) {
        return;
    }

    const QByteArray native = QDir::toNativeSeparators(cleaned).toLocal8Bit();
    _TheSimpleFileFactory->Append_Sub_Directory(native.constData());
}

void W3DViewMainWindow::setTexturePaths(const QString &path1, const QString &path2)
{
    QSettings settings;

    const QString cleaned1 = NormalizeOptionalPath(path1);
    if (cleaned1.compare(_texturePath1, Qt::CaseInsensitive) != 0) {
        applyTexturePath(cleaned1);
        _texturePath1 = cleaned1;
        settings.setValue("Config/TexturePath1", _texturePath1);
    }

    const QString cleaned2 = NormalizeOptionalPath(path2);
    if (cleaned2.compare(_texturePath2, Qt::CaseInsensitive) != 0) {
        applyTexturePath(cleaned2);
        _texturePath2 = cleaned2;
        settings.setValue("Config/TexturePath2", _texturePath2);
    }
}

void W3DViewMainWindow::applySettings(QSettings &settings)
{
    if (!_viewport) {
        return;
    }

    settings.beginGroup("Settings");

    if (settings.contains("AmbientLightR") && settings.contains("AmbientLightG") &&
        settings.contains("AmbientLightB")) {
        const float amb_r = settings.value("AmbientLightR").toFloat();
        const float amb_g = settings.value("AmbientLightG").toFloat();
        const float amb_b = settings.value("AmbientLightB").toFloat();
        _viewport->setAmbientLight(Vector3(amb_r, amb_g, amb_b));
    }

    const bool has_legacy_scene_light = settings.contains("SceneLightR")
        && settings.contains("SceneLightG") && settings.contains("SceneLightB");
    const Vector3 legacy_scene_light(
        settings.value("SceneLightR", 1.0f).toFloat(),
        settings.value("SceneLightG", 1.0f).toFloat(),
        settings.value("SceneLightB", 1.0f).toFloat());
    const bool has_scene_light_diffuse = settings.contains("SceneLightDiffuseR")
        && settings.contains("SceneLightDiffuseG") && settings.contains("SceneLightDiffuseB");
    const bool has_scene_light_specular = settings.contains("SceneLightSpecularR")
        && settings.contains("SceneLightSpecularG") && settings.contains("SceneLightSpecularB");
    const Vector3 stored_scene_light_diffuse(
        settings.value("SceneLightDiffuseR", legacy_scene_light.X).toFloat(),
        settings.value("SceneLightDiffuseG", legacy_scene_light.Y).toFloat(),
        settings.value("SceneLightDiffuseB", legacy_scene_light.Z).toFloat());
    const bool legacy_scene_light_was_updated = has_legacy_scene_light && has_scene_light_diffuse
        && (legacy_scene_light.X != stored_scene_light_diffuse.X
            || legacy_scene_light.Y != stored_scene_light_diffuse.Y
            || legacy_scene_light.Z != stored_scene_light_diffuse.Z);

    if (has_legacy_scene_light && (!has_scene_light_diffuse || legacy_scene_light_was_updated)) {
        // A legacy/MFC writer only updates SceneLightR/G/B. When those values no
        // longer mirror the stored diffuse channel, honor that update for both
        // channels instead of resurrecting stale Qt-only values.
        _viewport->setSceneLightSpecular(legacy_scene_light);
        _viewport->setSceneLightDiffuse(legacy_scene_light);
    } else {
        if (has_scene_light_diffuse) {
            _viewport->setSceneLightDiffuse(stored_scene_light_diffuse);
        }
        if (has_scene_light_specular) {
            _viewport->setSceneLightSpecular(Vector3(
                settings.value("SceneLightSpecularR").toFloat(),
                settings.value("SceneLightSpecularG").toFloat(),
                settings.value("SceneLightSpecularB").toFloat()));
        } else if (has_legacy_scene_light) {
            _viewport->setSceneLightSpecular(legacy_scene_light);
        }
    }

    if (settings.contains("SceneLightX") && settings.contains("SceneLightY") &&
        settings.contains("SceneLightZ") && settings.contains("SceneLightW")) {
        Quaternion orientation(true);
        orientation.X = settings.value("SceneLightX").toFloat();
        orientation.Y = settings.value("SceneLightY").toFloat();
        orientation.Z = settings.value("SceneLightZ").toFloat();
        orientation.W = settings.value("SceneLightW").toFloat();
        _viewport->setSceneLightOrientation(orientation);
    }

    if (settings.contains("SceneLightDistance") && settings.contains("SceneLightIntensity") &&
        settings.contains("SceneLightAttenStart") && settings.contains("SceneLightAttenEnd") &&
        settings.contains("SceneLightAttenOn")) {
        const float distance = settings.value("SceneLightDistance").toFloat();
        const float intensity = settings.value("SceneLightIntensity").toFloat();
        const float atten_start = settings.value("SceneLightAttenStart").toFloat();
        const float atten_end = settings.value("SceneLightAttenEnd").toFloat();
        const bool atten_on = settings.value("SceneLightAttenOn").toBool();
        _viewport->setSceneLightIntensity(intensity);
        _viewport->setSceneLightAttenuation(atten_start, atten_end, atten_on);
        _viewport->setSceneLightDistance(distance);
    }

    if (settings.contains("BackgroundR") && settings.contains("BackgroundG") &&
        settings.contains("BackgroundB")) {
        const float bg_r = settings.value("BackgroundR").toFloat();
        const float bg_g = settings.value("BackgroundG").toFloat();
        const float bg_b = settings.value("BackgroundB").toFloat();
        _viewport->setBackgroundColor(Vector3(bg_r, bg_g, bg_b));
    }

    if (settings.contains("BackgroundBMP")) {
        _viewport->setBackgroundBitmap(settings.value("BackgroundBMP").toString());
    }

    if (settings.contains("FogEnabled")) {
        _viewport->setFogEnabled(settings.value("FogEnabled").toBool());
    }

    settings.endGroup();
    if (_fogAction) {
        _fogAction->setChecked(_viewport->isFogEnabled());
    }
}

void W3DViewMainWindow::writeSettings(QSettings &settings,
                                      bool saveLighting,
                                      bool saveBackground) const
{
    if (!_viewport) {
        return;
    }

    settings.beginGroup("Settings");

    if (saveLighting) {
        const Vector3 ambient = _viewport->ambientLight();
        settings.setValue("AmbientLightR", ambient.X);
        settings.setValue("AmbientLightG", ambient.Y);
        settings.setValue("AmbientLightB", ambient.Z);

        const Vector3 scene_light_diffuse = _viewport->sceneLightDiffuse();
        const Vector3 scene_light_specular = _viewport->sceneLightSpecular();
        settings.setValue("SceneLightR", scene_light_diffuse.X);
        settings.setValue("SceneLightG", scene_light_diffuse.Y);
        settings.setValue("SceneLightB", scene_light_diffuse.Z);
        settings.setValue("SceneLightDiffuseR", scene_light_diffuse.X);
        settings.setValue("SceneLightDiffuseG", scene_light_diffuse.Y);
        settings.setValue("SceneLightDiffuseB", scene_light_diffuse.Z);
        settings.setValue("SceneLightSpecularR", scene_light_specular.X);
        settings.setValue("SceneLightSpecularG", scene_light_specular.Y);
        settings.setValue("SceneLightSpecularB", scene_light_specular.Z);

        const Quaternion orientation = _viewport->sceneLightOrientation();
        settings.setValue("SceneLightX", orientation.X);
        settings.setValue("SceneLightY", orientation.Y);
        settings.setValue("SceneLightZ", orientation.Z);
        settings.setValue("SceneLightW", orientation.W);

        settings.setValue("SceneLightDistance", _viewport->sceneLightDistance());
        settings.setValue("SceneLightIntensity", _viewport->sceneLightIntensity());

        float atten_start = 0.0f;
        float atten_end = 0.0f;
        bool atten_on = false;
        _viewport->sceneLightAttenuation(atten_start, atten_end, atten_on);
        settings.setValue("SceneLightAttenStart", atten_start);
        settings.setValue("SceneLightAttenEnd", atten_end);
        settings.setValue("SceneLightAttenOn", atten_on ? 1 : 0);
    }

    if (saveBackground) {
        const Vector3 background = _viewport->backgroundColor();
        settings.setValue("BackgroundR", background.X);
        settings.setValue("BackgroundG", background.Y);
        settings.setValue("BackgroundB", background.Z);
        settings.setValue("BackgroundBMP", _viewport->backgroundBitmap());
        settings.setValue("FogEnabled", _viewport->isFogEnabled());
    }

    settings.endGroup();
}
