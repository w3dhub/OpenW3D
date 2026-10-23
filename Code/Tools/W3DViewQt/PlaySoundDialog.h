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

class AudibleSoundClass;

namespace Ui {
class PlaySoundDialog;
}

class PlaySoundDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit PlaySoundDialog(const QString &filename, QWidget *parent = nullptr);
    ~PlaySoundDialog() override;

    bool isReady() const;

private slots:
    void playSound();
    void stopSound();

private:
    bool createSound();

    Ui::PlaySoundDialog *_ui = nullptr;
    QString _filename;
    AudibleSoundClass *_sound = nullptr;
};
