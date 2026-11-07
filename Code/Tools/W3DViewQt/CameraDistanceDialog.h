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

namespace Ui {
class CameraDistanceDialog;
}

class CameraDistanceDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit CameraDistanceDialog(float distance, QWidget *parent = nullptr);
    ~CameraDistanceDialog() override;

    float distance() const;

private:
    Ui::CameraDistanceDialog *_ui = nullptr;
};
