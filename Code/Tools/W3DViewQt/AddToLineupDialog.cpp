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

#include "AddToLineupDialog.h"

#include "W3DViewport.h"
#include "ui_AddToLineupDialog.h"

#include "assetmgr.h"
#include "rendobj.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>

AddToLineupDialog::AddToLineupDialog(W3DViewport *viewport, QWidget *parent)
    : QDialog(parent)
    , _viewport(viewport)
    , _ui(new Ui::AddToLineupDialog)
{
    _ui->setupUi(this);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &AddToLineupDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &AddToLineupDialog::reject);

    populateObjects();
}

AddToLineupDialog::~AddToLineupDialog()
{
    delete _ui;
}

QString AddToLineupDialog::selectedName() const
{
    return _ui->objectComboBox->currentText().trimmed();
}

void AddToLineupDialog::accept()
{
    const QString name = selectedName();
    if (name.isEmpty()) {
        QMessageBox::information(this, "Add To Lineup", "Please select an object or enter a name.");
        return;
    }

    QDialog::accept();
}

void AddToLineupDialog::populateObjects()
{
    if (!_viewport) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    RenderObjIterator *iterator = asset_manager->Create_Render_Obj_Iterator();
    if (!iterator) {
        return;
    }

    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const int class_id = iterator->Current_Item_Class_ID();
        if (!_viewport->canLineUpClass(class_id)) {
            continue;
        }
        const char *name = iterator->Current_Item_Name();
        if (name && name[0]) {
            _ui->objectComboBox->addItem(QString::fromLatin1(name));
        }
    }

    asset_manager->Release_Render_Obj_Iterator(iterator);
}
