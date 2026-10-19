/*
** Copyright 2026 OpenW3D contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed WITHOUT ANY WARRANTY; without even the
** implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "CameraSettingsDialog.h"

#include "W3DViewport.h"
#include "ui_CameraSettingsDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QtMath>
#include <cmath>

namespace {
constexpr double kLensConstant = 18.0 / 1000.0;
} // namespace

CameraSettingsDialog::CameraSettingsDialog(W3DViewport *viewport, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::CameraSettingsDialog)
    , _viewport(viewport)
{
    _ui->setupUi(this);

    connect(_ui->clipCheckBox, &QCheckBox::toggled,
            this, &CameraSettingsDialog::onClipCheckChanged);
    connect(_ui->fovCheckBox, &QCheckBox::toggled,
            this, &CameraSettingsDialog::onFovCheckChanged);
    connect(_ui->lensSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &CameraSettingsDialog::onLensChanged);
    connect(_ui->hfovSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &CameraSettingsDialog::onHfovChanged);
    auto *reset_button = _ui->buttonBox->button(QDialogButtonBox::Reset);
    connect(reset_button, &QPushButton::clicked, this, &CameraSettingsDialog::onReset);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshFromViewport();
}

CameraSettingsDialog::~CameraSettingsDialog()
{
    delete _ui;
}

bool CameraSettingsDialog::isManualFovEnabled() const
{
    return _ui->fovCheckBox->isChecked();
}

bool CameraSettingsDialog::isManualClipPlanesEnabled() const
{
    return _ui->clipCheckBox->isChecked();
}

double CameraSettingsDialog::hfovDegrees() const
{
    return _ui->hfovSpinBox->value();
}

double CameraSettingsDialog::vfovDegrees() const
{
    return _ui->vfovSpinBox->value();
}

double CameraSettingsDialog::lensMm() const
{
    return _ui->lensSpinBox->value();
}

float CameraSettingsDialog::nearClip() const
{
    return static_cast<float>(_ui->nearClipSpinBox->value());
}

float CameraSettingsDialog::farClip() const
{
    return static_cast<float>(_ui->farClipSpinBox->value());
}

void CameraSettingsDialog::onFovCheckChanged(bool checked)
{
    setFovControlsEnabled(checked);
}

void CameraSettingsDialog::onClipCheckChanged(bool checked)
{
    setClipControlsEnabled(checked);
}

void CameraSettingsDialog::onReset()
{
    if (_viewport) {
        _viewport->setManualFovEnabled(false);
        _viewport->setManualClipPlanesEnabled(false);
        _viewport->resetFov();
        _viewport->resetCamera();
    }

    refreshFromViewport();
}

void CameraSettingsDialog::onHfovChanged(double value)
{
    Q_UNUSED(value);
    updateLensFromHfov();
}

void CameraSettingsDialog::onLensChanged(double value)
{
    Q_UNUSED(value);
    updateFovFromLens();
}

void CameraSettingsDialog::refreshFromViewport()
{
    if (!_viewport) {
        return;
    }

    const bool manual_fov = _viewport->isManualFovEnabled();
    const bool manual_clip = _viewport->isManualClipPlanesEnabled();
    _ui->fovCheckBox->setChecked(manual_fov);
    _ui->clipCheckBox->setChecked(manual_clip);

    double hfov_deg = 0.0;
    double vfov_deg = 0.0;
    _viewport->cameraFovDegrees(hfov_deg, vfov_deg);
    _ui->hfovSpinBox->setValue(hfov_deg);
    _ui->vfovSpinBox->setValue(vfov_deg);

    updateLensFromHfov();

    float znear = 0.0f;
    float zfar = 0.0f;
    _viewport->cameraClipPlanes(znear, zfar);
    _ui->nearClipSpinBox->setValue(znear);
    _ui->farClipSpinBox->setValue(zfar);

    setFovControlsEnabled(manual_fov);
    setClipControlsEnabled(manual_clip);
}

void CameraSettingsDialog::updateLensFromHfov()
{
    if (_updating) {
        return;
    }

    _updating = true;
    const double hfov_rad = qDegreesToRadians(_ui->hfovSpinBox->value());
    if (hfov_rad > 0.0) {
        const double lens = (kLensConstant / std::tan(hfov_rad / 2.0)) * 1000.0;
        _ui->lensSpinBox->setValue(lens);
    }
    _updating = false;
}

void CameraSettingsDialog::updateFovFromLens()
{
    if (_updating) {
        return;
    }

    _updating = true;
    const double lens = _ui->lensSpinBox->value() / 1000.0;
    if (lens > 0.0) {
        const double hfov = std::atan(kLensConstant / lens) * 2.0;
        const double vfov = (3.0 * hfov) / 4.0;
        _ui->hfovSpinBox->setValue(qRadiansToDegrees(hfov));
        _ui->vfovSpinBox->setValue(qRadiansToDegrees(vfov));
    }
    _updating = false;
}

void CameraSettingsDialog::setFovControlsEnabled(bool enabled)
{
    _ui->hfovSpinBox->setEnabled(enabled);
    _ui->vfovSpinBox->setEnabled(enabled);
    _ui->lensSpinBox->setEnabled(enabled);
}

void CameraSettingsDialog::setClipControlsEnabled(bool enabled)
{
    _ui->nearClipSpinBox->setEnabled(enabled);
    _ui->farClipSpinBox->setEnabled(enabled);
}
