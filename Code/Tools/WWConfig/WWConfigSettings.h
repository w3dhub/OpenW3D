#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct RenderSettings
{
    int dynamicLOD = 3000;
    int staticLOD = 3000;
    int dynamicShadows = 1;
    int staticShadows = 1;
    int prelitMode = 0;
    int textureFilter = 0;
    int shadowMode = 0;
    int textureResolution = 0;
    int surfaceEffect = 1;
    int particleDetail = 1;
    int lightingMode = 0;
};

struct VideoSettings
{
    std::string deviceName;
    int width = 800;
    int height = 600;
    int bitDepth = 32;
    bool windowed = false;
    int textureDepth = 32;
};

struct AudioSettings
{
    std::string deviceName;
    bool stereo = true;
    int bitDepth = 16;
    int sampleRate = 44100;
    bool soundEnabled = true;
    bool musicEnabled = true;
    bool dialogEnabled = true;
    bool cinematicEnabled = true;
    float soundVolume = 1.0f;
    float musicVolume = 1.0f;
    float dialogVolume = 1.0f;
    float cinematicVolume = 1.0f;
    int speakerType = 0;
};

struct VideoResolution
{
    int width = 0;
    int height = 0;
    int bitDepth = 0;
};

struct VideoAdapterInfo
{
    std::string deviceName;
    std::string description;
    std::vector<VideoResolution> resolutions;
};

namespace WWConfig
{
bool LoadRenderSettings(RenderSettings &settings);
bool SaveRenderSettings(const RenderSettings &settings);
void AutoConfigRenderSettings();

bool LoadVideoSettings(VideoSettings &settings);
bool SaveVideoSettings(const VideoSettings &settings);

bool LoadAudioSettings(AudioSettings &settings);
bool SaveAudioSettings(const AudioSettings &settings);

bool EnumerateVideoAdapters(std::vector<VideoAdapterInfo> &adapters);

// Path to the ini used by the Qt port.
std::string GetConfigFilePath();
bool IsDriverWarningDisabled();
void SetDriverWarningDisabled(bool disabled);
}
