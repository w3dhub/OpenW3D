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

#include "BackgroundBitmapDialog.h"

#include "ui_BackgroundBitmapDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

BackgroundBitmapDialog::BackgroundBitmapDialog(const QString &currentPath, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::BackgroundBitmapDialog)
{
    _ui->setupUi(this);
    _ui->pathLineEdit->setText(QDir::toNativeSeparators(currentPath));

    connect(_ui->browseButton, &QPushButton::clicked, this, &BackgroundBitmapDialog::browse);
    connect(_ui->clearButton, &QPushButton::clicked, this,
            &BackgroundBitmapDialog::clearPath);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

BackgroundBitmapDialog::~BackgroundBitmapDialog()
{
    delete _ui;
}

QString BackgroundBitmapDialog::selectedPath() const
{
    return _ui->pathLineEdit->text().trimmed();
}

void BackgroundBitmapDialog::browse()
{
    QString startPath = selectedPath();
    if (startPath.isEmpty()) {
        startPath = QDir::currentPath();
    }

    const QString path = QFileDialog::getOpenFileName(
        this,
        "Background Bitmap",
        startPath,
        "Images (*.bmp *.tga *.dds);;Targa Images (*.tga);;All Files (*.*)");
    if (!path.isEmpty()) {
        _ui->pathLineEdit->setText(QDir::toNativeSeparators(path));
    }
}

void BackgroundBitmapDialog::clearPath()
{
    _ui->pathLineEdit->clear();
    _ui->pathLineEdit->setFocus();
}
