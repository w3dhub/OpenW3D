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
#include <QString>

namespace Ui {
class TexturePathDialog;
}

class TexturePathDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit TexturePathDialog(const QString &path1, const QString &path2, QWidget *parent = nullptr);
    ~TexturePathDialog() override;

    QString path1() const;
    QString path2() const;

private slots:
    void browsePath1();
    void browsePath2();

private:
    Ui::TexturePathDialog *_ui = nullptr;
};
