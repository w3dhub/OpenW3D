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
