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

#include "sphereobj.h"

namespace Ui {
class OpacityVectorEditDialog;
}

class OpacityVectorEditDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit OpacityVectorEditDialog(const AlphaVectorStruct &value, QWidget *parent = nullptr);
    ~OpacityVectorEditDialog() override;

    AlphaVectorStruct value() const;

private slots:
    void handleIntensitySlider(int value);
    void handleIntensitySpin(double value);
    void handleAngleChanged();

private:
    void syncFromValue();
    float sliderPositionFromIntensity(float intensity) const;
    float intensityFromSliderPosition(float position) const;
    void updateValueFromControls();

    Ui::OpacityVectorEditDialog *_ui = nullptr;
    AlphaVectorStruct _value;
};
