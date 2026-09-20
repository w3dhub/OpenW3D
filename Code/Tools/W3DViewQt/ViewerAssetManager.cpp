#include "ViewerAssetManager.h"

#include "AudibleSound.h"
#include "soundrobj.h"
#include "chunkio.h"
#include "hlod.h"
#include "htree.h"
#include "part_ldr.h"
#include "ramfile.h"
#include "w3d_file.h"

#include <QByteArray>
#include <QList>

#include <cstring>
#include <limits>
#include <memory>
#include <utility>

namespace {
struct Chunk
{
    uint32 id;
    QByteArray data;
    bool children;
};

bool ReadChunks(const QByteArray &data, QList<Chunk> &chunks)
{
    qsizetype offset = 0;
    while (offset < data.size()) {
        if (data.size() - offset < static_cast<qsizetype>(sizeof(ChunkHeader))) {
            return false;
        }
        ChunkHeader header;
        std::memcpy(&header, data.constData() + offset, sizeof(header));
        offset += sizeof(header);
        if (header.Get_Size() > data.size() - offset) {
            return false;
        }
        chunks.append({header.ChunkType, data.mid(offset, header.Get_Size()),
                       header.Get_Sub_Chunk_Flag() != 0});
        offset += header.Get_Size();
    }
    return true;
}

QByteArray EncodeChunks(const QList<Chunk> &chunks)
{
    QByteArray data;
    for (const Chunk &chunk : chunks) {
        ChunkHeader header(chunk.id, static_cast<uint32>(chunk.data.size()));
        header.Set_Sub_Chunk_Flag(chunk.children);
        data.append(reinterpret_cast<const char *>(&header), sizeof(header));
        data.append(chunk.data);
    }
    return data;
}

template<class T>
Chunk DataChunk(uint32 id, const T &value)
{
    return {id, QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), false};
}

class ViewerHlodPrototype final : public HLodPrototypeClass
{
public:
    ViewerHlodPrototype(HLodDefClass *definition, QList<Chunk> chunks)
        : HLodPrototypeClass(definition), chunks(std::move(chunks))
    {
    }

    const QList<Chunk> chunks;
};

HLodPrototypeClass *LoadHlod(QList<Chunk> chunks)
{
    if (chunks.isEmpty() || chunks.first().id != W3D_CHUNK_HLOD_HEADER ||
        chunks.first().data.size() != sizeof(W3dHLodHeaderStruct)) {
        return nullptr;
    }
    W3dHLodHeaderStruct header;
    std::memcpy(&header, chunks.first().data.constData(), sizeof(header));
    if (header.LodCount == 0 || header.LodCount >= static_cast<uint32>(chunks.size()) ||
        !std::memchr(header.Name, 0, sizeof(header.Name)) ||
        !std::memchr(header.HierarchyName, 0, sizeof(header.HierarchyName))) {
        return nullptr;
    }
    for (uint32 index = 0; index < header.LodCount; ++index) {
        if (chunks[index + 1].id != W3D_CHUNK_HLOD_LOD_ARRAY) {
            return nullptr;
        }
    }

    QByteArray fileData = EncodeChunks({{W3D_CHUNK_HLOD, EncodeChunks(chunks), true}});
    if (fileData.size() > std::numeric_limits<int>::max()) {
        return nullptr;
    }
    RAMFileClass file(fileData.data(), static_cast<int>(fileData.size()));
    if (!file.Open(FileClass::READ)) {
        return nullptr;
    }
    ChunkLoadClass load(&file);
    auto definition = std::make_unique<HLodDefClass>();
    if (!load.Open_Chunk() || definition->Load_W3D(load) != WW3D_ERROR_OK) {
        return nullptr;
    }
    return new ViewerHlodPrototype(definition.release(), std::move(chunks));
}

class ViewerHlodLoader final : public HLodLoaderClass
{
public:
    PrototypeClass *Load_W3D(ChunkLoadClass &load) override
    {
        const uint32 size = load.Cur_Chunk_Length();
        if (size > static_cast<uint32>(std::numeric_limits<int>::max())) {
            return nullptr;
        }
        QByteArray data(static_cast<int>(size), Qt::Uninitialized);
        if (load.Read(data.data(), size) != size) {
            return nullptr;
        }
        QList<Chunk> chunks;
        return ReadChunks(data, chunks) ? LoadHlod(std::move(chunks)) : nullptr;
    }
};

ViewerHlodLoader hlodLoader;

class ViewerEmitterDefinition final : public ParticleEmitterDefClass
{
public:
    ViewerEmitterDefinition() = default;

    explicit ViewerEmitterDefinition(const ParticleEmitterDefClass &definition)
        : ParticleEmitterDefClass(definition)
    {
    }

protected:
    WW3DErrorType Read_Line_Properties(ChunkLoadClass &load) override
    {
        // The engine hook tests EMITTER_INFO instead of EMITTER_LINE_PROPERTIES.
        // This definition is mutable; preserve flags and reserved fields as well.
        auto *properties = const_cast<W3dEmitterLinePropertiesStruct *>(Get_Line_Properties());
        return load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_LINE_PROPERTIES &&
            load.Cur_Chunk_Length() == sizeof(*properties) &&
            load.Read(properties, sizeof(*properties)) == sizeof(*properties)
                ? WW3D_ERROR_OK : WW3D_ERROR_LOAD_FAILED;
    }

    WW3DErrorType Save_Props(ChunkSaveClass &save) override
    {
        const WW3DErrorType result = ParticleEmitterDefClass::Save_Props(save);
        // The engine writer calls this hook but omits its existing line-property writer.
        return result == WW3D_ERROR_OK ? Save_Line_Properties(save) : result;
    }
};

class ViewerEmitterLoader final : public ParticleEmitterLoaderClass
{
public:
    PrototypeClass *Load_W3D(ChunkLoadClass &load) override
    {
        auto definition = std::make_unique<ViewerEmitterDefinition>();
        if (definition->Load_W3D(load) != WW3D_ERROR_OK) {
            return nullptr;
        }
        return new ParticleEmitterPrototypeClass(definition.release());
    }
};

ViewerEmitterLoader emitterLoader;

class ViewerSoundObject final : public SoundRenderObjClass
{
public:
    ViewerSoundObject() = default;

    explicit ViewerSoundObject(const SoundRenderObjClass &source)
    {
        Set_Name(source.Get_Name());
        Set_Flags(source.Get_Flags());
        Set_Transform(source.Get_Transform());
        if (auto *sound = source.Peek_Sound()) {
            if (auto *definition = sound->Get_Definition()) {
                Set_Sound(definition);
            } else {
                AudibleSoundDefinitionClass copiedDefinition;
                copiedDefinition.Initialize_From_Sound(sound);
                Set_Sound(&copiedDefinition);
            }
        }
    }

    ~ViewerSoundObject() override
    {
        // Release the sound while its owned definition still exists.
        Set_Sound(nullptr);
    }

    RenderObjClass *Clone() const override
    {
        return new ViewerSoundObject(*static_cast<const SoundRenderObjClass *>(this));
    }

    void Set_Sound(AudibleSoundDefinitionClass *definition) override
    {
        if (auto *sound = Peek_Sound()) {
            sound->Attach_To_Object(nullptr);
        }
        SoundRenderObjClass::Set_Sound(nullptr);
        if (definition) {
            _definition = *definition;
            SoundRenderObjClass::Set_Sound(&_definition);
            if (auto *sound = Peek_Sound()) {
                _volume = sound->Get_Volume();
            }
        }
    }

    void On_Frame_Update() override
    {
        if (auto *sound = Peek_Sound(); sound && sound->As_Sound3DClass()) {
            // Sound3D's edge attenuation reduces the runtime volume but does
            // not restore it outside the edge band. Reset before audio updates.
            sound->Set_Volume(_volume);
        }
        SoundRenderObjClass::On_Frame_Update();
    }

private:
    AudibleSoundDefinitionClass _definition;
    float _volume = 1.0f;
};

class ViewerSoundPrototype final : public SoundRenderObjPrototypeClass
{
public:
    using SoundRenderObjPrototypeClass::SoundRenderObjPrototypeClass;

    RenderObjClass *Create() override
    {
        auto *source = static_cast<SoundRenderObjClass *>(SoundRenderObjPrototypeClass::Create());
        if (!source) {
            return nullptr;
        }
        auto *object = new ViewerSoundObject(*source);
        source->Release_Ref();
        return object;
    }
};

class ViewerSoundLoader final : public SoundRenderObjLoaderClass
{
public:
    PrototypeClass *Load_W3D(ChunkLoadClass &load) override
    {
        auto *definition = new SoundRenderObjDefClass;
        PrototypeClass *prototype = nullptr;
        if (definition->Load_W3D(load) == WW3D_ERROR_OK) {
            prototype = new ViewerSoundPrototype(definition);
        }
        definition->Release_Ref();
        return prototype;
    }
};

ViewerSoundLoader soundLoader;

bool SetScreenSize(Chunk &array, float screenSize)
{
    QList<Chunk> children;
    if (!ReadChunks(array.data, children) || children.isEmpty() ||
        children.first().id != W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER ||
        children.first().data.size() != sizeof(W3dHLodArrayHeaderStruct)) {
        return false;
    }
    W3dHLodArrayHeaderStruct header;
    std::memcpy(&header, children.first().data.constData(), sizeof(header));
    header.MaxScreenSize = screenSize;
    children.first() = DataChunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, header);
    array.data = EncodeChunks(children);
    return true;
}

Chunk ModelArray(HLodClass &object, int level)
{
    const bool additional = level < 0;
    W3dHLodArrayHeaderStruct header{};
    header.ModelCount = additional ? object.Get_Additional_Model_Count()
                                   : object.Get_Lod_Model_Count(level);
    header.MaxScreenSize = additional ? 0.0f : object.Get_Max_Screen_Size(level);
    QList<Chunk> children{DataChunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, header)};
    for (uint32 index = 0; index < header.ModelCount; ++index) {
        RenderObjClass *model = additional ? object.Peek_Additional_Model(index)
                                          : object.Peek_Lod_Model(level, index);
        W3dHLodSubObjectStruct subobject{};
        subobject.BoneIndex = additional ? object.Get_Additional_Model_Bone(index)
                                        : object.Get_Lod_Model_Bone(level, index);
        if (model && model->Get_Name()) {
            std::strncpy(subobject.Name, model->Get_Name(), sizeof(subobject.Name) - 1);
        }
        children.append(DataChunk(W3D_CHUNK_HLOD_SUB_OBJECT, subobject));
    }
    return {static_cast<uint32>(additional ? W3D_CHUNK_HLOD_AGGREGATE_ARRAY : W3D_CHUNK_HLOD_LOD_ARRAY),
            EncodeChunks(children), true};
}
} // namespace

ViewerAssetManager::ViewerAssetManager()
{
    for (int index = 0; index < PrototypeLoaders.Count(); ++index) {
        if (PrototypeLoaders[index]->Chunk_Type() == W3D_CHUNK_HLOD) {
            PrototypeLoaders[index] = &hlodLoader;
            break;
        }
    }
    // Emitters are optional loaders and are not installed by the engine constructor.
    Register_Prototype_Loader(&emitterLoader);
    Register_Prototype_Loader(&soundLoader);
}

SoundRenderObjClass *CreateViewerSoundObject(const SoundRenderObjClass *source)
{
    return source ? new ViewerSoundObject(*source) : new ViewerSoundObject;
}

SoundRenderObjPrototypeClass *CreateViewerSoundPrototype(SoundRenderObjDefClass *definition)
{
    return new ViewerSoundPrototype(definition);
}

bool SaveViewerEmitter(ChunkSaveClass &save, const ParticleEmitterDefClass &definition)
{
    ViewerEmitterDefinition copy(definition);
    return copy.Save_W3D(save) == WW3D_ERROR_OK;
}

bool SaveViewerHlod(ChunkSaveClass &save, HLodPrototypeClass &prototype)
{
    if (auto *viewerPrototype = dynamic_cast<ViewerHlodPrototype *>(&prototype)) {
        if (!save.Begin_Chunk(W3D_CHUNK_HLOD)) {
            return false;
        }
        const Chunk &header = viewerPrototype->chunks.first();
        if (!save.Begin_Chunk(header.id)) {
            save.End_Chunk();
            return false;
        }
        const bool wroteHeader = save.Write(header.data.constData(), header.data.size()) ==
            header.data.size();
        const bool endedHeader = save.End_Chunk();
        // Opening the header marks the HLOD as a container. Copy the remaining
        // encoded chunks verbatim, including unknown and empty container chunks.
        const QByteArray arrays = EncodeChunks(viewerPrototype->chunks.sliced(1));
        const bool wroteArrays = save.Write(arrays.constData(), arrays.size()) == arrays.size();
        const bool ended = save.End_Chunk();
        return wroteHeader && endedHeader && wroteArrays && ended;
    }
    return prototype.Get_Definition()->Save(save) == WW3D_ERROR_OK;
}

HLodPrototypeClass *CreateViewerHlodPrototype(
    HLodClass &object, const HLodPrototypeClass *original)
{
    const int count = object.Get_LOD_Count();
    if (count <= 0) {
        return nullptr;
    }
    QList<Chunk> chunks;
    if (const auto *source = dynamic_cast<const ViewerHlodPrototype *>(original)) {
        chunks = source->chunks;
        W3dHLodHeaderStruct header;
        std::memcpy(&header, chunks.first().data.constData(), sizeof(header));
        // Viewer HLOD editing changes screen thresholds and the optional first NULL level.
        // Keep original model references too: unresolved assets must survive editing.
        const int difference = count - static_cast<int>(header.LodCount);
        if (difference == 1 && object.Is_NULL_Lod_Included()) {
            chunks.insert(1, ModelArray(object, 0));
        } else if (difference == -1 && !object.Is_NULL_Lod_Included()) {
            chunks.removeAt(1);
        } else if (difference != 0) {
            return nullptr;
        }
        header.LodCount = count;
        chunks.first() = DataChunk(W3D_CHUNK_HLOD_HEADER, header);
        for (int level = 0; level < count; ++level) {
            if (!SetScreenSize(chunks[level + 1], object.Get_Max_Screen_Size(level))) {
                return nullptr;
            }
        }
    } else {
        // New and converted models have no file-only proxy metadata.
        if (object.Get_Proxy_Count() != 0) {
            return nullptr;
        }
        W3dHLodHeaderStruct header{};
        header.Version = W3D_CURRENT_HLOD_VERSION;
        header.LodCount = count;
        std::strncpy(header.Name, object.Get_Name(), sizeof(header.Name) - 1);
        if (const HTreeClass *tree = object.Get_HTree()) {
            std::strncpy(header.HierarchyName, tree->Get_Name(), sizeof(header.HierarchyName) - 1);
        }
        chunks.append(DataChunk(W3D_CHUNK_HLOD_HEADER, header));
        for (int level = 0; level < count; ++level) {
            chunks.append(ModelArray(object, level));
        }
        if (object.Get_Additional_Model_Count() > 0) {
            chunks.append(ModelArray(object, -1));
        }
    }
    return LoadHlod(std::move(chunks));
}
