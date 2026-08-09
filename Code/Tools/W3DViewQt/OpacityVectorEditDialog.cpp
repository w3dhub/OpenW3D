#include "OpacityVectorEditDialog.h"

#include "ui_OpacityVectorEditDialog.h"

#include "euler.h"
#include "matrix3.h"
#include "quat.h"
#include "wwmath.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QSpinBox>
#include <algorithm>
#include <cmath>

namespace {
constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
constexpr float kRadToDeg = 180.0f / 3.14159265358979323846f;

void QuaternionToAngles(const Quaternion &quat, float &y_deg, float &z_deg)
{
    Matrix3D rotation = Build_Matrix3D(quat);
    EulerAnglesClass euler(rotation, EulerOrderXYZr);
    y_deg = static_cast<float>(euler.Get_Angle(1) * kRadToDeg);
    z_deg = static_cast<float>(euler.Get_Angle(2) * kRadToDeg);
    y_deg = static_cast<float>(WWMath::Wrap(y_deg, 0.0f, 360.0f));
    z_deg = static_cast<float>(WWMath::Wrap(z_deg, 0.0f, 360.0f));
}
}

OpacityVectorEditDialog::OpacityVectorEditDialog(const AlphaVectorStruct &value, QWidget *parent)
    : QDialog(parent),
      _ui(new Ui::OpacityVectorEditDialog),
      _value(value)
{
    _ui->setupUi(this);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &OpacityVectorEditDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &OpacityVectorEditDialog::reject);
    connect(_ui->intensitySlider, &QSlider::valueChanged, this,
            &OpacityVectorEditDialog::handleIntensitySlider);
    connect(_ui->intensitySpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &OpacityVectorEditDialog::handleIntensitySpin);
    connect(_ui->angleYSpin, qOverload<int>(&QSpinBox::valueChanged), this,
            &OpacityVectorEditDialog::handleAngleChanged);
    connect(_ui->angleZSpin, qOverload<int>(&QSpinBox::valueChanged), this,
            &OpacityVectorEditDialog::handleAngleChanged);

    syncFromValue();
}

OpacityVectorEditDialog::~OpacityVectorEditDialog()
{
    delete _ui;
}

AlphaVectorStruct OpacityVectorEditDialog::value() const
{
    return _value;
}

void OpacityVectorEditDialog::handleIntensitySlider(int value)
{
    const float position = static_cast<float>(value) / 10.0f;
    const float intensity = intensityFromSliderPosition(position);
    _ui->intensitySpin->blockSignals(true);
    _ui->intensitySpin->setValue(intensity);
    _ui->intensitySpin->blockSignals(false);
    updateValueFromControls();
}

void OpacityVectorEditDialog::handleIntensitySpin(double value)
{
    const float position = sliderPositionFromIntensity(static_cast<float>(value));
    _ui->intensitySlider->blockSignals(true);
    _ui->intensitySlider->setValue(static_cast<int>(position * 10.0f));
    _ui->intensitySlider->blockSignals(false);
    updateValueFromControls();
}

void OpacityVectorEditDialog::handleAngleChanged()
{
    updateValueFromControls();
}

void OpacityVectorEditDialog::syncFromValue()
{
    float y_deg = 0.0f;
    float z_deg = 0.0f;
    QuaternionToAngles(_value.angle, y_deg, z_deg);

    _ui->angleYSpin->setValue(static_cast<int>(std::clamp(y_deg, 0.0f, 179.0f)));
    _ui->angleZSpin->setValue(static_cast<int>(std::clamp(z_deg, 0.0f, 179.0f)));
    _ui->intensitySpin->setValue(_value.intensity);

    const float position = sliderPositionFromIntensity(_value.intensity);
    _ui->intensitySlider->setValue(static_cast<int>(position * 10.0f));
}

float OpacityVectorEditDialog::sliderPositionFromIntensity(float intensity) const
{
    const float percent = std::clamp(intensity / 10.0f, 0.0f, 1.0f);
    const float pos = std::atan(percent * 11.0f) / (84.5f * kDegToRad) * 10.0f;
    return std::clamp(pos, 0.0f, 10.0f);
}

float OpacityVectorEditDialog::intensityFromSliderPosition(float position) const
{
    const float percent = std::tan((position / 10.0f) * 84.5f * kDegToRad) / 11.0f;
    return 10.0f * std::clamp(percent, 0.0f, 1.0f);
}

void OpacityVectorEditDialog::updateValueFromControls()
{
    const float intensity = static_cast<float>(_ui->intensitySpin->value());
    const float y_deg = static_cast<float>(_ui->angleYSpin->value());
    const float z_deg = static_cast<float>(_ui->angleZSpin->value());

    Matrix3 rot_mat(true);
    rot_mat.Rotate_Y(y_deg * kDegToRad);
    rot_mat.Rotate_Z(z_deg * kDegToRad);

    _value.angle = Build_Quaternion(rot_mat);
    _value.intensity = intensity;
}
