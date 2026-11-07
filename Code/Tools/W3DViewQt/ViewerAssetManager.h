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

#include "assetmgr.h"

class ChunkSaveClass;
class HLodClass;
class HLodPrototypeClass;
class ParticleEmitterDefClass;
class SoundRenderObjClass;
class SoundRenderObjDefClass;
class SoundRenderObjPrototypeClass;

// Viewer loaders preserve HLOD metadata and load the emitter line-properties chunk.
// On-demand dependencies go through the same loaders.
class ViewerAssetManager final : public WW3DAssetManager
{
public:
    ViewerAssetManager();
};

SoundRenderObjClass *CreateViewerSoundObject(const SoundRenderObjClass *source = nullptr);
SoundRenderObjPrototypeClass *CreateViewerSoundPrototype(SoundRenderObjDefClass *definition);

bool SaveViewerEmitter(ChunkSaveClass &save, const ParticleEmitterDefClass &definition);
bool SaveViewerHlod(ChunkSaveClass &save, HLodPrototypeClass &prototype);
HLodPrototypeClass *CreateViewerHlodPrototype(
    HLodClass &object, const HLodPrototypeClass *original = nullptr);
