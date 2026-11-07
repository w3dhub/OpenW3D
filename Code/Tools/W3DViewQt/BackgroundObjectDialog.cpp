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

#include "BackgroundObjectDialog.h"

#include "ui_BackgroundObjectDialog.h"

#include "assetmgr.h"
#include "rendobj.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>

#include <algorithm>

BackgroundObjectDialog::BackgroundObjectDialog(const QString &currentName, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::BackgroundObjectDialog)
{
    _ui->setupUi(this);
    _ui->buttonBox->addButton(_ui->clearButton, QDialogButtonBox::ResetRole);

    connect(_ui->listWidget, &QListWidget::itemSelectionChanged, this,
            &BackgroundObjectDialog::onSelectionChanged);
    connect(_ui->clearButton, &QPushButton::clicked, this, &BackgroundObjectDialog::onClear);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    RenderObjIterator *iterator = asset_manager->Create_Render_Obj_Iterator();
    if (!iterator) {
        return;
    }

    QStringList object_names;
    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const char *name = iterator->Current_Item_Name();
        if (!name || !name[0]) {
            continue;
        }

        if (!asset_manager->Render_Obj_Exists(name)) {
            continue;
        }

        if (iterator->Current_Item_Class_ID() != RenderObjClass::CLASSID_HMODEL) {
            continue;
        }

        object_names.push_back(QString::fromLatin1(name));
    }

    asset_manager->Release_Render_Obj_Iterator(iterator);

    std::sort(object_names.begin(), object_names.end(), [](const QString &left,
                                                           const QString &right) {
        const int case_insensitive = QString::compare(left, right, Qt::CaseInsensitive);
        if (case_insensitive != 0) {
            return case_insensitive < 0;
        }

        return QString::compare(left, right, Qt::CaseSensitive) < 0;
    });
    _ui->listWidget->addItems(object_names);

    if (!currentName.isEmpty()) {
        const QList<QListWidgetItem *> matches =
            _ui->listWidget->findItems(currentName, Qt::MatchFixedString);
        if (!matches.isEmpty()) {
            _ui->listWidget->setCurrentItem(matches.front());
        }
    }

    if (!_ui->listWidget->currentItem() && _ui->listWidget->count() > 0) {
        _ui->listWidget->setCurrentRow(0);
    }

    onSelectionChanged();
}

BackgroundObjectDialog::~BackgroundObjectDialog()
{
    delete _ui;
}

QString BackgroundObjectDialog::selectedName() const
{
    const QList<QListWidgetItem *> selected_items = _ui->listWidget->selectedItems();
    return selected_items.isEmpty() ? QString() : selected_items.front()->text();
}

void BackgroundObjectDialog::onSelectionChanged()
{
    const QString name = selectedName();
    _ui->currentLabel->setText(name.isEmpty() ? "Current Object: (none)"
                                              : QString("Current Object: %1").arg(name));
}

void BackgroundObjectDialog::onClear()
{
    _ui->listWidget->clearSelection();
    _ui->listWidget->setCurrentRow(-1);
    onSelectionChanged();
}
