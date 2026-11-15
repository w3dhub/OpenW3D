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

#include "CameraDistanceDialog.h"

#include "ui_CameraDistanceDialog.h"

#include <QDialogButtonBox>

CameraDistanceDialog::CameraDistanceDialog(float distance, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::CameraDistanceDialog)
{
    _ui->setupUi(this);
    _ui->distanceSpinBox->setValue(distance);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

CameraDistanceDialog::~CameraDistanceDialog()
{
    delete _ui;
}

float CameraDistanceDialog::distance() const
{
    return static_cast<float>(_ui->distanceSpinBox->value());
}
