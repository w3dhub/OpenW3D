#include "ViewerAssetManager.h"
#include "RenderObjUtils.h"

#include "chunkio.h"
#include "hlod.h"
#include "part_ldr.h"
#include "proxy.h"
#include "ramfile.h"
#include "sphereobj.h"
#include "w3d_file.h"
#include "wwmath.h"

#include <QCoreApplication>
#include <QtTest/QTest>

#include <cstring>
#include <functional>
#include <memory>

namespace {
QByteArray serialize(const std::function<bool(ChunkSaveClass &)> &writer)
{
    QByteArray bytes(65536, '\0');
    RAMFileClass file(bytes.data(), static_cast<int>(bytes.size()));
    if (!file.Open(FileClass::WRITE)) {
        return {};
    }
    ChunkSaveClass save(&file);
    if (!writer(save) || save.Cur_Chunk_Depth() != 0) {
        return {};
    }
    bytes.resize(file.Size());
    return bytes;
}

template<class T>
bool dataChunk(ChunkSaveClass &save, uint32 id, const T &value)
{
    return save.Begin_Chunk(id) && save.Write(&value, sizeof(value)) == sizeof(value)
        && save.End_Chunk();
}

bool hierarchy(ChunkSaveClass &save)
{
    W3dHierarchyStruct header{};
    header.Version = W3D_CURRENT_HTREE_VERSION;
    std::strcpy(header.Name, "IO_RIG");
    header.NumPivots = 1;
    W3dPivotStruct pivot{};
    std::strcpy(pivot.Name, "ROOTTRANSFORM");
    pivot.ParentIdx = static_cast<uint32>(-1);
    pivot.Rotation.Q[3] = 1.0f;
    return save.Begin_Chunk(W3D_CHUNK_HIERARCHY)
        && dataChunk(save, W3D_CHUNK_HIERARCHY_HEADER, header)
        && dataChunk(save, W3D_CHUNK_PIVOTS, pivot) && save.End_Chunk();
}

bool modelArray(ChunkSaveClass &save, uint32 id, float area,
                std::initializer_list<const char *> names)
{
    W3dHLodArrayHeaderStruct header{};
    header.ModelCount = static_cast<uint32>(names.size());
    header.MaxScreenSize = area;
    if (!save.Begin_Chunk(id) ||
        !dataChunk(save, W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, header)) {
        return false;
    }
    for (const char *name : names) {
        W3dHLodSubObjectStruct model{};
        std::strncpy(model.Name, name, sizeof(model.Name) - 1);
        if (!dataChunk(save, W3D_CHUNK_HLOD_SUB_OBJECT, model)) {
            return false;
        }
    }
    return save.End_Chunk();
}

QByteArray hlodFixture()
{
    return serialize([](ChunkSaveClass &save) {
        W3dHLodHeaderStruct header{};
        header.Version = W3D_CURRENT_HLOD_VERSION;
        header.LodCount = 2;
        std::strcpy(header.Name, "IO_HLOD");
        std::strcpy(header.HierarchyName, "IO_RIG");
        const uint32 unknown = 0x76543210;
        ChunkHeader emptyContainer(0x100fffe, 0);
        emptyContainer.Set_Sub_Chunk_Flag(true);
        return save.Begin_Chunk(W3D_CHUNK_HLOD)
            && dataChunk(save, W3D_CHUNK_HLOD_HEADER, header)
            && modelArray(save, W3D_CHUNK_HLOD_LOD_ARRAY, 64.0f, {"IO_PART", "MISSING_LOW"})
            && modelArray(save, W3D_CHUNK_HLOD_LOD_ARRAY, NO_MAX_SCREEN_SIZE, {"IO_PART"})
            && modelArray(save, W3D_CHUNK_HLOD_AGGREGATE_ARRAY, 321.0f,
                          {"IO_PART", "MISSING_ATTACHMENT"})
            && modelArray(save, W3D_CHUNK_HLOD_PROXY_ARRAY, 123.0f,
                          {"PROXY_ALPHA", "PROXY_BETA"})
            && dataChunk(save, 0x100ffff, unknown)
            && save.Write(&emptyContainer, sizeof(emptyContainer)) == sizeof(emptyContainer)
            && save.End_Chunk();
    });
}

QByteArray childBytes(const QByteArray &file, uint32 id, int occurrence = 0)
{
    qsizetype offset = sizeof(ChunkHeader);
    while (offset + static_cast<qsizetype>(sizeof(ChunkHeader)) <= file.size()) {
        ChunkHeader header;
        std::memcpy(&header, file.constData() + offset, sizeof(header));
        const qsizetype size = sizeof(ChunkHeader) + header.Get_Size();
        if (size > file.size() - offset) {
            return {};
        }
        if (header.ChunkType == id && occurrence-- == 0) {
            return file.mid(offset, size);
        }
        offset += size;
    }
    return {};
}

bool loadAssets(ViewerAssetManager &manager, QByteArray bytes)
{
    RAMFileClass file(bytes.data(), static_cast<int>(bytes.size()));
    return manager.Load_3D_Assets(file);
}

template<class T>
struct ReleaseRef {
    void operator()(T *object) const { if (object) object->Release_Ref(); }
};

class EmitterFixture final : public ParticleEmitterDefClass
{
public:
    EmitterFixture()
    {
        Set_Name("IO_LINE");
        auto *properties = const_cast<W3dEmitterLinePropertiesStruct *>(Get_Line_Properties());
        properties->Flags = 0x00010000;
        for (int index = 0; index < 9; ++index) {
            properties->Reserved[index] = 0x10203040 + index;
        }
    }
};
}

class ViewerAssetIOTests final : public QObject
{
    Q_OBJECT
private slots:
    void emitterLinePropertiesRoundTrip_data();
    void emitterLinePropertiesRoundTrip();
    void hlodMetadataSurvivesExportAndLodEdits();
};

void ViewerAssetIOTests::emitterLinePropertiesRoundTrip_data()
{
    QTest::addColumn<int>("mode");
    QTest::newRow("triangles") << W3D_EMITTER_RENDER_MODE_TRI_PARTICLES;
    QTest::newRow("quads") << W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES;
    QTest::newRow("line") << W3D_EMITTER_RENDER_MODE_LINE;
    QTest::newRow("tetra") << W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA;
    QTest::newRow("prism") << W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM;
}

void ViewerAssetIOTests::emitterLinePropertiesRoundTrip()
{
    QFETCH(int, mode);
    EmitterFixture original;
    original.Set_Render_Mode(mode);
    original.Set_Merge_Intersections(true);
    original.Set_Freeze_Random(true);
    original.Set_Disable_Sorting(true);
    original.Set_End_Caps(true);
    original.Set_Line_Texture_Mapping_Mode(W3D_ELINE_TILED_TEXTURE_MAP);
    original.Set_Subdivision_Level(5);
    original.Set_Noise_Amplitude(1.25f);
    original.Set_Merge_Abort_Factor(2.75f);
    original.Set_Texture_Tile_Factor(3.5f);
    original.Set_UV_Offset_Rate(Vector2(-0.25f, 0.625f));
    QByteArray exported = serialize([&](ChunkSaveClass &save) {
        return SaveViewerEmitter(save, original);
    });
    QVERIFY(!exported.isEmpty());
    QVERIFY(!childBytes(exported, W3D_CHUNK_EMITTER_LINE_PROPERTIES).isEmpty());

    ViewerAssetManager manager;
    manager.Set_WW3D_Load_On_Demand(false);
    QVERIFY(loadAssets(manager, exported));
    auto *prototype = dynamic_cast<ParticleEmitterPrototypeClass *>(
        manager.Find_Prototype(original.Get_Name()));
    QVERIFY(prototype);
    const ParticleEmitterDefClass &restored = *prototype->Get_Definition();
    QCOMPARE(restored.Get_Render_Mode(), mode);
    QCOMPARE(std::memcmp(original.Get_Line_Properties(), restored.Get_Line_Properties(),
                         sizeof(W3dEmitterLinePropertiesStruct)), 0);
    const QByteArray exportedAgain = serialize([&](ChunkSaveClass &save) {
        return SaveViewerEmitter(save, restored);
    });
    QCOMPARE(exportedAgain, exported);
}

void ViewerAssetIOTests::hlodMetadataSurvivesExportAndLodEdits()
{
    ViewerAssetManager manager;
    manager.Set_WW3D_Load_On_Demand(false);
    std::unique_ptr<SphereRenderObjClass, ReleaseRef<SphereRenderObjClass>> part(new SphereRenderObjClass);
    part->Set_Name("IO_PART");
    manager.Add_Prototype(new SpherePrototypeClass(part.get()));
    QVERIFY(loadAssets(manager, serialize(hierarchy)));
    const QByteArray original = hlodFixture();
    QVERIFY(!original.isEmpty());
    QVERIFY(loadAssets(manager, original));
    auto *prototype = dynamic_cast<HLodPrototypeClass *>(manager.Find_Prototype("IO_HLOD"));
    QVERIFY(prototype);
    QCOMPARE(serialize([&](ChunkSaveClass &save) { return SaveViewerHlod(save, *prototype); }),
             original);

    std::unique_ptr<HLodClass, ReleaseRef<HLodClass>> object(
        dynamic_cast<HLodClass *>(prototype->Create()));
    QVERIFY(object);
    QCOMPARE(object->Get_Additional_Model_Count(), 1);
    QCOMPARE(object->Get_Proxy_Count(), 2);
    ProxyClass proxy;
    QVERIFY(object->Get_Proxy(1, proxy));
    QCOMPARE(QByteArray(proxy.Get_Name()), QByteArray("PROXY_BETA"));
    object->Set_Max_Screen_Size(0, 456.0f);
    object->Include_NULL_Lod(true);
    QVERIFY(UpdateLodPrototype(*object));
    prototype = dynamic_cast<HLodPrototypeClass *>(manager.Find_Prototype("IO_HLOD"));
    QVERIFY(prototype);
    QByteArray edited = serialize([&](ChunkSaveClass &save) {
        return SaveViewerHlod(save, *prototype);
    });
    QVERIFY(!edited.isEmpty());
    for (uint32 id : {uint32(W3D_CHUNK_HLOD_AGGREGATE_ARRAY),
                      uint32(W3D_CHUNK_HLOD_PROXY_ARRAY), uint32(0x100ffff), uint32(0x100fffe)}) {
        QCOMPARE(childBytes(edited, id), childBytes(original, id));
    }
    QVERIFY(childBytes(edited, W3D_CHUNK_HLOD_LOD_ARRAY, 1).contains("MISSING_LOW"));
    object.reset(dynamic_cast<HLodClass *>(prototype->Create()));
    QVERIFY(object);
    QCOMPARE(object->Get_LOD_Count(), 3);
    QCOMPARE(object->Get_Max_Screen_Size(1), 456.0f);
    QCOMPARE(object->Get_Additional_Model_Count(), 1);
    QCOMPARE(object->Get_Proxy_Count(), 2);
    object->Include_NULL_Lod(false);
    QVERIFY(UpdateLodPrototype(*object));
    prototype = dynamic_cast<HLodPrototypeClass *>(manager.Find_Prototype("IO_HLOD"));
    QVERIFY(prototype);
    edited = serialize([&](ChunkSaveClass &save) { return SaveViewerHlod(save, *prototype); });
    QVERIFY(childBytes(edited, W3D_CHUNK_HLOD_LOD_ARRAY).contains("MISSING_LOW"));
    QCOMPARE(childBytes(edited, W3D_CHUNK_HLOD_PROXY_ARRAY),
             childBytes(original, W3D_CHUNK_HLOD_PROXY_ARRAY));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    WWMath::Init();
    ViewerAssetIOTests tests;
    const int result = QTest::qExec(&tests, argc, argv);
    WWMath::Shutdown();
    return result;
}

#include "ViewerAssetIOTests.moc"
