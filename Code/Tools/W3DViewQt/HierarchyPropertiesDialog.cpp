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

#include "HierarchyPropertiesDialog.h"

#include "MeshPropertiesDialog.h"
#include "ui_HierarchyPropertiesDialog.h"

#include "assetmgr.h"
#include "rendobj.h"

#include <QDialogButtonBox>
#include <QTreeWidgetItem>

HierarchyPropertiesDialog::HierarchyPropertiesDialog(const QString &hierarchyName, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::HierarchyPropertiesDialog)
    , _hierarchyName(hierarchyName)
{
    _ui->setupUi(this);
    connect(_ui->subObjectList,
            &QTreeWidget::itemDoubleClicked,
            this,
            &HierarchyPropertiesDialog::showSubObjectProperties);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (_hierarchyName.isEmpty()) {
        setErrorState("No hierarchy selected.");
        return;
    }

    _ui->descriptionLabel->setText(QString("Hierarchy: %1").arg(_hierarchyName));

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        setErrorState("WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = _hierarchyName.toLatin1();
    RenderObjClass *hierarchy = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!hierarchy) {
        setErrorState("Failed to load hierarchy.");
        return;
    }

    _ui->polygonCountValue->setText(QString::number(hierarchy->Get_Num_Polys()));

    const int sub_count = hierarchy->Get_Num_Sub_Objects();
    _ui->subObjectCountValue->setText(QString::number(sub_count));

    for (int index = 0; index < sub_count; ++index) {
        RenderObjClass *sub_obj = hierarchy->Get_Sub_Object(index);
        if (!sub_obj) {
            continue;
        }

        const char *sub_name = sub_obj->Get_Name();
        if (sub_name && sub_name[0]) {
            auto *item = new QTreeWidgetItem(_ui->subObjectList);
            item->setText(0, QString::fromLatin1(sub_name));
        }

        sub_obj->Release_Ref();
    }

    _ui->subObjectList->resizeColumnToContents(0);
    hierarchy->Release_Ref();
}

HierarchyPropertiesDialog::~HierarchyPropertiesDialog()
{
    delete _ui;
}

void HierarchyPropertiesDialog::showSubObjectProperties(QTreeWidgetItem *item, int column)
{
    if (!item || column != 0) {
        return;
    }

    const QString name = item->text(0);
    if (name.isEmpty()) {
        return;
    }

    MeshPropertiesDialog dialog(name, this);
    dialog.exec();
}

void HierarchyPropertiesDialog::setErrorState(const QString &message)
{
    _ui->descriptionLabel->setText(message);
    _ui->polygonCountValue->setText("n/a");
    _ui->subObjectCountValue->setText("n/a");
    _ui->subObjectList->setEnabled(false);
}
