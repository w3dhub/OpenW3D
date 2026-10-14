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
