#include "BoneManagementDialog.h"

#include "ui_BoneManagementDialog.h"

#include "RenderObjUtils.h"
#include "W3DViewport.h"

#include "assetmgr.h"
#include "rendobj.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <algorithm>

namespace {
QString NormalizeName(const char *name)
{
    return name ? QString::fromLatin1(name) : QString();
}
}

BoneManagementDialog::BoneManagementDialog(RenderObjClass *baseModel,
                                           W3DViewport *viewport,
                                           QWidget *parent)
    : QDialog(parent)
    , _baseModel(baseModel)
    , _viewport(viewport)
    , _ui(new Ui::BoneManagementDialog)
{
    _ui->setupUi(this);

    if (_baseModel) {
        _baseModel->Add_Ref();
        _backupModel = _baseModel->Clone();
    }

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &BoneManagementDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &BoneManagementDialog::reject);

    connect(_ui->boneTree,
            &QTreeWidget::currentItemChanged,
            this,
            &BoneManagementDialog::onBoneSelectionChanged);
    connect(_ui->objectCombo,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &BoneManagementDialog::onObjectSelectionChanged);
    connect(_ui->attachButton, &QPushButton::clicked, this, &BoneManagementDialog::onAttachClicked);

    populateBones();
    populateObjectList();

    if (_ui->boneTree->topLevelItemCount() > 0) {
        _ui->boneTree->setCurrentItem(_ui->boneTree->topLevelItem(0));
    } else {
        updateControls(nullptr);
    }
}

BoneManagementDialog::~BoneManagementDialog()
{
    if (_baseModel) {
        _baseModel->Release_Ref();
    }
    if (_backupModel) {
        _backupModel->Release_Ref();
    }

    delete _ui;
}

void BoneManagementDialog::accept()
{
    if (_baseModel) {
        UpdateAggregatePrototype(*_baseModel);
    }

    QDialog::accept();
}

void BoneManagementDialog::reject()
{
    if (_backupModel && _viewport) {
        _viewport->clearAnimation();
        _viewport->setRenderObject(_backupModel);
    }

    QDialog::reject();
}

void BoneManagementDialog::onBoneSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    updateControls(current);
}

void BoneManagementDialog::onObjectSelectionChanged(int index)
{
    Q_UNUSED(index);
    updateAttachButton();
}

void BoneManagementDialog::onAttachClicked()
{
    if (!_baseModel || _boneName.isEmpty()) {
        return;
    }

    const QString object_name = _ui->objectCombo->currentText();
    if (object_name.isEmpty()) {
        return;
    }

    QTreeWidgetItem *bone_item = currentBoneItem();
    if (!bone_item) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    if (_attachMode) {
        const QByteArray name_bytes = object_name.toLatin1();
        RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
        if (render_obj) {
            _baseModel->Add_Sub_Object_To_Bone(render_obj, _boneName.toLatin1().constData());
            auto *child = new QTreeWidgetItem(bone_item);
            child->setText(0, object_name);
            render_obj->Release_Ref();
        }
    } else {
        const int bone_index = _baseModel->Get_Bone_Index(_boneName.toLatin1().constData());
        if (bone_index >= 0) {
            const int count = _baseModel->Get_Num_Sub_Objects_On_Bone(bone_index);
            for (int index = 0; index < count; ++index) {
                RenderObjClass *sub_obj = _baseModel->Get_Sub_Object_On_Bone(index, bone_index);
                if (!sub_obj) {
                    continue;
                }

                const QString sub_name = NormalizeName(sub_obj->Get_Name());
                if (sub_name.compare(object_name, Qt::CaseInsensitive) == 0) {
                    _baseModel->Remove_Sub_Object(sub_obj);
                    sub_obj->Release_Ref();
                    removeObjectFromBone(bone_item, object_name);
                    break;
                }

                sub_obj->Release_Ref();
            }
        }
    }

    _ui->boneTree->setCurrentItem(bone_item);
    updateControls(bone_item);
}

void BoneManagementDialog::populateBones()
{
    if (!_baseModel) {
        return;
    }

    const int bone_count = _baseModel->Get_Num_Bones();
    for (int index = 0; index < bone_count; ++index) {
        const char *bone_name = _baseModel->Get_Bone_Name(index);
        if (!bone_name || !bone_name[0]) {
            continue;
        }

        auto *bone_item = new QTreeWidgetItem(_ui->boneTree);
        bone_item->setText(0, QString::fromLatin1(bone_name));
        fillBoneItem(bone_item, index);
    }

    _ui->boneTree->sortItems(0, Qt::AscendingOrder);
}

void BoneManagementDialog::populateObjectList()
{
    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    RenderObjIterator *iterator = asset_manager->Create_Render_Obj_Iterator();
    if (!iterator) {
        return;
    }

    QStringList names;
    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const char *name = iterator->Current_Item_Name();
        if (!name || !name[0]) {
            continue;
        }

        if (!asset_manager->Render_Obj_Exists(name)) {
            continue;
        }

        names.append(QString::fromLatin1(name));
    }

    asset_manager->Release_Render_Obj_Iterator(iterator);

    names.removeDuplicates();
    std::sort(names.begin(), names.end(), [](const QString &a, const QString &b) {
        return QString::compare(a, b, Qt::CaseInsensitive) < 0;
    });

    _ui->objectCombo->clear();
    _ui->objectCombo->addItems(names);
    if (_ui->objectCombo->count() > 0) {
        _ui->objectCombo->setCurrentIndex(0);
    }
}

void BoneManagementDialog::fillBoneItem(QTreeWidgetItem *boneItem, int boneIndex)
{
    if (!boneItem || !_baseModel) {
        return;
    }

    const char *base_name = _baseModel->Get_Base_Model_Name();
    if (!base_name || !base_name[0]) {
        base_name = _baseModel->Get_Name();
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager || !base_name) {
        return;
    }

    RenderObjClass *orig_model = asset_manager->Create_Render_Obj(base_name);
    if (!orig_model) {
        return;
    }

    QStringList original_names;
    const int orig_count = orig_model->Get_Num_Sub_Objects_On_Bone(boneIndex);
    for (int index = 0; index < orig_count; ++index) {
        RenderObjClass *sub_obj = orig_model->Get_Sub_Object_On_Bone(index, boneIndex);
        if (!sub_obj) {
            continue;
        }

        original_names.append(NormalizeName(sub_obj->Get_Name()));
        sub_obj->Release_Ref();
    }

    const int count = _baseModel->Get_Num_Sub_Objects_On_Bone(boneIndex);
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = _baseModel->Get_Sub_Object_On_Bone(index, boneIndex);
        if (!sub_obj) {
            continue;
        }

        const QString sub_name = NormalizeName(sub_obj->Get_Name());
        const bool exists = original_names.contains(sub_name, Qt::CaseInsensitive);
        if (!exists && !sub_name.isEmpty()) {
            auto *child = new QTreeWidgetItem(boneItem);
            child->setText(0, sub_name);
        }

        sub_obj->Release_Ref();
    }

    orig_model->Release_Ref();
}

void BoneManagementDialog::updateControls(QTreeWidgetItem *selectedItem)
{
    if (!selectedItem) {
        _boneName.clear();
        _ui->boneGroup->setTitle("Bone:");
        _ui->attachButton->setEnabled(false);
        return;
    }

    QTreeWidgetItem *bone_item = selectedItem->parent() ? selectedItem->parent() : selectedItem;
    _boneName = bone_item->text(0);
    _ui->boneGroup->setTitle(QString("Bone: %1").arg(_boneName));

    if (selectedItem->parent()) {
        const QString child_name = selectedItem->text(0);
        const int index = _ui->objectCombo->findText(child_name, Qt::MatchFixedString);
        if (index >= 0) {
            _ui->objectCombo->setCurrentIndex(index);
        }
    }

    updateAttachButton();
}

void BoneManagementDialog::updateAttachButton()
{
    const QString current_name = _ui->objectCombo->currentText();
    if (_boneName.isEmpty() || current_name.isEmpty()) {
        _ui->attachButton->setEnabled(false);
        return;
    }

    if (isRenderObjAlreadyAttached(current_name)) {
        _ui->attachButton->setText("Remove");
        _attachMode = false;
    } else {
        _ui->attachButton->setText("Attach");
        _attachMode = true;
    }

    _ui->attachButton->setEnabled(true);
}

bool BoneManagementDialog::isRenderObjAlreadyAttached(const QString &name) const
{
    QTreeWidgetItem *bone_item = currentBoneItem();
    if (!bone_item) {
        return false;
    }

    const int count = bone_item->childCount();
    for (int index = 0; index < count; ++index) {
        QTreeWidgetItem *child = bone_item->child(index);
        if (child && child->text(0).compare(name, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

QTreeWidgetItem *BoneManagementDialog::currentBoneItem() const
{
    QTreeWidgetItem *current = _ui->boneTree->currentItem();
    if (!current) {
        return nullptr;
    }

    return current->parent() ? current->parent() : current;
}

void BoneManagementDialog::removeObjectFromBone(QTreeWidgetItem *boneItem, const QString &name)
{
    if (!boneItem) {
        return;
    }

    const int count = boneItem->childCount();
    for (int index = 0; index < count; ++index) {
        QTreeWidgetItem *child = boneItem->child(index);
        if (child && child->text(0).compare(name, Qt::CaseInsensitive) == 0) {
            delete boneItem->takeChild(index);
            break;
        }
    }
}
