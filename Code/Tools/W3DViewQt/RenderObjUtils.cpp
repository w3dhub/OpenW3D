#include "RenderObjUtils.h"

#include "agg_def.h"
#include "assetmgr.h"
#include "hlod.h"
#include "part_emt.h"
#include "rendobj.h"
#include "ringobj.h"
#include "soundrobj.h"
#include "sphereobj.h"

#include <QByteArray>
#include <QString>

#include <memory>

void UpdateLodPrototype(HLodClass &hlod)
{
    auto *definition = new HLodDefClass(hlod);
    auto *prototype = new HLodPrototypeClass(definition);

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        delete prototype;
        return;
    }

    asset_manager->Remove_Prototype(definition->Get_Name());
    asset_manager->Add_Prototype(prototype);
}

void UpdateAggregatePrototype(RenderObjClass &render_obj)
{
    auto *definition = new AggregateDefClass(render_obj);
    auto *prototype = new AggregatePrototypeClass(definition);

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        delete prototype;
        return;
    }

    asset_manager->Remove_Prototype(definition->Get_Name());
    asset_manager->Add_Prototype(prototype);
}

bool RenameAggregatePrototype(const char *old_name, const char *new_name)
{
    if (!old_name || !new_name) {
        return false;
    }

    const QString old_text = QString::fromLatin1(old_name);
    const QString new_text = QString::fromLatin1(new_name);
    if (old_text.compare(new_text, Qt::CaseInsensitive) == 0) {
        return false;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return false;
    }

    auto *proto = static_cast<AggregatePrototypeClass *>(asset_manager->Find_Prototype(old_name));
    if (!proto) {
        return false;
    }

    AggregateDefClass *definition = proto->Get_Definition();
    if (!definition) {
        return false;
    }

    AggregateDefClass *new_definition = definition->Clone();
    asset_manager->Remove_Prototype(old_name);

    new_definition->Set_Name(new_name);
    auto *new_proto = new AggregatePrototypeClass(new_definition);
    asset_manager->Add_Prototype(new_proto);
    return true;
}

bool UpdateSpherePrototype(SphereRenderObjClass &sphere,
                           const QString &registered_name,
                           QString *error_message)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        if (error_message) {
            *error_message = "WW3D asset manager is not available.";
        }
        return false;
    }

    const char *object_name = sphere.Get_Name();
    const QString new_name = object_name ? QString::fromLatin1(object_name).trimmed() : QString();
    if (new_name.isEmpty()) {
        if (error_message) {
            *error_message = "Sphere name is required.";
        }
        return false;
    }

    const QByteArray new_name_bytes = new_name.toLatin1();
    const bool replaces_registered_name = !registered_name.isEmpty() &&
        registered_name.compare(new_name, Qt::CaseInsensitive) == 0;
    if (asset_manager->Find_Prototype(new_name_bytes.constData()) && !replaces_registered_name) {
        if (error_message) {
            *error_message = QString("An asset named '%1' already exists.").arg(new_name);
        }
        return false;
    }

    auto prototype = std::make_unique<SpherePrototypeClass>(&sphere);
    if (!registered_name.isEmpty()) {
        const QByteArray registered_name_bytes = registered_name.toLatin1();
        asset_manager->Remove_Prototype(registered_name_bytes.constData());
    }
    asset_manager->Add_Prototype(prototype.release());
    return true;
}

bool UpdateRingPrototype(RingRenderObjClass &ring,
                         const QString &registered_name,
                         QString *error_message)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        if (error_message) {
            *error_message = "WW3D asset manager is not available.";
        }
        return false;
    }

    const char *object_name = ring.Get_Name();
    const QString new_name = object_name ? QString::fromLatin1(object_name).trimmed() : QString();
    if (new_name.isEmpty()) {
        if (error_message) {
            *error_message = "Ring name is required.";
        }
        return false;
    }

    const QByteArray new_name_bytes = new_name.toLatin1();
    const bool replaces_registered_name = !registered_name.isEmpty() &&
        registered_name.compare(new_name, Qt::CaseInsensitive) == 0;
    if (asset_manager->Find_Prototype(new_name_bytes.constData()) && !replaces_registered_name) {
        if (error_message) {
            *error_message = QString("An asset named '%1' already exists.").arg(new_name);
        }
        return false;
    }

    auto prototype = std::make_unique<RingPrototypeClass>(&ring);
    if (!registered_name.isEmpty()) {
        const QByteArray registered_name_bytes = registered_name.toLatin1();
        asset_manager->Remove_Prototype(registered_name_bytes.constData());
    }
    asset_manager->Add_Prototype(prototype.release());
    return true;
}

bool UpdateSoundPrototype(SoundRenderObjClass &sound,
                          const QString &registered_name,
                          QString *error_message)
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        if (error_message) {
            *error_message = "WW3D asset manager is not available.";
        }
        return false;
    }

    const char *object_name = sound.Get_Name();
    const QString new_name = object_name ? QString::fromLatin1(object_name).trimmed() : QString();
    if (new_name.isEmpty()) {
        if (error_message) {
            *error_message = "Sound object name is required.";
        }
        return false;
    }

    const QByteArray new_name_bytes = new_name.toLatin1();
    const bool replaces_registered_name = !registered_name.isEmpty()
        && registered_name.compare(new_name, Qt::CaseInsensitive) == 0;
    if (asset_manager->Find_Prototype(new_name_bytes.constData()) && !replaces_registered_name) {
        if (error_message) {
            *error_message = QString("An asset named '%1' already exists.").arg(new_name);
        }
        return false;
    }

    auto *definition = new SoundRenderObjDefClass(sound);
    auto prototype = std::make_unique<SoundRenderObjPrototypeClass>(definition);
    // The prototype retains its own reference to the definition.
    definition->Release_Ref();

    if (!registered_name.isEmpty()) {
        const QByteArray registered_name_bytes = registered_name.toLatin1();
        asset_manager->Remove_Prototype(registered_name_bytes.constData());
    }
    asset_manager->Add_Prototype(prototype.release());
    return true;
}

namespace {
bool ContainsName(const QStringList &names, const QString &candidate)
{
    for (const auto &name : names) {
        if (name.compare(candidate, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}
} // namespace

void CollectEmitterNames(RenderObjClass &render_obj, QStringList &names)
{
    const int count = render_obj.Get_Num_Sub_Objects();
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = render_obj.Get_Sub_Object(index);
        if (!sub_obj) {
            continue;
        }

        if (sub_obj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
            const char *name = sub_obj->Get_Name();
            if (name) {
                const QString text = QString::fromLatin1(name);
                if (!text.isEmpty() && !ContainsName(names, text)) {
                    names.push_back(text);
                }
            }
        }

        CollectEmitterNames(*sub_obj, names);
        sub_obj->Release_Ref();
    }
}

int CountParticles(RenderObjClass *render_obj)
{
    if (!render_obj) {
        return 0;
    }

    int count = 0;
    const int sub_count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < sub_count; ++index) {
        RenderObjClass *sub_obj = render_obj->Get_Sub_Object(index);
        if (sub_obj) {
            count += CountParticles(sub_obj);
            sub_obj->Release_Ref();
        }
    }

    if (render_obj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
        auto *emitter = static_cast<ParticleEmitterClass *>(render_obj);
        ParticleBufferClass *buffer = emitter->Peek_Buffer();
        if (buffer) {
            count += buffer->Get_Particle_Count();
        }
    }

    return count;
}
