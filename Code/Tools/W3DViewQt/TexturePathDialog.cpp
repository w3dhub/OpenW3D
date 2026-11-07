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

#include "TexturePathDialog.h"

#include "ui_TexturePathDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>

TexturePathDialog::TexturePathDialog(const QString &path1, const QString &path2, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::TexturePathDialog)
{
    _ui->setupUi(this);
    _ui->path1LineEdit->setText(QDir::toNativeSeparators(path1));
    _ui->path2LineEdit->setText(QDir::toNativeSeparators(path2));

    connect(_ui->path1BrowseButton, &QPushButton::clicked, this, &TexturePathDialog::browsePath1);
    connect(_ui->path2BrowseButton, &QPushButton::clicked, this, &TexturePathDialog::browsePath2);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

TexturePathDialog::~TexturePathDialog()
{
    delete _ui;
}

QString TexturePathDialog::path1() const
{
    return _ui->path1LineEdit->text().trimmed();
}

QString TexturePathDialog::path2() const
{
    return _ui->path2LineEdit->text().trimmed();
}

void TexturePathDialog::browsePath1()
{
    const QString start = _ui->path1LineEdit->text();
    const QString dir = QFileDialog::getExistingDirectory(this, "Texture Path 1", start);
    if (!dir.isEmpty()) {
        _ui->path1LineEdit->setText(QDir::toNativeSeparators(dir));
    }
}

void TexturePathDialog::browsePath2()
{
    const QString start = _ui->path2LineEdit->text();
    const QString dir = QFileDialog::getExistingDirectory(this, "Texture Path 2", start);
    if (!dir.isEmpty()) {
        _ui->path2LineEdit->setText(QDir::toNativeSeparators(dir));
    }
}
