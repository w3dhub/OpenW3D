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

#include "ScaleDialog.h"

#include "ui_ScaleDialog.h"

#include <QDialogButtonBox>
#include <QMessageBox>

ScaleDialog::ScaleDialog(double scale, const QString &prompt, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::ScaleDialog)
{
    _ui->setupUi(this);
    _ui->promptLabel->setText(prompt);
    _ui->scaleSpinBox->setValue(scale);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &ScaleDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ScaleDialog::~ScaleDialog()
{
    delete _ui;
}

double ScaleDialog::scale() const
{
    return _ui->scaleSpinBox->value();
}

void ScaleDialog::accept()
{
    if (_ui->scaleSpinBox->value() <= 0.0) {
        QMessageBox::information(this, "Invalid Scale", "Scale must be a value greater than zero.");
        return;
    }

    QDialog::accept();
}
