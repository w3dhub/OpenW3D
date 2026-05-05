#pragma once

#include <QWidget>

#include "WWConfigBackend.h"

class QCheckBox;
class QComboBox;
class QPushButton;
class QSlider;
class QGroupBox;

class PerformancePage : public QWidget
{
    Q_OBJECT

public:
    explicit PerformancePage(WWConfigBackend &backend, QWidget *parent = nullptr);

    void refresh();
    bool save();

signals:
    void settingsChanged();

private:
    void buildUi();
    void loadSettings();
    void updateControlsFromSettings();
    void updateSettingsFromControls();
    void applyPreset(int level);
    int determinePresetLevel() const;
    int geometryLevelFromSettings() const;

    void setExpertControlsEnabled(bool enabled);
    void updateCapabilityCombos();
    int comboValue(const QComboBox *combo, int fallback) const;
    static void setComboValue(QComboBox *combo, int value, int fallback);

    WWConfigBackend &m_backend;
    RenderSettings m_settings;
    VideoSettings m_videoSettings;
    bool m_blockSignals = false;

    QSlider *m_overallSlider = nullptr;
    QCheckBox *m_expertCheck = nullptr;
    QPushButton *m_autoButton = nullptr;

    QGroupBox *m_expertGroup = nullptr;
    QSlider *m_geometrySlider = nullptr;
    QSlider *m_shadowSlider = nullptr;
    QSlider *m_textureSlider = nullptr;
    QSlider *m_surfaceSlider = nullptr;
    QSlider *m_particleSlider = nullptr;
    QComboBox *m_lightingCombo = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QCheckBox *m_terrainCheck = nullptr;
};
