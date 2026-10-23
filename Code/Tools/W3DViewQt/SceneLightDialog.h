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

#include "W3DViewport.h"

#include <QDialog>

class QSlider;
namespace Ui {
class SceneLightDialog;
}

class SceneLightDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SceneLightDialog(W3DViewport &viewport, QWidget *parent = nullptr);
    ~SceneLightDialog() override;

public slots:
    void reject() override;

private:
    enum Channel {
        Diffuse = 1 << 0,
        Specular = 1 << 1,
        Both = Diffuse | Specular,
    };

    void setColorControls(const Vector3 &color);
    void applyColorFromControls();
    void colorSliderChanged(QSlider *source, int value);
    void applyAttenuation();
    void updateAttenuationControls(bool enabled);

    Ui::SceneLightDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
    Channel _currentChannel = Diffuse;
    W3DViewport::SceneLightState _initialState;
};
