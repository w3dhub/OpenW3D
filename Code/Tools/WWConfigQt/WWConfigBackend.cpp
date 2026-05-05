#include "WWConfigBackend.h"

#if defined(_WIN32)
#include <d3d9.h>
#endif

#include <QObject>

#include "../WWConfig/Locale_API.h"
#include "../WWConfig/WWConfigSettings.h"
#include "../WWConfig/wwconfig_ids.h"
#if defined(_WIN32)
#include "../../ww3d2/dx8caps.h"
#include "../../ww3d2/ww3d.h"
#endif

WWConfigBackend::WWConfigBackend() = default;

WWConfigBackend::~WWConfigBackend()
{
    if (m_localeInitialized) {
        Locale_Restore();
        m_localeInitialized = false;
    }
}

bool WWConfigBackend::initializeLocale(int languageOverride)
{
    if (m_localeInitialized) {
        Locale_Restore();
        m_localeInitialized = false;
    }

    int language = languageOverride;
    if (Locale_Init(language, "WWConfig.loc")) {
        m_localeInitialized = true;
        return true;
    }

    return false;
}

QString WWConfigBackend::localizedString(int stringId) const
{
    if (!m_localeInitialized) {
        return {};
    }

    const wchar_t *text = Locale_GetString(stringId);
    if (!text) {
        return {};
    }

    return QString::fromWCharArray(text);
}

RenderSettings WWConfigBackend::loadRenderSettings() const
{
    RenderSettings settings;
    WWConfig::LoadRenderSettings(settings);
    return settings;
}

bool WWConfigBackend::saveRenderSettings(const RenderSettings &settings)
{
    return WWConfig::SaveRenderSettings(settings);
}

void WWConfigBackend::autoConfigRenderSettings()
{
    WWConfig::AutoConfigRenderSettings();
}

VideoSettings WWConfigBackend::loadVideoSettings() const
{
    VideoSettings settings;
    WWConfig::LoadVideoSettings(settings);
    return settings;
}

bool WWConfigBackend::saveVideoSettings(const VideoSettings &settings)
{
    return WWConfig::SaveVideoSettings(settings);
}

AudioSettings WWConfigBackend::loadAudioSettings() const
{
    AudioSettings settings;
    WWConfig::LoadAudioSettings(settings);
    return settings;
}

bool WWConfigBackend::saveAudioSettings(const AudioSettings &settings)
{
    return WWConfig::SaveAudioSettings(settings);
}

std::vector<VideoAdapterInfo> WWConfigBackend::enumerateVideoAdapters() const
{
    std::vector<VideoAdapterInfo> adapters;
    WWConfig::EnumerateVideoAdapters(adapters);
    return adapters;
}

QString WWConfigBackend::configPath() const
{
    return QString::fromStdString(WWConfig::GetConfigFilePath());
}

bool WWConfigBackend::checkDriverWarning(DriverWarningInfo &info) const
{
    info.show = false;
    info.message.clear();

#if !defined(_WIN32)
    return false;
#else
    if (WWConfig::IsDriverWarningDisabled()) {
        return false;
    }

    VideoSettings settings;
    WWConfig::LoadVideoSettings(settings);

    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        return false;
    }

    int adapterIndex = D3DADAPTER_DEFAULT;
    if (!settings.deviceName.empty()) {
        const int count = static_cast<int>(d3d->GetAdapterCount());
        for (int i = 0; i < count; ++i) {
            D3DADAPTER_IDENTIFIER9 id = {};
            if (SUCCEEDED(d3d->GetAdapterIdentifier(i, 0, &id))) {
                if (_stricmp(id.Description, settings.deviceName.c_str()) == 0) {
                    adapterIndex = i;
                    break;
                }
            }
        }
    }

    D3DCAPS9 caps = {};
    D3DADAPTER_IDENTIFIER9 adapterId = {};
    if (FAILED(d3d->GetDeviceCaps(adapterIndex, D3DDEVTYPE_HAL, &caps)) ||
        FAILED(d3d->GetAdapterIdentifier(adapterIndex, 0, &adapterId))) {
        d3d->Release();
        return false;
    }

    DX8Caps dxCaps(d3d, caps, WW3D_FORMAT_UNKNOWN, adapterId);
    const auto status = dxCaps.Get_Driver_Version_Status();
    d3d->Release();

    if (status != DX8Caps::DRIVER_STATUS_BAD) {
        return false;
    }

    const QString driverName =
        QString::fromLatin1(static_cast<const char *>(dxCaps.Get_Driver_Name().Peek_Buffer()));
    const unsigned build = dxCaps.Get_Driver_Build_Version();
    const QString versionText =
        QStringLiteral("%1.%2").arg(build / 100).arg(build % 100, 2, 10, QLatin1Char('0'));

    const QString header = localizedString(IDS_KNOW_PROBLEMS);
    const QString nameLabel = localizedString(IDS_DRIVER_NAME);
    const QString versionLabel = localizedString(IDS_DRIVER_VERSION);

    auto fallback = [](const QString &text, const char *builtin) {
        return text.isEmpty() ? QObject::tr(builtin) : text;
    };

    info.message = QStringLiteral("%1\n\n%2 %3\n%4 %5")
                       .arg(fallback(header, "Your video driver is known to cause problems with Renegade."),
                            fallback(nameLabel, "Driver:"),
                            driverName,
                            fallback(versionLabel, "Driver version:"),
                            versionText);
    info.show = true;
    return true;
#endif
}

void WWConfigBackend::disableDriverWarning() const
{
    WWConfig::SetDriverWarningDisabled(true);
}
