#include "WWConfigSettings.h"

#include "ini.h"
#include "registry.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "cpudetect.h"
#include "formconv.h"
#include "wwstring.h"

#include "../../Combat/specialbuilds.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <d3d9.h>

#ifndef RENEGADE_SUB_KEY_NAME_RENDER
#if defined(FREEDEDICATEDSERVER)
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeFDS\\Render"
#elif defined(MULTIPLAYERDEMO)
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeMPDemo\\Render"
#elif defined(BETACLIENT)
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeBeta\\Render"
#else
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\Renegade\\Render"
#endif
#endif

#ifndef RENEGADE_SUB_KEY_NAME_AUDIO
#if defined(FREEDEDICATEDSERVER)
#define RENEGADE_SUB_KEY_NAME_AUDIO "Software\\Westwood\\RenegadeFDS\\Sound"
#elif defined(MULTIPLAYERDEMO)
#define RENEGADE_SUB_KEY_NAME_AUDIO "Software\\Westwood\\RenegadeMPDemo\\Sound"
#elif defined(BETACLIENT)
#define RENEGADE_SUB_KEY_NAME_AUDIO "Software\\Westwood\\RenegadeBeta\\Sound"
#else
#define RENEGADE_SUB_KEY_NAME_AUDIO "Software\\Westwood\\Renegade\\Sound"
#endif
#endif

namespace
{
#if	defined(FREEDEDICATEDSERVER)
constexpr const char *kKeyNameSettings = "Software\\Westwood\\RenegadeFDS\\System Settings";
constexpr const char *kKeyNameOptions = "Software\\Westwood\\RenegadeFDS\\Options";
#elif defined(MULTIPLAYERDEMO)
constexpr const char *kKeyNameSettings = "Software\\Westwood\\RenegadeMPDemo\\System Settings";
constexpr const char *kKeyNameOptions = "Software\\Westwood\\RenegadeMPDemo\\Options";
#elif defined(BETACLIENT)
constexpr const char *kKeyNameSettings = "Software\\Westwood\\RenegadeBeta\\System Settings";
constexpr const char *kKeyNameOptions = "Software\\Westwood\\RenegadeBeta\\Options";
#else
constexpr const char *kKeyNameSettings = "Software\\Westwood\\Renegade\\System Settings";
constexpr const char *kKeyNameOptions = "Software\\Westwood\\Renegade\\Options";
#endif

constexpr const char *kValueNameDynLOD = "Dynamic_LOD_Budget";
constexpr const char *kValueNameStaticLOD = "Static_LOD_Budget";
constexpr const char *kValueNameDynShadows = "Dynamic_Projectors";
constexpr const char *kValueNameTextureFilter = "Texture_Filter_Mode";
constexpr const char *kValueNamePrelitMode = "Prelit_Mode";
constexpr const char *kValueNameShadowMode = "Shadow_Mode";
constexpr const char *kValueNameStaticShadows = "Static_Projectors";
constexpr const char *kValueNameTextureRes = "Texture_Resolution";
constexpr const char *kValueNameSurfaceEffect = "Surface_Effect_Detail";
constexpr const char *kValueNameParticleDetail = "Particle_Detail";

constexpr const char *kValueNameAudioStereo = "stereo";
constexpr const char *kValueNameAudioBits = "bits";
constexpr const char *kValueNameAudioHertz = "hertz";
constexpr const char *kValueNameAudioDevice = "device name";
constexpr const char *kValueNameAudioMusicEnabled = "music enabled";
constexpr const char *kValueNameAudioSoundEnabled = "sound enabled";
constexpr const char *kValueNameAudioDialogEnabled = "dialog enabled";
constexpr const char *kValueNameAudioCinematicEnabled = "cinematic enabled";
constexpr const char *kValueNameAudioMusicVolume = "music volume";
constexpr const char *kValueNameAudioSoundVolume = "sound volume";
constexpr const char *kValueNameAudioDialogVolume = "dialog volume";
constexpr const char *kValueNameAudioCinematicVolume = "cinematic volume";
constexpr const char *kValueNameAudioSpeakerType = "speaker type";
constexpr const char *kValueNameDriverWarningDisabled = "DriverVersionCheckDisabled";

constexpr int kVolumeScale = 100;

constexpr const char *kIniSectionRender = "Render";
constexpr const char *kIniSectionSound = "Sound";
constexpr const char *kIniSectionSettings = "System Settings";

bool SaveIni(INIClass &ini)
{
    const std::string path = WWConfig::GetConfigFilePath();
    return ini.Save(path.c_str()) >= 0;
}

bool LoadRenderSettingsFromIni(INIClass &ini, RenderSettings &settings)
{
    if (!ini.Is_Loaded() || !ini.Section_Present(kIniSectionSettings)) {
        return false;
    }

    settings.dynamicLOD = ini.Get_Int(kIniSectionSettings, kValueNameDynLOD, settings.dynamicLOD);
    settings.staticLOD = ini.Get_Int(kIniSectionSettings, kValueNameStaticLOD, settings.staticLOD);
    settings.dynamicShadows = ini.Get_Int(kIniSectionSettings, kValueNameDynShadows, settings.dynamicShadows);
    settings.staticShadows = ini.Get_Int(kIniSectionSettings, kValueNameStaticShadows, settings.staticShadows);
    settings.prelitMode = ini.Get_Int(kIniSectionSettings, kValueNamePrelitMode, settings.prelitMode);
    settings.textureFilter = ini.Get_Int(kIniSectionSettings, kValueNameTextureFilter, settings.textureFilter);
    settings.shadowMode = ini.Get_Int(kIniSectionSettings, kValueNameShadowMode, settings.shadowMode);
    settings.textureResolution = ini.Get_Int(kIniSectionSettings, kValueNameTextureRes, settings.textureResolution);
    settings.surfaceEffect = ini.Get_Int(kIniSectionSettings, kValueNameSurfaceEffect, settings.surfaceEffect);
    settings.particleDetail = ini.Get_Int(kIniSectionSettings, kValueNameParticleDetail, settings.particleDetail);
    settings.lightingMode = ini.Get_Int(kIniSectionSettings, kValueNamePrelitMode, settings.lightingMode);

    return true;
}

bool LoadRenderSettingsFromRegistry(RenderSettings &settings)
{
    RegistryClass registry(kKeyNameSettings);
    if (!registry.Is_Valid()) {
        return false;
    }

    settings.dynamicLOD = registry.Get_Int(kValueNameDynLOD, settings.dynamicLOD);
    settings.staticLOD = registry.Get_Int(kValueNameStaticLOD, settings.staticLOD);
    settings.dynamicShadows = registry.Get_Int(kValueNameDynShadows, settings.dynamicShadows);
    settings.staticShadows = registry.Get_Int(kValueNameStaticShadows, settings.staticShadows);
    settings.prelitMode = registry.Get_Int(kValueNamePrelitMode, settings.prelitMode);
    settings.textureFilter = registry.Get_Int(kValueNameTextureFilter, settings.textureFilter);
    settings.shadowMode = registry.Get_Int(kValueNameShadowMode, settings.shadowMode);
    settings.textureResolution = registry.Get_Int(kValueNameTextureRes, settings.textureResolution);
    settings.surfaceEffect = registry.Get_Int(kValueNameSurfaceEffect, settings.surfaceEffect);
    settings.particleDetail = registry.Get_Int(kValueNameParticleDetail, settings.particleDetail);
    settings.lightingMode = registry.Get_Int(kValueNamePrelitMode, settings.lightingMode);

    return true;
}

void SaveRenderSettingsToIni(const RenderSettings &settings, INIClass &ini)
{
    ini.Put_Int(kIniSectionSettings, kValueNameDynLOD, settings.dynamicLOD);
    ini.Put_Int(kIniSectionSettings, kValueNameStaticLOD, settings.staticLOD);
    ini.Put_Int(kIniSectionSettings, kValueNameDynShadows, settings.dynamicShadows);
    ini.Put_Int(kIniSectionSettings, kValueNameStaticShadows, settings.staticShadows);
    ini.Put_Int(kIniSectionSettings, kValueNamePrelitMode, settings.prelitMode);
    ini.Put_Int(kIniSectionSettings, kValueNameTextureFilter, settings.textureFilter);
    ini.Put_Int(kIniSectionSettings, kValueNameShadowMode, settings.shadowMode);
    ini.Put_Int(kIniSectionSettings, kValueNameTextureRes, settings.textureResolution);
    ini.Put_Int(kIniSectionSettings, kValueNameSurfaceEffect, settings.surfaceEffect);
    ini.Put_Int(kIniSectionSettings, kValueNameParticleDetail, settings.particleDetail);
}

bool SaveRenderSettingsToRegistry(const RenderSettings &settings)
{
    RegistryClass registry(kKeyNameSettings);
    if (!registry.Is_Valid()) {
        return false;
    }

    registry.Set_Int(kValueNameDynLOD, settings.dynamicLOD);
    registry.Set_Int(kValueNameStaticLOD, settings.staticLOD);
    registry.Set_Int(kValueNameDynShadows, settings.dynamicShadows);
    registry.Set_Int(kValueNameStaticShadows, settings.staticShadows);
    registry.Set_Int(kValueNamePrelitMode, settings.prelitMode);
    registry.Set_Int(kValueNameTextureFilter, settings.textureFilter);
    registry.Set_Int(kValueNameShadowMode, settings.shadowMode);
    registry.Set_Int(kValueNameTextureRes, settings.textureResolution);
    registry.Set_Int(kValueNameSurfaceEffect, settings.surfaceEffect);
    registry.Set_Int(kValueNameParticleDetail, settings.particleDetail);

    return true;
}

bool LoadVideoSettingsFromIni(INIClass &ini, VideoSettings &settings)
{
    if (!ini.Is_Loaded() || !ini.Section_Present(kIniSectionRender)) {
        return false;
    }

    char deviceName[256] = {};
    ini.Get_String(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_NAME, "", deviceName, sizeof(deviceName));
    if (deviceName[0] != '\0') {
        settings.deviceName = deviceName;
    }

    const int width = ini.Get_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_WIDTH, -1);
    const int height = ini.Get_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_HEIGHT, -1);
    const int bitDepth = ini.Get_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_DEPTH, -1);
    const int windowed = ini.Get_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_WINDOWED, settings.windowed ? 1 : 0);
    const int textureDepth = ini.Get_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, -1);

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

bool LoadVideoSettingsFromRegistry(VideoSettings &settings)
{
    RegistryClass registry(RENEGADE_SUB_KEY_NAME_RENDER);
    if (!registry.Is_Valid()) {
        return false;
    }

    char deviceName[256] = {};
    registry.Get_String(VALUE_NAME_RENDER_DEVICE_NAME, deviceName, sizeof(deviceName));
    if (deviceName[0] != '\0') {
        settings.deviceName = deviceName;
    }

    const int width = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_WIDTH, -1);
    const int height = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_HEIGHT, -1);
    const int bitDepth = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_DEPTH, -1);
    const int windowed = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_WINDOWED, settings.windowed ? 1 : 0);
    const int textureDepth = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, -1);

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
    ini.Put_String(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_NAME, settings.deviceName.c_str());
    ini.Put_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_WIDTH, settings.width);
    ini.Put_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_HEIGHT, settings.height);
    ini.Put_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_DEPTH, settings.bitDepth);
    ini.Put_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_WINDOWED, settings.windowed ? 1 : 0);
    ini.Put_Int(kIniSectionRender, VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, settings.textureDepth);
}

bool SaveVideoSettingsToRegistry(const VideoSettings &settings)
{
    RegistryClass registry(RENEGADE_SUB_KEY_NAME_RENDER);
    if (!registry.Is_Valid()) {
        return false;
    }

    registry.Set_String(VALUE_NAME_RENDER_DEVICE_NAME, settings.deviceName.c_str());
    registry.Set_Int(VALUE_NAME_RENDER_DEVICE_WIDTH, settings.width);
    registry.Set_Int(VALUE_NAME_RENDER_DEVICE_HEIGHT, settings.height);
    registry.Set_Int(VALUE_NAME_RENDER_DEVICE_DEPTH, settings.bitDepth);
    registry.Set_Int(VALUE_NAME_RENDER_DEVICE_WINDOWED, settings.windowed ? 1 : 0);
    registry.Set_Int(VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, settings.textureDepth);
    return true;
}

bool LoadAudioSettingsFromIni(INIClass &ini, AudioSettings &settings)
{
    if (!ini.Is_Loaded() || !ini.Section_Present(kIniSectionSound)) {
        return false;
    }

    char deviceName[256] = {};
    ini.Get_String(kIniSectionSound, kValueNameAudioDevice, "", deviceName, sizeof(deviceName));
    if (deviceName[0] != '\0') {
        settings.deviceName = deviceName;
    }

    settings.stereo = ini.Get_Int(kIniSectionSound, kValueNameAudioStereo, settings.stereo ? 1 : 0) != 0;
    settings.bitDepth = ini.Get_Int(kIniSectionSound, kValueNameAudioBits, settings.bitDepth);
    settings.sampleRate = ini.Get_Int(kIniSectionSound, kValueNameAudioHertz, settings.sampleRate);
    settings.musicEnabled = ini.Get_Int(kIniSectionSound, kValueNameAudioMusicEnabled, settings.musicEnabled ? 1 : 0) != 0;
    settings.soundEnabled = ini.Get_Int(kIniSectionSound, kValueNameAudioSoundEnabled, settings.soundEnabled ? 1 : 0) != 0;
    settings.dialogEnabled = ini.Get_Int(kIniSectionSound, kValueNameAudioDialogEnabled, settings.dialogEnabled ? 1 : 0) != 0;
    settings.cinematicEnabled = ini.Get_Int(kIniSectionSound, kValueNameAudioCinematicEnabled, settings.cinematicEnabled ? 1 : 0) != 0;
    settings.soundVolume = std::clamp(ini.Get_Int(kIniSectionSound, kValueNameAudioSoundVolume, static_cast<int>(settings.soundVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.musicVolume = std::clamp(ini.Get_Int(kIniSectionSound, kValueNameAudioMusicVolume, static_cast<int>(settings.musicVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.dialogVolume = std::clamp(ini.Get_Int(kIniSectionSound, kValueNameAudioDialogVolume, static_cast<int>(settings.dialogVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.cinematicVolume = std::clamp(ini.Get_Int(kIniSectionSound, kValueNameAudioCinematicVolume, static_cast<int>(settings.cinematicVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.speakerType = ini.Get_Int(kIniSectionSound, kValueNameAudioSpeakerType, settings.speakerType);

    return true;
}

bool LoadAudioSettingsFromRegistry(AudioSettings &settings)
{
    RegistryClass registry(RENEGADE_SUB_KEY_NAME_AUDIO);
    if (!registry.Is_Valid()) {
        return false;
    }

    char deviceName[256] = {};
    registry.Get_String(kValueNameAudioDevice, deviceName, sizeof(deviceName));
    if (deviceName[0] != '\0') {
        settings.deviceName = deviceName;
    }

    settings.stereo = registry.Get_Int(kValueNameAudioStereo, settings.stereo ? 1 : 0) != 0;
    settings.bitDepth = registry.Get_Int(kValueNameAudioBits, settings.bitDepth);
    settings.sampleRate = registry.Get_Int(kValueNameAudioHertz, settings.sampleRate);
    settings.musicEnabled = registry.Get_Int(kValueNameAudioMusicEnabled, settings.musicEnabled ? 1 : 0) != 0;
    settings.soundEnabled = registry.Get_Int(kValueNameAudioSoundEnabled, settings.soundEnabled ? 1 : 0) != 0;
    settings.dialogEnabled = registry.Get_Int(kValueNameAudioDialogEnabled, settings.dialogEnabled ? 1 : 0) != 0;
    settings.cinematicEnabled = registry.Get_Int(kValueNameAudioCinematicEnabled, settings.cinematicEnabled ? 1 : 0) != 0;
    settings.soundVolume = std::clamp(registry.Get_Int(kValueNameAudioSoundVolume, static_cast<int>(settings.soundVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.musicVolume = std::clamp(registry.Get_Int(kValueNameAudioMusicVolume, static_cast<int>(settings.musicVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.dialogVolume = std::clamp(registry.Get_Int(kValueNameAudioDialogVolume, static_cast<int>(settings.dialogVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.cinematicVolume = std::clamp(registry.Get_Int(kValueNameAudioCinematicVolume, static_cast<int>(settings.cinematicVolume * kVolumeScale)) / static_cast<float>(kVolumeScale), 0.0f, 1.0f);
    settings.speakerType = registry.Get_Int(kValueNameAudioSpeakerType, settings.speakerType);

    return true;
}

void SaveAudioSettingsToIni(const AudioSettings &settings, INIClass &ini)
{
    ini.Put_String(kIniSectionSound, kValueNameAudioDevice, settings.deviceName.c_str());
    ini.Put_Int(kIniSectionSound, kValueNameAudioStereo, settings.stereo ? 1 : 0);
    ini.Put_Int(kIniSectionSound, kValueNameAudioBits, settings.bitDepth);
    ini.Put_Int(kIniSectionSound, kValueNameAudioHertz, settings.sampleRate);
    ini.Put_Int(kIniSectionSound, kValueNameAudioMusicEnabled, settings.musicEnabled ? 1 : 0);
    ini.Put_Int(kIniSectionSound, kValueNameAudioSoundEnabled, settings.soundEnabled ? 1 : 0);
    ini.Put_Int(kIniSectionSound, kValueNameAudioDialogEnabled, settings.dialogEnabled ? 1 : 0);
    ini.Put_Int(kIniSectionSound, kValueNameAudioCinematicEnabled, settings.cinematicEnabled ? 1 : 0);
    ini.Put_Int(kIniSectionSound, kValueNameAudioSoundVolume, static_cast<int>(std::clamp(settings.soundVolume, 0.0f, 1.0f) * kVolumeScale));
    ini.Put_Int(kIniSectionSound, kValueNameAudioMusicVolume, static_cast<int>(std::clamp(settings.musicVolume, 0.0f, 1.0f) * kVolumeScale));
    ini.Put_Int(kIniSectionSound, kValueNameAudioDialogVolume, static_cast<int>(std::clamp(settings.dialogVolume, 0.0f, 1.0f) * kVolumeScale));
    ini.Put_Int(kIniSectionSound, kValueNameAudioCinematicVolume, static_cast<int>(std::clamp(settings.cinematicVolume, 0.0f, 1.0f) * kVolumeScale));
    ini.Put_Int(kIniSectionSound, kValueNameAudioSpeakerType, settings.speakerType);
}

bool SaveAudioSettingsToRegistry(const AudioSettings &settings)
{
    RegistryClass registry(RENEGADE_SUB_KEY_NAME_AUDIO);
    if (!registry.Is_Valid()) {
        return false;
    }

    registry.Set_String(kValueNameAudioDevice, settings.deviceName.c_str());
    registry.Set_Int(kValueNameAudioStereo, settings.stereo ? 1 : 0);
    registry.Set_Int(kValueNameAudioBits, settings.bitDepth);
    registry.Set_Int(kValueNameAudioHertz, settings.sampleRate);
    registry.Set_Int(kValueNameAudioMusicEnabled, settings.musicEnabled ? 1 : 0);
    registry.Set_Int(kValueNameAudioSoundEnabled, settings.soundEnabled ? 1 : 0);
    registry.Set_Int(kValueNameAudioDialogEnabled, settings.dialogEnabled ? 1 : 0);
    registry.Set_Int(kValueNameAudioCinematicEnabled, settings.cinematicEnabled ? 1 : 0);
    registry.Set_Int(kValueNameAudioSoundVolume, static_cast<int>(std::clamp(settings.soundVolume, 0.0f, 1.0f) * kVolumeScale));
    registry.Set_Int(kValueNameAudioMusicVolume, static_cast<int>(std::clamp(settings.musicVolume, 0.0f, 1.0f) * kVolumeScale));
    registry.Set_Int(kValueNameAudioDialogVolume, static_cast<int>(std::clamp(settings.dialogVolume, 0.0f, 1.0f) * kVolumeScale));
    registry.Set_Int(kValueNameAudioCinematicVolume, static_cast<int>(std::clamp(settings.cinematicVolume, 0.0f, 1.0f) * kVolumeScale));
    registry.Set_Int(kValueNameAudioSpeakerType, settings.speakerType);

    return true;
}

}

namespace WWConfig
{
std::string GetConfigFilePath()
{
    if (const char *envPath = std::getenv("RENEGADE_CONFIG_INI")) {
        if (*envPath != '\0') {
            return std::string(envPath);
        }
    }

#ifdef _WIN32
    char modulePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, modulePath, MAX_PATH) > 0) {
        std::string path(modulePath);
        const size_t slash = path.find_last_of("\\/");
        if (slash != std::string::npos) {
            path.resize(slash + 1);
        } else {
            path.clear();
        }
        return path + "Renegade.ini";
    }
#endif

    return std::string("Renegade.ini");
}

bool LoadRenderSettings(RenderSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    if (LoadRenderSettingsFromIni(ini, settings)) {
        return true;
    }

    return LoadRenderSettingsFromRegistry(settings);
}

bool SaveRenderSettings(const RenderSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    SaveRenderSettingsToIni(settings, ini);
    const bool iniSaved = SaveIni(ini);

    const bool registrySaved = SaveRenderSettingsToRegistry(settings);
    return iniSaved || registrySaved;
}

static void InitializeAdapterSelection(IDirect3D9 **outD3D, D3DCAPS9 *outCaps, D3DADAPTER_IDENTIFIER9 *outAdapterId, D3DFORMAT &displayFormat)
{
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        *outD3D = nullptr;
        return;
    }

    RegistryClass renderRegistry(RENEGADE_SUB_KEY_NAME_RENDER);
    renderRegistry.Is_Valid();
    int currentAdapterIndex = D3DADAPTER_DEFAULT;
    char deviceName[256] = {0};
    renderRegistry.Get_String(VALUE_NAME_RENDER_DEVICE_NAME, deviceName, sizeof(deviceName));

    int adapterCount = d3d->GetAdapterCount();
    for (int adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
        D3DADAPTER_IDENTIFIER9 id = {};
        if (SUCCEEDED(d3d->GetAdapterIdentifier(adapterIndex, 0, &id))) {
            StringClass name(id.Description, true);
            if (name == deviceName) {
                currentAdapterIndex = adapterIndex;
                break;
            }
        }
    }

    if (FAILED(d3d->GetDeviceCaps(currentAdapterIndex, D3DDEVTYPE_HAL, outCaps))) {
        d3d->Release();
        *outD3D = nullptr;
        return;
    }

    ZeroMemory(outAdapterId, sizeof(D3DADAPTER_IDENTIFIER9));
    if (FAILED(d3d->GetAdapterIdentifier(currentAdapterIndex, 0, outAdapterId))) {
        d3d->Release();
        *outD3D = nullptr;
        return;
    }

    renderRegistry.Set_String(VALUE_NAME_RENDER_DEVICE_NAME, outAdapterId->Description);
    renderRegistry.Set_Int(VALUE_NAME_RENDER_DEVICE_WIDTH, 800);
    renderRegistry.Set_Int(VALUE_NAME_RENDER_DEVICE_HEIGHT, 600);
    renderRegistry.Set_Int(VALUE_NAME_RENDER_DEVICE_DEPTH, 16);
    renderRegistry.Set_Int(VALUE_NAME_RENDER_DEVICE_WINDOWED, 0);
    renderRegistry.Set_Int(VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, 16);

    displayFormat = D3DFMT_R5G6B5;
    *outD3D = d3d;
}

void AutoConfigRenderSettings()
{
    RegistryClass registry(kKeyNameSettings);
    if (!registry.Is_Valid()) {
        return;
    }

    RegistryClass renderRegistry(RENEGADE_SUB_KEY_NAME_RENDER);
    if (!renderRegistry.Is_Valid()) {
        return;
    }

    IDirect3D9 *d3d = nullptr;
    D3DCAPS9 tmpCaps = {};
    const D3DCAPS9 *d3dCaps = nullptr;
    D3DADAPTER_IDENTIFIER9 adapterId = {};
    D3DFORMAT displayFormat = D3DFMT_R5G6B5;

    InitializeAdapterSelection(&d3d, &tmpCaps, &adapterId, displayFormat);
    if (!d3d) {
        return;
    }
    d3dCaps = &tmpCaps;

    DX8Caps caps(d3d, *d3dCaps, D3DFormat_To_WW3DFormat(displayFormat), adapterId);
    bool canDoMultiPass = caps.Can_Do_Multi_Pass();
    bool highEndProcessor = CPUDetectClass::Has_SSE_Instruction_Set();
    if (CPUDetectClass::Get_Processor_Manufacturer() == CPUDetectClass::MANUFACTURER_AMD &&
        CPUDetectClass::Get_AMD_Processor() >= CPUDetectClass::AMD_PROCESSOR_ATHLON_025) {
        highEndProcessor = true;
    }

    registry.Set_Int(kValueNameTextureRes, caps.Support_DXTC() ? 0 : 1);

    if (caps.Support_TnL()) {
        registry.Set_Int(kValueNameDynLOD, 10000);
        registry.Set_Int(kValueNameStaticLOD, 10000);
    } else if (highEndProcessor) {
        registry.Set_Int(kValueNameDynLOD, 5000);
        registry.Set_Int(kValueNameStaticLOD, 5000);
    } else {
        registry.Set_Int(kValueNameDynLOD, 0);
        registry.Set_Int(kValueNameStaticLOD, 0);
    }

    if (caps.Support_Render_To_Texture_Format(D3DFormat_To_WW3DFormat(displayFormat))) {
        if (caps.Support_TnL()) {
            registry.Set_Int(kValueNameShadowMode, 3);
            registry.Set_Int(kValueNameStaticShadows, 1);
        } else {
            registry.Set_Int(kValueNameShadowMode, 2);
            registry.Set_Int(kValueNameStaticShadows, 0);
        }
    } else {
        registry.Set_Int(kValueNameStaticShadows, 0);
        registry.Set_Int(kValueNameShadowMode, highEndProcessor ? 1 : 0);
    }

    if (caps.Support_TnL()) {
        registry.Set_Int(kValueNameSurfaceEffect, 2);
    } else {
        registry.Set_Int(kValueNameSurfaceEffect, highEndProcessor ? 1 : 0);
    }

    if (caps.Support_TnL() && highEndProcessor) {
        registry.Set_Int(kValueNameParticleDetail, 2);
    } else if (caps.Support_TnL() || highEndProcessor) {
        registry.Set_Int(kValueNameParticleDetail, 1);
    } else {
        registry.Set_Int(kValueNameParticleDetail, 0);
    }

    if (!canDoMultiPass || CPUDetectClass::Get_Total_Physical_Memory() < 100 * 1024 * 1024) {
        registry.Set_Int(kValueNamePrelitMode, 0);
    } else if (caps.Get_Max_Textures_Per_Pass() >= 2) {
        registry.Set_Int(kValueNamePrelitMode, 2);
    } else {
        registry.Set_Int(kValueNamePrelitMode, 1);
    }

    RegistryClass registryOptions(kKeyNameOptions);
    if (registryOptions.Is_Valid()) {
        registryOptions.Set_Int("ScreenUVBias", 1);
    }

    if (d3d) {
        d3d->Release();
    }

    RenderSettings updatedSettings;
    if (LoadRenderSettingsFromRegistry(updatedSettings)) {
        SaveRenderSettings(updatedSettings);
    }
}

bool LoadVideoSettings(VideoSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    if (LoadVideoSettingsFromIni(ini, settings)) {
        return true;
    }

    return LoadVideoSettingsFromRegistry(settings);
}

bool SaveVideoSettings(const VideoSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    SaveVideoSettingsToIni(settings, ini);
    const bool iniSaved = SaveIni(ini);

    const bool registrySaved = SaveVideoSettingsToRegistry(settings);
    return iniSaved || registrySaved;
}

bool LoadAudioSettings(AudioSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    if (LoadAudioSettingsFromIni(ini, settings)) {
        return true;
    }

    return LoadAudioSettingsFromRegistry(settings);
}

bool SaveAudioSettings(const AudioSettings &settings)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    SaveAudioSettingsToIni(settings, ini);
    const bool iniSaved = SaveIni(ini);

    const bool registrySaved = SaveAudioSettingsToRegistry(settings);
    return iniSaved || registrySaved;
}

bool IsDriverWarningDisabled()
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    const int disabled = ini.Get_Int(kIniSectionRender, kValueNameDriverWarningDisabled, 0);
    return disabled >= 87;
}

void SetDriverWarningDisabled(bool disabled)
{
    const std::string iniPath = GetConfigFilePath();
    INIClass ini(iniPath.c_str());
    ini.Put_Int(kIniSectionRender, kValueNameDriverWarningDisabled, disabled ? 87 : 0);
    SaveIni(ini);
}

bool EnumerateVideoAdapters(std::vector<VideoAdapterInfo> &adapters)
{
    adapters.clear();

    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        return false;
    }

    constexpr D3DFORMAT kFormats[] = {
        D3DFMT_X8R8G8B8,
        D3DFMT_A8R8G8B8,
        D3DFMT_R5G6B5,
    };

    const UINT adapterCount = d3d->GetAdapterCount();
    adapters.reserve(adapterCount);

    for (UINT adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
        D3DADAPTER_IDENTIFIER9 identifier = {};
        if (FAILED(d3d->GetAdapterIdentifier(adapterIndex, 0, &identifier))) {
            continue;
        }

        VideoAdapterInfo adapterInfo;
        adapterInfo.deviceName = identifier.Description;
        adapterInfo.description = identifier.Description;

        for (D3DFORMAT format : kFormats) {
            const UINT modeCount = d3d->GetAdapterModeCount(adapterIndex, format);
            for (UINT modeIndex = 0; modeIndex < modeCount; ++modeIndex) {
                D3DDISPLAYMODE mode = {};
                if (FAILED(d3d->EnumAdapterModes(adapterIndex, format, modeIndex, &mode))) {
                    continue;
                }

                if (mode.Width < 640 || mode.Height < 480) {
                    continue;
                }

                VideoResolution resolution;
                resolution.width = static_cast<int>(mode.Width);
                resolution.height = static_cast<int>(mode.Height);
                resolution.bitDepth = (format == D3DFMT_R5G6B5) ? 16 : 32;

                const auto exists = std::find_if(adapterInfo.resolutions.begin(),
                                                 adapterInfo.resolutions.end(),
                                                 [&](const VideoResolution &existing) {
                                                     return existing.width == resolution.width &&
                                                            existing.height == resolution.height &&
                                                            existing.bitDepth == resolution.bitDepth;
                                                 });
                if (exists == adapterInfo.resolutions.end()) {
                    adapterInfo.resolutions.push_back(resolution);
                }
            }
        }

        if (adapterInfo.resolutions.empty()) {
            continue;
        }

        std::sort(adapterInfo.resolutions.begin(),
                  adapterInfo.resolutions.end(),
                  [](const VideoResolution &lhs, const VideoResolution &rhs) {
                      if (lhs.bitDepth != rhs.bitDepth) {
                          return lhs.bitDepth < rhs.bitDepth;
                      }
                      if (lhs.width != rhs.width) {
                          return lhs.width < rhs.width;
                      }
                      if (lhs.height != rhs.height) {
                          return lhs.height < rhs.height;
                      }
                      return false;
                  });

        adapters.push_back(std::move(adapterInfo));
    }

    d3d->Release();
    return !adapters.empty();
}

} // namespace WWConfig
