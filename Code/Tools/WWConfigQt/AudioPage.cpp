#include "AudioPage.h"

#include <algorithm>
#include <cmath>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QSlider>
#include <QVBoxLayout>

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
      m_backend(backend)
{
    buildUi();
    refresh();
}

void AudioPage::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    auto *deviceGroup = new QGroupBox(tr("Device"), this);
    auto *deviceLayout = new QGridLayout(deviceGroup);
    deviceLayout->setContentsMargins(6, 6, 6, 6);
    deviceLayout->setHorizontalSpacing(6);
    deviceLayout->setVerticalSpacing(4);

    deviceLayout->addWidget(new QLabel(tr("Driver:"), deviceGroup), 0, 0, Qt::AlignLeft);
    m_driverList = new QListWidget(deviceGroup);
    m_driverList->setUniformItemSizes(true);
    m_driverList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_driverList->setMinimumHeight(60);
    deviceLayout->addWidget(m_driverList, 0, 1, 1, 2);

    deviceLayout->setColumnStretch(1, 1);
    layout->addWidget(deviceGroup);

    auto *volumeGroup = new QGroupBox(tr("Volume"), this);
    auto *volumeLayout = new QGridLayout(volumeGroup);
    volumeLayout->setContentsMargins(6, 6, 6, 6);
    volumeLayout->setHorizontalSpacing(6);
    volumeLayout->setVerticalSpacing(4);

    auto createRow = [&](int row, const QString &labelText, QCheckBox *&check, QSlider *&slider) {
        check = new QCheckBox(labelText, volumeGroup);
        slider = new QSlider(Qt::Horizontal, volumeGroup);
        slider->setRange(0, 100);
        slider->setTickInterval(10);
        volumeLayout->addWidget(check, row, 0, Qt::AlignLeft);
        volumeLayout->addWidget(slider, row, 1);
    };

    createRow(0, tr("Sound Effects"), m_soundEnableCheck, m_soundSlider);
    createRow(1, tr("Music"), m_musicEnableCheck, m_musicSlider);
    createRow(2, tr("Dialog"), m_dialogEnableCheck, m_dialogSlider);
    createRow(3, tr("Cinematic"), m_cinematicEnableCheck, m_cinematicSlider);

    volumeLayout->setColumnStretch(1, 1);
    layout->addWidget(volumeGroup);

    auto *playbackGroup = new QGroupBox(tr("Playback"), this);
    auto *playbackLayout = new QGridLayout(playbackGroup);
    playbackLayout->setContentsMargins(6, 6, 6, 6);
    playbackLayout->setHorizontalSpacing(8);
    playbackLayout->setVerticalSpacing(4);

    m_qualityCombo = new QComboBox(playbackGroup);
    m_qualityCombo->addItems({tr("8-bit"), tr("16-bit")});
    playbackLayout->addWidget(new QLabel(tr("Quality"), playbackGroup), 0, 0, Qt::AlignLeft);
    playbackLayout->addWidget(m_qualityCombo, 1, 0);

    m_rateCombo = new QComboBox(playbackGroup);
    m_rateCombo->addItems({tr("11 kHz"), tr("22 kHz"), tr("44 kHz")});
    playbackLayout->addWidget(new QLabel(tr("Playback Rate"), playbackGroup), 0, 1, Qt::AlignLeft);
    playbackLayout->addWidget(m_rateCombo, 1, 1);

    m_speakerCombo = new QComboBox(playbackGroup);
    m_speakerCombo->addItems({tr("2 Speaker"), tr("Headphones"), tr("Surround"), tr("4 Speaker")});
    playbackLayout->addWidget(new QLabel(tr("Speaker Setup"), playbackGroup), 0, 2, Qt::AlignLeft);
    playbackLayout->addWidget(m_speakerCombo, 1, 2);

    playbackLayout->setColumnStretch(0, 1);
    playbackLayout->setColumnStretch(1, 1);
    playbackLayout->setColumnStretch(2, 1);
    layout->addWidget(playbackGroup);

    m_stereoCheck = new QCheckBox(tr("Stereo playback"), this);
    layout->addWidget(m_stereoCheck, 0, Qt::AlignLeft);

    auto *note = new QLabel(tr("Press OK to save audio changes to Renegade.ini."), this);
    note->setWordWrap(true);
    layout->addWidget(note);
    layout->addStretch();

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
