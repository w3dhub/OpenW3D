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

class SoundRenderObjClass;

namespace Ui {
class SoundEditDialog;
}

class SoundEditDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SoundEditDialog(SoundRenderObjClass *sound, QWidget *parent = nullptr);
    ~SoundEditDialog() override;

    SoundRenderObjClass *sound() const;
    QString oldName() const;

protected:
    void accept() override;

private slots:
    void browseSoundFile();
    void toggleSoundType();
    void playSound();

private:
    void loadFromSound();
    void updateEnableState();

    Ui::SoundEditDialog *_ui = nullptr;
    SoundRenderObjClass *_sound = nullptr;
    QString _oldName;
};
