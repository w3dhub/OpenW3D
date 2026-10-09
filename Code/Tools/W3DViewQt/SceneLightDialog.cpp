#include "SceneLightDialog.h"

#include "W3DViewport.h"
#include "ui_SceneLightDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>

SceneLightDialog::SceneLightDialog(W3DViewport &viewport, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::SceneLightDialog)
    , _viewport(&viewport)
{
    _ui->setupUi(this);

    _initialState = _viewport->sceneLightState();

    setColorControls(_initialState.diffuse);
    _ui->distanceSpinBox->setValue(_initialState.distance);
    _ui->intensitySlider->setValue(static_cast<int>(_initialState.intensity * 100.0f));
    _ui->attenuationStartSpinBox->setValue(_initialState.attenuationStart);
    _ui->attenuationEndSpinBox->setValue(_initialState.attenuationEnd);
    _ui->attenuationGroupBox->setChecked(_initialState.attenuationEnabled);
    updateAttenuationControls(_initialState.attenuationEnabled);

    connect(_ui->redSlider, &QSlider::valueChanged, this,
            [this](int value) { colorSliderChanged(_ui->redSlider, value); });
    connect(_ui->greenSlider, &QSlider::valueChanged, this,
            [this](int value) { colorSliderChanged(_ui->greenSlider, value); });
    connect(_ui->blueSlider, &QSlider::valueChanged, this,
            [this](int value) { colorSliderChanged(_ui->blueSlider, value); });
    connect(_ui->grayscaleCheckBox, &QCheckBox::toggled, this, [this](bool enabled) {
        if (enabled) {
            const QSignalBlocker green_blocker(_ui->greenSlider);
            const QSignalBlocker blue_blocker(_ui->blueSlider);
            _ui->greenSlider->setValue(_ui->redSlider->value());
            _ui->blueSlider->setValue(_ui->redSlider->value());
            applyColorFromControls();
        }
    });

    connect(_ui->diffuseRadioButton, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            _currentChannel = Diffuse;
            setColorControls(_viewport->sceneLightDiffuse());
        }
    });
    connect(_ui->specularRadioButton, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            _currentChannel = Specular;
            setColorControls(_viewport->sceneLightSpecular());
        }
    });
    connect(_ui->bothRadioButton, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            _currentChannel = Both;
        }
    });

    connect(_ui->intensitySlider, &QSlider::valueChanged, this, [this](int value) {
        _viewport->setSceneLightIntensity(static_cast<float>(value) / 100.0f);
    });
    connect(_ui->distanceSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [this](double value) { _viewport->setSceneLightDistance(static_cast<float>(value)); });
    connect(_ui->attenuationStartSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [this](double) { applyAttenuation(); });
    connect(_ui->attenuationEndSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [this](double) { applyAttenuation(); });
    connect(_ui->attenuationGroupBox, &QGroupBox::toggled, this, [this](bool enabled) {
        updateAttenuationControls(enabled);
        applyAttenuation();
    });

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &SceneLightDialog::reject);
}

SceneLightDialog::~SceneLightDialog()
{
    delete _ui;
}

void SceneLightDialog::reject()
{
    if (_viewport) {
        _viewport->setSceneLightState(_initialState);
    }

    QDialog::reject();
}

void SceneLightDialog::setColorControls(const Vector3 &color)
{
    const QSignalBlocker red_blocker(_ui->redSlider);
    const QSignalBlocker green_blocker(_ui->greenSlider);
    const QSignalBlocker blue_blocker(_ui->blueSlider);
    const QSignalBlocker grayscale_blocker(_ui->grayscaleCheckBox);

    _ui->redSlider->setValue(static_cast<int>(color.X * 100.0f));
    _ui->greenSlider->setValue(static_cast<int>(color.Y * 100.0f));
    _ui->blueSlider->setValue(static_cast<int>(color.Z * 100.0f));
    _ui->grayscaleCheckBox->setChecked(color.X == color.Y && color.X == color.Z);
}

void SceneLightDialog::applyColorFromControls()
{
    const Vector3 color(static_cast<float>(_ui->redSlider->value()) / 100.0f,
                        static_cast<float>(_ui->greenSlider->value()) / 100.0f,
                        static_cast<float>(_ui->blueSlider->value()) / 100.0f);

    if (_currentChannel & Diffuse) {
        _viewport->setSceneLightDiffuse(color);
    }
    if (_currentChannel & Specular) {
        _viewport->setSceneLightSpecular(color);
    }
}

void SceneLightDialog::colorSliderChanged(QSlider *source, int value)
{
    if (_ui->grayscaleCheckBox->isChecked()) {
        const QSignalBlocker red_blocker(_ui->redSlider);
        const QSignalBlocker green_blocker(_ui->greenSlider);
        const QSignalBlocker blue_blocker(_ui->blueSlider);
        if (source != _ui->redSlider) {
            _ui->redSlider->setValue(value);
        }
        if (source != _ui->greenSlider) {
            _ui->greenSlider->setValue(value);
        }
        if (source != _ui->blueSlider) {
            _ui->blueSlider->setValue(value);
        }
    }

    applyColorFromControls();
}

void SceneLightDialog::applyAttenuation()
{
    _viewport->setSceneLightAttenuation(
        static_cast<float>(_ui->attenuationStartSpinBox->value()),
        static_cast<float>(_ui->attenuationEndSpinBox->value()),
        _ui->attenuationGroupBox->isChecked());
}

void SceneLightDialog::updateAttenuationControls(bool enabled)
{
    _ui->attenuationStartLabel->setEnabled(enabled);
    _ui->attenuationStartSpinBox->setEnabled(enabled);
    _ui->attenuationEndLabel->setEnabled(enabled);
    _ui->attenuationEndSpinBox->setEnabled(enabled);
}
