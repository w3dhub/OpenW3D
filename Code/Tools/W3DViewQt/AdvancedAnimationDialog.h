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
#include <QVector>

class HAnimClass;
class W3DViewport;

namespace Ui {
class AdvancedAnimationDialog;
}

class AdvancedAnimationDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedAnimationDialog(W3DViewport *viewport,
                                     const QString &renderObjectName,
                                     QWidget *parent = nullptr);
    ~AdvancedAnimationDialog() override;

protected:
    void accept() override;

private slots:
    void updateReport();
    void onTabChanged(int index);

private:
    void loadAnimations();
    void populateMixingList();
    QString makeChannelString(int boneIndex, HAnimClass *anim) const;

    Ui::AdvancedAnimationDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
    QString _renderObjectName;
    QVector<HAnimClass *> _animations;
    bool _hasHierarchy = false;
    QString _hierarchyName;
};
