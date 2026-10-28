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

#pragma once

#include <QDialog>

class W3DViewport;

namespace Ui {
class CameraSettingsDialog;
}

class CameraSettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit CameraSettingsDialog(W3DViewport *viewport, QWidget *parent = nullptr);
    ~CameraSettingsDialog() override;

    bool isManualFovEnabled() const;
    bool isManualClipPlanesEnabled() const;
    double hfovDegrees() const;
    double vfovDegrees() const;
    double lensMm() const;
    float nearClip() const;
    float farClip() const;

private slots:
    void onFovCheckChanged(bool checked);
    void onClipCheckChanged(bool checked);
    void onReset();
    void onHfovChanged(double value);
    void onLensChanged(double value);

private:
    void refreshFromViewport();
    void updateLensFromHfov();
    void updateFovFromLens();
    void setFovControlsEnabled(bool enabled);
    void setClipControlsEnabled(bool enabled);

    Ui::CameraSettingsDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
    bool _updating = false;
};
