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

#include "vector3.h"

#include <QDialog>
#include <QString>

#include <functional>

class QSlider;
namespace Ui {
class ColorLightDialog;
}

class ColorLightDialog final : public QDialog
{
    Q_OBJECT

public:
    using ApplyCallback = std::function<void(const Vector3 &)>;

    explicit ColorLightDialog(const QString &title,
                              const Vector3 &initialColor,
                              ApplyCallback applyCallback,
                              QWidget *parent = nullptr);
    ~ColorLightDialog() override;

    Vector3 selectedColor() const;

public slots:
    void reject() override;

private:
    void colorSliderChanged(QSlider *source, int value);
    void applySelectedColor();
    void updateValueLabels();

    Ui::ColorLightDialog *_ui = nullptr;
    Vector3 _initialColor;
    ApplyCallback _applyCallback;
};
