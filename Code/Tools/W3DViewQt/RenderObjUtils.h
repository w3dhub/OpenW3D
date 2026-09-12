#pragma once

#include <QStringList>

class RenderObjClass;
class HLodClass;
class RingRenderObjClass;
class SoundRenderObjClass;
class SphereRenderObjClass;

void UpdateLodPrototype(HLodClass &hlod);
void UpdateAggregatePrototype(RenderObjClass &render_obj);
bool RenameAggregatePrototype(const char *old_name, const char *new_name);
bool UpdateSpherePrototype(SphereRenderObjClass &sphere,
                           const QString &registered_name,
                           QString *error_message = nullptr);
bool UpdateRingPrototype(RingRenderObjClass &ring,
                         const QString &registered_name,
                         QString *error_message = nullptr);
bool UpdateSoundPrototype(SoundRenderObjClass &sound,
                          const QString &registered_name,
                          QString *error_message = nullptr);
void CollectEmitterNames(RenderObjClass &render_obj, QStringList &names);
int CountParticles(RenderObjClass *render_obj);
