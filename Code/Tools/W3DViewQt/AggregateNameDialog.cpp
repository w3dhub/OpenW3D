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

#include "AggregateNameDialog.h"

#include "ui_AggregateNameDialog.h"

#include "w3d_file.h"

#include <QDialogButtonBox>

AggregateNameDialog::AggregateNameDialog(const QString &title,
                                         const QString &defaultName,
                                         QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AggregateNameDialog)
{
    _ui->setupUi(this);
    setWindowTitle(title);
    _ui->nameLineEdit->setMaxLength(W3D_NAME_LEN - 1);
    _ui->nameLineEdit->setText(defaultName);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AggregateNameDialog::~AggregateNameDialog()
{
    delete _ui;
}

QString AggregateNameDialog::name() const
{
    return _ui->nameLineEdit->text();
}
