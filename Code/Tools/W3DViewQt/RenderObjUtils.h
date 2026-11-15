/*
** Copyright 2026 OpenW3D contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed WITHOUT ANY WARRANTY; without even the
** implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <QStringList>

class RenderObjClass;
class HLodClass;
class RingRenderObjClass;
class SoundRenderObjClass;
class SphereRenderObjClass;

bool UpdateLodPrototype(HLodClass &hlod);
QString AggregateBaseModelName(const RenderObjClass &render_obj);
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
