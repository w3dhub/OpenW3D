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

class QTreeWidgetItem;
class RenderObjClass;
class W3DViewport;

namespace Ui {
class BoneManagementDialog;
}

class BoneManagementDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit BoneManagementDialog(RenderObjClass *baseModel,
                                  W3DViewport *viewport,
                                  QWidget *parent = nullptr);
    ~BoneManagementDialog() override;

protected:
    void accept() override;
    void reject() override;

private slots:
    void onBoneSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onObjectSelectionChanged(int index);
    void onAttachClicked();

private:
    void populateBones();
    void populateObjectList();
    void fillBoneItem(QTreeWidgetItem *boneItem, int boneIndex);
    void updateControls(QTreeWidgetItem *selectedItem);
    void updateAttachButton();
    bool isRenderObjAlreadyAttached(const QString &name) const;
    QTreeWidgetItem *currentBoneItem() const;
    void removeObjectFromBone(QTreeWidgetItem *boneItem, const QString &name);

    RenderObjClass *_baseModel = nullptr;
    RenderObjClass *_backupModel = nullptr;
    W3DViewport *_viewport = nullptr;

    Ui::BoneManagementDialog *_ui = nullptr;
    QString _boneName;
    bool _attachMode = true;
};
