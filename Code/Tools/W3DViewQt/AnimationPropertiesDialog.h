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
class AnimationPropertiesDialog;
}

class AnimationPropertiesDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AnimationPropertiesDialog(const QString &animationName, QWidget *parent = nullptr);
    ~AnimationPropertiesDialog() override;

private:
    void setErrorState(const QString &message);

    Ui::AnimationPropertiesDialog *_ui = nullptr;
};
