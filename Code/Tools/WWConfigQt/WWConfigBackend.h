#pragma once

#include <QString>
#include <vector>
#include "../WWConfig/WWConfigSettings.h"

struct DriverWarningInfo
{
    bool show = false;
    QString message;
};

class WWConfigBackend
{
public:
    WWConfigBackend();
    ~WWConfigBackend();

    bool initializeLocale(int languageOverride);
    bool isLocaleReady() const { return m_localeInitialized; }
    QString localizedString(int stringId) const;

    RenderSettings loadRenderSettings() const;
    bool saveRenderSettings(const RenderSettings &settings);
    void autoConfigRenderSettings();

    VideoSettings loadVideoSettings() const;
    bool saveVideoSettings(const VideoSettings &settings);

    AudioSettings loadAudioSettings() const;
    bool saveAudioSettings(const AudioSettings &settings);

    std::vector<VideoAdapterInfo> enumerateVideoAdapters() const;

    QString configPath() const;
    bool checkDriverWarning(DriverWarningInfo &info) const;
    void disableDriverWarning() const;

private:
    bool m_localeInitialized = false;
};
