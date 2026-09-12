#include "AudioPage.h"
#include "ui_AudioPage.h"

#include <algorithm>
#include <cmath>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QSlider>

namespace
{
QString DeviceDisplayName(const AudioSettings &settings)
{
    if (settings.deviceName.empty()) {
        return QObject::tr("Default Device");
    }
    return QString::fromStdString(settings.deviceName);
}

int RateIndexFromSampleRate(int sampleRate)
{
    switch (sampleRate) {
    case 11025: return 0;
    case 22050: return 1;
    default: return 2;
    }
}

int SampleRateFromIndex(int index)
{
    switch (index) {
    case 0: return 11025;
    case 1: return 22050;
    default: return 44100;
    }
}
} // namespace

AudioPage::AudioPage(WWConfigBackend &backend, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::AudioPage),
      m_backend(backend)
{
    buildUi();
    refresh();
}

AudioPage::~AudioPage()
{
    delete m_ui;
}

void AudioPage::buildUi()
{
    m_ui->setupUi(this);
    m_driverList = m_ui->driverList;
    m_stereoCheck = m_ui->stereoCheck;
    m_qualityCombo = m_ui->qualityCombo;
    m_rateCombo = m_ui->rateCombo;
    m_speakerCombo = m_ui->speakerCombo;
    m_soundEnableCheck = m_ui->soundEnableCheck;
    m_musicEnableCheck = m_ui->musicEnableCheck;
    m_dialogEnableCheck = m_ui->dialogEnableCheck;
    m_cinematicEnableCheck = m_ui->cinematicEnableCheck;
    m_soundSlider = m_ui->soundSlider;
    m_musicSlider = m_ui->musicSlider;
    m_dialogSlider = m_ui->dialogSlider;
    m_cinematicSlider = m_ui->cinematicSlider;

    auto applyOnChange = [this]() {
        updateSettingsFromControls();
    };

    connect(m_stereoCheck, &QCheckBox::toggled, this, applyOnChange);
    connect(m_qualityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, applyOnChange);
    connect(m_rateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, applyOnChange);
    connect(m_speakerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, applyOnChange);
    connect(m_soundEnableCheck, &QCheckBox::toggled, this, applyOnChange);
    connect(m_musicEnableCheck, &QCheckBox::toggled, this, applyOnChange);
    connect(m_dialogEnableCheck, &QCheckBox::toggled, this, applyOnChange);
    connect(m_cinematicEnableCheck, &QCheckBox::toggled, this, applyOnChange);
    connect(m_soundSlider, &QSlider::valueChanged, this, applyOnChange);
    connect(m_musicSlider, &QSlider::valueChanged, this, applyOnChange);
    connect(m_dialogSlider, &QSlider::valueChanged, this, applyOnChange);
    connect(m_cinematicSlider, &QSlider::valueChanged, this, applyOnChange);
}

void AudioPage::refresh()
{
    m_blockSignals = true;
    m_settings = m_backend.loadAudioSettings();
    updateFromSettings();
    m_blockSignals = false;
}

bool AudioPage::save()
{
    updateSettingsFromControls();
    return m_backend.saveAudioSettings(m_settings);
}

void AudioPage::updateFromSettings()
{
    m_driverList->clear();
    auto *item = new QListWidgetItem(DeviceDisplayName(m_settings));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    m_driverList->addItem(item);
    m_driverList->setCurrentRow(0);

    m_stereoCheck->setChecked(m_settings.stereo);

    const int qualityIndex = (m_settings.bitDepth <= 8) ? 0 : 1;
    m_qualityCombo->setCurrentIndex(qualityIndex);
    m_rateCombo->setCurrentIndex(RateIndexFromSampleRate(m_settings.sampleRate));
    m_speakerCombo->setCurrentIndex(std::clamp(m_settings.speakerType, 0, 3));

    setVolumeRow(m_soundSlider, m_soundEnableCheck, m_settings.soundVolume, m_settings.soundEnabled);
    setVolumeRow(m_musicSlider, m_musicEnableCheck, m_settings.musicVolume, m_settings.musicEnabled);
    setVolumeRow(m_dialogSlider, m_dialogEnableCheck, m_settings.dialogVolume, m_settings.dialogEnabled);
    setVolumeRow(m_cinematicSlider, m_cinematicEnableCheck, m_settings.cinematicVolume, m_settings.cinematicEnabled);
}

void AudioPage::updateSettingsFromControls()
{
    if (m_blockSignals) {
        return;
    }

    m_settings.stereo = m_stereoCheck->isChecked();
    m_settings.bitDepth = (m_qualityCombo->currentIndex() == 0) ? 8 : 16;
    m_settings.sampleRate = SampleRateFromIndex(m_rateCombo->currentIndex());
    m_settings.speakerType = m_speakerCombo->currentIndex();

    m_settings.soundEnabled = m_soundEnableCheck->isChecked();
    m_settings.musicEnabled = m_musicEnableCheck->isChecked();
    m_settings.dialogEnabled = m_dialogEnableCheck->isChecked();
    m_settings.cinematicEnabled = m_cinematicEnableCheck->isChecked();

    m_soundSlider->setEnabled(m_settings.soundEnabled);
    m_musicSlider->setEnabled(m_settings.musicEnabled);
    m_dialogSlider->setEnabled(m_settings.dialogEnabled);
    m_cinematicSlider->setEnabled(m_settings.cinematicEnabled);

    m_settings.soundVolume = std::clamp(m_soundSlider->value() / 100.0f, 0.0f, 1.0f);
    m_settings.musicVolume = std::clamp(m_musicSlider->value() / 100.0f, 0.0f, 1.0f);
    m_settings.dialogVolume = std::clamp(m_dialogSlider->value() / 100.0f, 0.0f, 1.0f);
    m_settings.cinematicVolume = std::clamp(m_cinematicSlider->value() / 100.0f, 0.0f, 1.0f);

}

void AudioPage::setVolumeRow(QSlider *slider, QCheckBox *check, float value, bool enabled)
{
    slider->setValue(static_cast<int>(std::lround(std::clamp(value, 0.0f, 1.0f) * 100.0f)));
    check->setChecked(enabled);
    slider->setEnabled(enabled);
}
