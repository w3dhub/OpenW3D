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
class AnimatedSoundOptionsDialog;
}

class AnimatedSoundOptionsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AnimatedSoundOptionsDialog(const QString &definitionLibraryPath,
                                        const QString &iniPath,
                                        const QString &dataPath,
                                        QWidget *parent = nullptr);
    ~AnimatedSoundOptionsDialog() override;

    QString definitionLibraryPath() const;
    QString iniPath() const;
    QString dataPath() const;

    static void LoadAnimatedSoundSettings();

private slots:
    void browseDefinitionLibrary();
    void browseIniPath();
    void browseDataPath();

private:
    Ui::AnimatedSoundOptionsDialog *_ui = nullptr;
};
