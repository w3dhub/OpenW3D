#include "WWConfigSettings.h"

#include "ini.h"
#include "openw3d.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <string>
#include <utility>

namespace
{
// Keep these value names aligned with the legacy DirectX wrapper INI schema.
constexpr const char *kIniRenderDeviceName = "Name";
constexpr const char *kIniRenderDeviceWidth = "Width";
constexpr const char *kIniRenderDeviceHeight = "Height";
constexpr const char *kIniRenderDeviceDepth = "Depth";
constexpr const char *kIniRenderDeviceWindowed = "Windowed";
constexpr const char *kIniRenderDeviceTextureDepth = "TextureDepth";

constexpr const char *kIniValueNameDynLOD = "DynamicLODBudget";
constexpr const char *kIniValueNameStaticLOD = "StaticLODBudget";
constexpr const char *kIniValueNameDynShadows = "DynamicProjectors";
constexpr const char *kIniValueNameTextureFilter = "TextureFilterMode";
constexpr const char *kIniValueNamePrelitMode = "PrelitMode";
constexpr const char *kIniValueNameShadowMode = "ShadowMode";
constexpr const char *kIniValueNameStaticShadows = "StaticProjectors";
constexpr const char *kIniValueNameTextureRes = "TextureResolution";
constexpr const char *kIniValueNameSurfaceEffect = "SurfaceEffectDetail";
constexpr const char *kIniValueNameParticleDetail = "ParticleDetail";

constexpr const char *kIniValueNameAudioStereo = "Stereo";
constexpr const char *kIniValueNameAudioBits = "Bits";
constexpr const char *kIniValueNameAudioHertz = "Hertz";
constexpr const char *kIniValueNameAudioDevice = "DeviceName";
constexpr const char *kIniValueNameAudioMusicEnabled = "MusicEnabled";
constexpr const char *kIniValueNameAudioSoundEnabled = "SoundEnabled";
constexpr const char *kIniValueNameAudioDialogEnabled = "DialogEnabled";
constexpr const char *kIniValueNameAudioCinematicEnabled = "CinematicEnabled";
constexpr const char *kIniValueNameAudioMusicVolume = "MusicVolume";
constexpr const char *kIniValueNameAudioSoundVolume = "SoundVolume";
constexpr const char *kIniValueNameAudioDialogVolume = "DialogVolume";
constexpr const char *kIniValueNameAudioCinematicVolume = "CinematicVolume";
constexpr const char *kIniValueNameAudioSpeakerType = "SpeakerType";

constexpr const char *kValueNameDriverWarningDisabled = "DriverVersionCheckDisabled";

bool SaveIni(INIClass &ini)
{
    const std::string path = WWConfig::GetConfigFilePath();
    return ini.Save(path.c_str()) >= 0;
}

bool LoadRenderSettingsFromIni(INIClass &ini, RenderSettings &settings)
{
    if (!ini.Is_Loaded() || !ini.Section_Present(W3D_SECTION_SYSTEM)) {
        return false;
    }

    settings.dynamicLOD = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameDynLOD, settings.dynamicLOD);
    settings.staticLOD = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameStaticLOD, settings.staticLOD);
    settings.dynamicShadows = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameDynShadows, settings.dynamicShadows);
    settings.staticShadows = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameStaticShadows, settings.staticShadows);
    settings.prelitMode = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNamePrelitMode, settings.prelitMode);
    settings.textureFilter = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameTextureFilter, settings.textureFilter);
    settings.shadowMode = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameShadowMode, settings.shadowMode);
    settings.textureResolution = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameTextureRes, settings.textureResolution);
    settings.surfaceEffect = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameSurfaceEffect, settings.surfaceEffect);
    settings.particleDetail = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNameParticleDetail, settings.particleDetail);
    settings.lightingMode = ini.Get_Int(W3D_SECTION_SYSTEM, kIniValueNamePrelitMode, settings.lightingMode);

    return true;
}

void SaveRenderSettingsToIni(const RenderSettings &settings, INIClass &ini)
{
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameDynLOD, settings.dynamicLOD);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameStaticLOD, settings.staticLOD);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameDynShadows, settings.dynamicShadows);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameStaticShadows, settings.staticShadows);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNamePrelitMode, settings.prelitMode);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameTextureFilter, settings.textureFilter);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameShadowMode, settings.shadowMode);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameTextureRes, settings.textureResolution);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameSurfaceEffect, settings.surfaceEffect);
    ini.Put_Int(W3D_SECTION_SYSTEM, kIniValueNameParticleDetail, settings.particleDetail);
}

bool LoadVideoSettingsFromIni(INIClass &ini, VideoSettings &settings)
{
    if (!ini.Is_Loaded() || !ini.Section_Present(W3D_SECTION_RENDER)) {
        return false;
    }

    char deviceName[256] = {};
    ini.Get_String(W3D_SECTION_RENDER, kIniRenderDeviceName, "", deviceName, sizeof(deviceName));
    if (deviceName[0] != '\0') {
        settings.deviceName = deviceName;
    }

    const int width = ini.Get_Int(W3D_SECTION_RENDER, kIniRenderDeviceWidth, -1);
    const int height = ini.Get_Int(W3D_SECTION_RENDER, kIniRenderDeviceHeight, -1);
    const int bitDepth = ini.Get_Int(W3D_SECTION_RENDER, kIniRenderDeviceDepth, -1);
    const int windowed = ini.Get_Int(W3D_SECTION_RENDER, kIniRenderDeviceWindowed, settings.windowed ? 1 : 0);
    const int textureDepth = ini.Get_Int(W3D_SECTION_RENDER, kIniRenderDeviceTextureDepth, -1);

    if (width > 0) {
        settings.width = width;
    }
    if (height > 0) {
        settings.height = height;
    }
    if (bitDepth > 0) {
        settings.bitDepth = bitDepth;
    }
    settings.windowed = windowed != 0;
    if (textureDepth > 0) {
        settings.textureDepth = textureDepth;
    }

    return true;
}

void SaveVideoSettingsToIni(const VideoSettings &settings, INIClass &ini)
{
    ini.Put_String(W3D_SECTION_RENDER, kIniRenderDeviceName, settings.deviceName.c_str());
    ini.Put_Int(W3D_SECTION_RENDER, kIniRenderDeviceWidth, settings.width);
    ini.Put_Int(W3D_SECTION_RENDER, kIniRenderDeviceHeight, settings.height);
    ini.Put_Int(W3D_SECTION_RENDER, kIniRenderDeviceDepth, settings.bitDepth);
    ini.Put_Int(W3D_SECTION_RENDER, kIniRenderDeviceWindowed, settings.windowed ? 1 : 0);
    ini.Put_Int(W3D_SECTION_RENDER, kIniRenderDeviceTextureDepth, settings.textureDepth);
}

bool LoadAudioSettingsFromIni(INIClass &ini, AudioSettings &settings)
{
    if (!ini.Is_Loaded() || !ini.Section_Present(W3D_SECTION_SOUND)) {
        return false;
    }

    char deviceName[256] = {};
    ini.Get_String(W3D_SECTION_SOUND, kIniValueNameAudioDevice, "", deviceName, sizeof(deviceName));
    if (deviceName[0] != '\0') {
        settings.deviceName = deviceName;
    }

    settings.stereo = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioStereo, settings.stereo ? 1 : 0) != 0;
    settings.bitDepth = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioBits, settings.bitDepth);
    settings.sampleRate = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioHertz, settings.sampleRate);
    settings.musicEnabled = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioMusicEnabled, settings.musicEnabled ? 1 : 0) != 0;
    settings.soundEnabled = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioSoundEnabled, settings.soundEnabled ? 1 : 0) != 0;
    settings.dialogEnabled = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioDialogEnabled, settings.dialogEnabled ? 1 : 0) != 0;
    settings.cinematicEnabled = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioCinematicEnabled, settings.cinematicEnabled ? 1 : 0) != 0;
    settings.soundVolume = std::clamp(ini.Get_Float(W3D_SECTION_SOUND, kIniValueNameAudioSoundVolume, settings.soundVolume), 0.0f, 1.0f);
    settings.musicVolume = std::clamp(ini.Get_Float(W3D_SECTION_SOUND, kIniValueNameAudioMusicVolume, settings.musicVolume), 0.0f, 1.0f);
    settings.dialogVolume = std::clamp(ini.Get_Float(W3D_SECTION_SOUND, kIniValueNameAudioDialogVolume, settings.dialogVolume), 0.0f, 1.0f);
    settings.cinematicVolume = std::clamp(ini.Get_Float(W3D_SECTION_SOUND, kIniValueNameAudioCinematicVolume, settings.cinematicVolume), 0.0f, 1.0f);
    settings.speakerType = ini.Get_Int(W3D_SECTION_SOUND, kIniValueNameAudioSpeakerType, settings.speakerType);

    return true;
}

void SaveAudioSettingsToIni(const AudioSettings &settings, INIClass &ini)
{
    ini.Put_String(W3D_SECTION_SOUND, kIniValueNameAudioDevice, settings.deviceName.c_str());
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioStereo, settings.stereo ? 1 : 0);
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioBits, settings.bitDepth);
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioHertz, settings.sampleRate);
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioMusicEnabled, settings.musicEnabled ? 1 : 0);
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioSoundEnabled, settings.soundEnabled ? 1 : 0);
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioDialogEnabled, settings.dialogEnabled ? 1 : 0);
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioCinematicEnabled, settings.cinematicEnabled ? 1 : 0);
    ini.Put_Float(W3D_SECTION_SOUND, kIniValueNameAudioSoundVolume, std::clamp(settings.soundVolume, 0.0f, 1.0f));
    ini.Put_Float(W3D_SECTION_SOUND, kIniValueNameAudioMusicVolume, std::clamp(settings.musicVolume, 0.0f, 1.0f));
    ini.Put_Float(W3D_SECTION_SOUND, kIniValueNameAudioDialogVolume, std::clamp(settings.dialogVolume, 0.0f, 1.0f));
    ini.Put_Float(W3D_SECTION_SOUND, kIniValueNameAudioCinematicVolume, std::clamp(settings.cinematicVolume, 0.0f, 1.0f));
    ini.Put_Int(W3D_SECTION_SOUND, kIniValueNameAudioSpeakerType, settings.speakerType);
}

void AppendResolutionModes(VideoAdapterInfo &adapter, int bitDepth)
{
    constexpr std::array<std::pair<int, int>, 10> kModes = {
        std::pair{640, 480},
        std::pair{800, 600},
        std::pair{1024, 768},
        std::pair{1152, 864},
        std::pair{1280, 720},
        std::pair{1280, 800},
        std::pair{1366, 768},
        std::pair{1440, 900},
        std::pair{1600, 900},
        std::pair{1920, 1080},
    };

    for (const auto &[width, height] : kModes) {
        VideoResolution mode;
        mode.width = width;
        mode.height = height;
        mode.bitDepth = bitDepth;
        adapter.resolutions.push_back(mode);
    }
}
} // namespace

namespace WWConfig
{
std::string GetConfigFilePath()
{
    if (const char *envPath = std::getenv("OPENW3D_CONFIG_INI")) {
        if (*envPath != '\0') {
            return std::string(envPath);
        }
    }

    if (const char *envPath = std::getenv("RENEGADE_CONFIG_INI")) {
        if (*envPath != '\0') {
            return std::string(envPath);
        }
    }

    return std::string(W3D_CONF_FILE);
}

bool LoadRenderSettings(RenderSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    return LoadRenderSettingsFromIni(ini, settings);
}

bool SaveRenderSettings(const RenderSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    SaveRenderSettingsToIni(settings, ini);
    return SaveIni(ini);
}

void AutoConfigRenderSettings()
{
    RenderSettings settings;
    LoadRenderSettings(settings);
    SaveRenderSettings(settings);
}

bool LoadVideoSettings(VideoSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    const bool loaded = LoadVideoSettingsFromIni(ini, settings);
    if (settings.deviceName.empty()) {
        settings.deviceName = "OpenW3D";
    }
    return loaded;
}

bool SaveVideoSettings(const VideoSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    SaveVideoSettingsToIni(settings, ini);
    return SaveIni(ini);
}

bool LoadAudioSettings(AudioSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    return LoadAudioSettingsFromIni(ini, settings);
}

bool SaveAudioSettings(const AudioSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    SaveAudioSettingsToIni(settings, ini);
    return SaveIni(ini);
}

bool IsDriverWarningDisabled()
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    const int disabled = ini.Get_Int(W3D_SECTION_RENDER, kValueNameDriverWarningDisabled, 0);
    return disabled >= 87;
}

void SetDriverWarningDisabled(bool disabled)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    ini.Put_Int(W3D_SECTION_RENDER, kValueNameDriverWarningDisabled, disabled ? 87 : 0);
    SaveIni(ini);
}

bool EnumerateVideoAdapters(std::vector<VideoAdapterInfo> &adapters)
{
    adapters.clear();

    VideoAdapterInfo adapter;
    adapter.deviceName = "OpenW3D";
    adapter.description = "OpenW3D Default Adapter";
    AppendResolutionModes(adapter, 16);
    AppendResolutionModes(adapter, 32);

    adapters.push_back(std::move(adapter));
    return true;
}
} // namespace WWConfig
