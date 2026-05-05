#include "PerformancePage.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "../WWConfig/wwconfig_ids.h"
#include "../../ww3d2/ww3d.h"
#if defined(_WIN32)
#include "../../ww3d2/dx8caps.h"
#include "../../ww3d2/formconv.h"
#endif
#include "../../ww3d2/texture.h"

namespace
{
struct PresetLevel
{
    int geometry;
    int character;
    int texture;
    int surface;
    int particle;
    int textureFilter;
    int lightingMode;
    bool terrainShadows;
};

constexpr PresetLevel kPresets[] = {
    {0, 0, 0, 0, 0, TextureClass::TEXTURE_FILTER_BILINEAR, WW3D::PRELIT_MODE_VERTEX, false},
    {0, 1, 1, 0, 0, TextureClass::TEXTURE_FILTER_BILINEAR, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE, false},
    {1, 2, 2, 1, 1, TextureClass::TEXTURE_FILTER_TRILINEAR, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE, true},
    {2, 3, 2, 2, 2, TextureClass::TEXTURE_FILTER_TRILINEAR, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE, true},
};
constexpr int kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

int textureSliderFromResolution(int textureRes)
{
    return std::clamp(2 - textureRes, 0, 2);
}

int textureResolutionFromSlider(int sliderValue)
{
    return std::max(2 - sliderValue, 0);
}

struct RenderCapabilityInfo
{
    bool canMultiPass = true;
    bool canMultiTexture = true;
    bool canAnisotropic = false;
};

QString LocalizedText(const WWConfigBackend &backend, int id, const QString &fallback)
{
    const QString text = backend.localizedString(id);
    return text.isEmpty() ? fallback : text;
}

RenderCapabilityInfo QueryRenderCapabilities(const VideoSettings &settings)
{
    RenderCapabilityInfo caps;

#if !defined(_WIN32)
    (void)settings;
    return caps;
#else
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        return caps;
    }

    UINT adapterIndex = D3DADAPTER_DEFAULT;
    if (!settings.deviceName.empty()) {
        const UINT adapterCount = d3d->GetAdapterCount();
        for (UINT i = 0; i < adapterCount; ++i) {
            D3DADAPTER_IDENTIFIER9 identifier = {};
            if (FAILED(d3d->GetAdapterIdentifier(i, 0, &identifier))) {
                continue;
            }
            if (_stricmp(identifier.Description, settings.deviceName.c_str()) == 0) {
                adapterIndex = i;
                break;
            }
        }
    }

    D3DCAPS9 d3dCaps = {};
    D3DADAPTER_IDENTIFIER9 adapterId = {};
    if (SUCCEEDED(d3d->GetDeviceCaps(adapterIndex, D3DDEVTYPE_HAL, &d3dCaps)) &&
        SUCCEEDED(d3d->GetAdapterIdentifier(adapterIndex, 0, &adapterId))) {
        const D3DFORMAT displayFormat = (settings.bitDepth == 16) ? D3DFMT_R5G6B5 : D3DFMT_A8R8G8B8;
        DX8Caps dxCaps(d3d, d3dCaps, D3DFormat_To_WW3DFormat(displayFormat), adapterId);
        caps.canMultiPass = dxCaps.Can_Do_Multi_Pass();
        caps.canMultiTexture = dxCaps.Get_Max_Textures_Per_Pass() > 1;
        caps.canAnisotropic = dxCaps.Support_Anisotropic_Filtering();
    }

    d3d->Release();
    return caps;
#endif
}
} // namespace

PerformancePage::PerformancePage(WWConfigBackend &backend, QWidget *parent)
    : QWidget(parent),
      m_backend(backend)
{
    buildUi();
    refresh();
}

void PerformancePage::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);

    auto *detailGroup = new QGroupBox(LocalizedText(m_backend, IDS_DETAIL, tr("Detail")), this);
    auto *detailLayout = new QGridLayout(detailGroup);

    auto *lowFast = new QLabel(LocalizedText(m_backend, IDS_LOW_DESC, tr("Low (fastest)")), detailGroup);
    lowFast->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    detailLayout->addWidget(lowFast, 0, 0);

    m_overallSlider = new QSlider(Qt::Horizontal, detailGroup);
    m_overallSlider->setRange(0, 3);
    m_overallSlider->setTickInterval(1);
    m_overallSlider->setTickPosition(QSlider::TicksBelow);
    detailLayout->addWidget(m_overallSlider, 0, 1);

    auto *highSlow = new QLabel(LocalizedText(m_backend, IDS_HIGH_DESC, tr("High (slowest)")), detailGroup);
    highSlow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    detailLayout->addWidget(highSlow, 0, 2);

    m_expertCheck = new QCheckBox(LocalizedText(m_backend, IDS_EXPERT_MODE, tr("Expert Mode")), detailGroup);
    detailLayout->addWidget(m_expertCheck, 1, 0, 1, 2);

    m_autoButton = new QPushButton(LocalizedText(m_backend, IDS_AUTOCONFIG, tr("Auto Config")), detailGroup);
    detailLayout->addWidget(m_autoButton, 1, 2, Qt::AlignRight);

    detailLayout->setColumnStretch(1, 1);
    mainLayout->addWidget(detailGroup);

    m_expertGroup = new QGroupBox(LocalizedText(m_backend, IDS_EXPERT_SETTINGS, tr("Expert Settings")), this);
    auto *groupLayout = new QGridLayout(m_expertGroup);

    auto createSlider = [this](int maximum = 2) {
        auto *slider = new QSlider(Qt::Horizontal, m_expertGroup);
        slider->setRange(0, maximum);
        slider->setTickInterval(1);
        slider->setTickPosition(QSlider::TicksBelow);
        return slider;
    };

    m_geometrySlider = createSlider();
    m_shadowSlider = createSlider(3);
    m_textureSlider = createSlider();
    m_particleSlider = createSlider();
    m_surfaceSlider = createSlider();

    int row = 0;
    auto addSliderRow = [&](const QString &label, QSlider *slider) {
        auto *title = new QLabel(label, m_expertGroup);
        groupLayout->addWidget(title, row, 0);
        groupLayout->addWidget(slider, row, 1);
        auto *lowLabel = new QLabel(LocalizedText(m_backend, IDS_LOW, tr("Low")), m_expertGroup);
        lowLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        groupLayout->addWidget(lowLabel, row, 2);
        auto *highLabel = new QLabel(LocalizedText(m_backend, IDS_HIGH, tr("High")), m_expertGroup);
        groupLayout->addWidget(highLabel, row, 3);
        ++row;
    };

    addSliderRow(LocalizedText(m_backend, IDS_GEOMETRY_DETAIL, tr("Geometry Detail")), m_geometrySlider);
    addSliderRow(LocalizedText(m_backend, IDS_CHARACTER_SHADOWS, tr("Character Shadows")), m_shadowSlider);
    addSliderRow(LocalizedText(m_backend, IDS_TEXTURE_DETAIL, tr("Texture Detail")), m_textureSlider);
    addSliderRow(LocalizedText(m_backend, IDS_PARTICLE_DETAIL, tr("Particle Detail")), m_particleSlider);
    addSliderRow(LocalizedText(m_backend, IDS_SURFACE_EFFECT_DETAIL, tr("Surface Effect Detail")), m_surfaceSlider);

    m_lightingCombo = new QComboBox(m_expertGroup);
    groupLayout->addWidget(new QLabel(LocalizedText(m_backend, IDS_LIGHTING_MODE, tr("Lighting Mode")), m_expertGroup),
                           row,
                           0);
    groupLayout->addWidget(m_lightingCombo, row, 1);
    ++row;

    m_filterCombo = new QComboBox(m_expertGroup);
    groupLayout->addWidget(new QLabel(LocalizedText(m_backend, IDS_TEXTURE_FILTER, tr("Texture Filter")), m_expertGroup),
                           row,
                           0);
    groupLayout->addWidget(m_filterCombo, row, 1);
    ++row;

    m_terrainCheck = new QCheckBox(LocalizedText(m_backend, IDS_TERRAIN_SHADOWS, tr("Terrain Casts Shadows")),
                                   m_expertGroup);
    groupLayout->addWidget(m_terrainCheck, row, 0, 1, 4);

    groupLayout->setColumnStretch(1, 1);
    mainLayout->addWidget(m_expertGroup);

    setExpertControlsEnabled(false);

    connect(m_overallSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_blockSignals) {
            return;
        }
        if (!m_expertCheck->isChecked()) {
            applyPreset(value);
        }
    });

    connect(m_expertCheck, &QCheckBox::toggled, this, [this](bool checked) {
        setExpertControlsEnabled(checked);
        if (!checked) {
            applyPreset(m_overallSlider->value());
        }
    });

    connect(m_autoButton, &QPushButton::clicked, this, [this]() {
        const RenderSettings previousRenderSettings = m_backend.loadRenderSettings();
        const VideoSettings previousVideoSettings = m_backend.loadVideoSettings();

        m_backend.autoConfigRenderSettings();
        m_settings = m_backend.loadRenderSettings();
        m_videoSettings = m_backend.loadVideoSettings();

        m_backend.saveRenderSettings(previousRenderSettings);
        m_backend.saveVideoSettings(previousVideoSettings);

        updateControlsFromSettings();
    });

    auto sliderChanged = [this]() {
        if (m_blockSignals) {
            return;
        }
        updateSettingsFromControls();
    };

    connect(m_geometrySlider, &QSlider::valueChanged, this, sliderChanged);
    connect(m_shadowSlider, &QSlider::valueChanged, this, sliderChanged);
    connect(m_textureSlider, &QSlider::valueChanged, this, sliderChanged);
    connect(m_surfaceSlider, &QSlider::valueChanged, this, sliderChanged);
    connect(m_particleSlider, &QSlider::valueChanged, this, sliderChanged);
    connect(m_lightingCombo, &QComboBox::currentIndexChanged, this, sliderChanged);
    connect(m_filterCombo, &QComboBox::currentIndexChanged, this, sliderChanged);
    connect(m_terrainCheck, &QCheckBox::toggled, this, sliderChanged);
}

void PerformancePage::refresh()
{
    loadSettings();
    updateControlsFromSettings();
}

bool PerformancePage::save()
{
    updateSettingsFromControls();
    return m_backend.saveRenderSettings(m_settings);
}

void PerformancePage::loadSettings()
{
    m_settings = m_backend.loadRenderSettings();
    m_videoSettings = m_backend.loadVideoSettings();
}

void PerformancePage::updateControlsFromSettings()
{
    m_blockSignals = true;

    updateCapabilityCombos();

    m_geometrySlider->setValue(geometryLevelFromSettings());
    m_shadowSlider->setValue(std::clamp(m_settings.shadowMode, 0, 3));
    m_textureSlider->setValue(textureSliderFromResolution(m_settings.textureResolution));
    m_surfaceSlider->setValue(std::clamp(m_settings.surfaceEffect, 0, 2));
    m_particleSlider->setValue(std::clamp(m_settings.particleDetail, 0, 2));
    const int lightingFallback = (m_settings.prelitMode == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS)
                                     ? WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE
                                     : WW3D::PRELIT_MODE_VERTEX;
    setComboValue(m_lightingCombo, m_settings.prelitMode, lightingFallback);
    const int filterFallback = (m_settings.textureFilter == TextureClass::TEXTURE_FILTER_BILINEAR)
                                   ? TextureClass::TEXTURE_FILTER_BILINEAR
                                   : TextureClass::TEXTURE_FILTER_TRILINEAR;
    setComboValue(m_filterCombo, m_settings.textureFilter, filterFallback);
    m_terrainCheck->setChecked(m_settings.staticShadows != 0);

    const int presetLevel = determinePresetLevel();
    m_overallSlider->setValue(presetLevel);

    m_blockSignals = false;
}

void PerformancePage::updateSettingsFromControls()
{
    const int lodValue = [this]() {
        switch (m_geometrySlider->value()) {
        case 0: return 0;
        case 1: return 5000;
        default: return 10000;
        }
    }();
    m_settings.dynamicLOD = lodValue;
    m_settings.staticLOD = lodValue;

    m_settings.shadowMode = m_shadowSlider->value();
    m_settings.dynamicShadows = (m_settings.shadowMode != 0) ? 1 : 0;
    m_settings.textureResolution = textureResolutionFromSlider(m_textureSlider->value());
    m_settings.surfaceEffect = m_surfaceSlider->value();
    m_settings.particleDetail = m_particleSlider->value();
    m_settings.prelitMode = comboValue(m_lightingCombo, m_settings.prelitMode);
    m_settings.textureFilter = comboValue(m_filterCombo, m_settings.textureFilter);
    m_settings.staticShadows = m_terrainCheck->isChecked() ? 1 : 0;
}

void PerformancePage::applyPreset(int level)
{
    const int index = std::clamp(level, 0, kPresetCount - 1);
    const PresetLevel &preset = kPresets[index];

    m_blockSignals = true;
    m_geometrySlider->setValue(preset.geometry);
    m_shadowSlider->setValue(preset.character);
    m_textureSlider->setValue(preset.texture);
    m_surfaceSlider->setValue(preset.surface);
    m_particleSlider->setValue(preset.particle);
    const int lightingFallback = (preset.lightingMode == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS)
                                     ? WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE
                                     : WW3D::PRELIT_MODE_VERTEX;
    setComboValue(m_lightingCombo, preset.lightingMode, lightingFallback);
    const int filterFallback = (preset.textureFilter == TextureClass::TEXTURE_FILTER_BILINEAR)
                                   ? TextureClass::TEXTURE_FILTER_BILINEAR
                                   : TextureClass::TEXTURE_FILTER_TRILINEAR;
    setComboValue(m_filterCombo, preset.textureFilter, filterFallback);
    m_terrainCheck->setChecked(preset.terrainShadows);
    m_blockSignals = false;

    updateSettingsFromControls();
}

int PerformancePage::determinePresetLevel() const
{
    const std::array<int, 8> optionValues = {
        m_shadowSlider->value(),
        m_textureSlider->value(),
        m_particleSlider->value(),
        m_surfaceSlider->value(),
        m_geometrySlider->value(),
        comboValue(m_filterCombo, m_settings.textureFilter),
        comboValue(m_lightingCombo, m_settings.prelitMode),
        m_terrainCheck->isChecked() ? 1 : 0,
    };

    std::array<float, 8> optionLevels = {};
    std::array<int, 8> countPerOption = {};

    for (int level = kPresetCount - 1; level >= 0; --level) {
        const auto &preset = kPresets[level];
        const std::array<int, 8> presetValues = {
            preset.character,
            preset.texture,
            preset.particle,
            preset.surface,
            preset.geometry,
            preset.textureFilter,
            preset.lightingMode,
            preset.terrainShadows ? 1 : 0,
        };

        for (size_t index = 0; index < presetValues.size(); ++index) {
            if (optionValues[index] == presetValues[index]) {
                optionLevels[index] += static_cast<float>(level);
                countPerOption[index] += 1;
            }
        }
    }

    float levelSum = 0.0f;
    for (size_t index = 0; index < optionValues.size(); ++index) {
        const float normalized = (countPerOption[index] > 0)
                                     ? (optionLevels[index] / countPerOption[index])
                                     : 0.0f;
        levelSum += normalized;
    }

    const float levelRating = levelSum / static_cast<float>(optionValues.size());
    const int level = static_cast<int>(levelRating + 0.5f);
    return std::clamp(level, 0, kPresetCount - 1);
}

int PerformancePage::geometryLevelFromSettings() const
{
    if (m_settings.dynamicLOD < 1000 && m_settings.staticLOD < 1000) {
        return 0;
    }
    if (m_settings.dynamicLOD <= 5000 && m_settings.staticLOD <= 5000) {
        return 1;
    }
    return 2;
}

void PerformancePage::setExpertControlsEnabled(bool enabled)
{
    m_expertGroup->setEnabled(enabled);
}

void PerformancePage::updateCapabilityCombos()
{
    const RenderCapabilityInfo caps = QueryRenderCapabilities(m_videoSettings);

    m_lightingCombo->clear();
    m_lightingCombo->addItem(LocalizedText(m_backend, IDS_VERTEX, tr("Vertex")), WW3D::PRELIT_MODE_VERTEX);
    if (caps.canMultiPass) {
        m_lightingCombo->addItem(LocalizedText(m_backend, IDS_MULTI_PASS, tr("Multi-pass")),
                                 WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
    }
    if (caps.canMultiTexture) {
        m_lightingCombo->addItem(LocalizedText(m_backend, IDS_MULTI_TEXTURE, tr("Multi-texture")),
                                 WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
    }

    m_filterCombo->clear();
    m_filterCombo->addItem(LocalizedText(m_backend, IDS_BILINEAR, tr("Bilinear")),
                           TextureClass::TEXTURE_FILTER_BILINEAR);
    m_filterCombo->addItem(LocalizedText(m_backend, IDS_TRILINEAR, tr("Trilinear")),
                           TextureClass::TEXTURE_FILTER_TRILINEAR);
    if (caps.canAnisotropic) {
        m_filterCombo->addItem(LocalizedText(m_backend, IDS_ANISOTROPIC, tr("Anisotropic")),
                               TextureClass::TEXTURE_FILTER_ANISOTROPIC);
    }
}

int PerformancePage::comboValue(const QComboBox *combo, int fallback) const
{
    if (!combo) {
        return fallback;
    }

    const QVariant data = combo->currentData();
    if (data.isValid()) {
        return data.toInt();
    }

    return combo->currentIndex();
}

void PerformancePage::setComboValue(QComboBox *combo, int value, int fallback)
{
    if (!combo) {
        return;
    }

    int index = combo->findData(value);
    if (index < 0) {
        index = combo->findData(fallback);
    }
    if (index < 0 && combo->count() > 0) {
        index = combo->count() - 1;
    }
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}
