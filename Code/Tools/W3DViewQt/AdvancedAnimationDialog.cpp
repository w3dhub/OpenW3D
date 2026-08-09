#include "AdvancedAnimationDialog.h"

#include "W3DViewport.h"
#include "ui_AdvancedAnimationDialog.h"

#include "assetmgr.h"
#include "hanim.h"
#include "htree.h"
#include "rendobj.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QMessageBox>
#include <QTabWidget>
#include <QTableWidget>
#include <algorithm>

namespace {
constexpr int kMaxReportAnims = 128;

QString AnimName(HAnimClass *anim)
{
    return anim && anim->Get_Name() ? QString::fromLatin1(anim->Get_Name()) : QString();
}
}

AdvancedAnimationDialog::AdvancedAnimationDialog(W3DViewport *viewport,
                                                 const QString &renderObjectName,
                                                 QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AdvancedAnimationDialog)
    , _viewport(viewport)
    , _renderObjectName(renderObjectName)
{
    _ui->setupUi(this);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &AdvancedAnimationDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(_ui->tabWidget, &QTabWidget::currentChanged,
            this, &AdvancedAnimationDialog::onTabChanged);
    connect(_ui->mixingListWidget, &QListWidget::itemSelectionChanged,
            this, &AdvancedAnimationDialog::updateReport);

    loadAnimations();
    populateMixingList();
    updateReport();
}

AdvancedAnimationDialog::~AdvancedAnimationDialog()
{
    for (auto *anim : _animations) {
        if (anim) {
            anim->Release_Ref();
        }
    }
    delete _ui;
}

void AdvancedAnimationDialog::accept()
{
    if (!_viewport || _renderObjectName.isEmpty()) {
        QDialog::accept();
        return;
    }

    QVector<int> selected_indices;
    const QList<QListWidgetItem *> selected_items = _ui->mixingListWidget->selectedItems();
    if (selected_items.isEmpty()) {
        QDialog::accept();
        return;
    }
    selected_indices.reserve(selected_items.size());
    for (auto *item : selected_items) {
        const int row = _ui->mixingListWidget->row(item);
        if (row >= 0 && row < _animations.size()) {
            selected_indices.append(row);
        }
    }
    if (selected_indices.isEmpty()) {
        QDialog::accept();
        return;
    }
    std::sort(selected_indices.begin(), selected_indices.end());

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        QMessageBox::warning(this, "Advanced Animation", "WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = _renderObjectName.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        QMessageBox::warning(this, "Advanced Animation", "Failed to load render object.");
        return;
    }

    const int count = selected_indices.size();
    auto *combo = new HAnimComboClass(count);
    int idx = 0;
    for (int anim_index : selected_indices) {
        HAnimClass *anim = _animations[anim_index];
        combo->Set_Motion(idx, anim);
        combo->Set_Weight(idx, 1.0f);
        if (auto *combo_data = combo->Peek_Anim_Combo_Data(idx)) {
            combo_data->Build_Active_Pivot_Map();
        }
        ++idx;
    }

    _viewport->clearAnimation();
    _viewport->setRenderObject(render_obj);
    _viewport->setAnimationCombo(combo);
    render_obj->Release_Ref();

    QDialog::accept();
}

void AdvancedAnimationDialog::updateReport()
{
    _ui->reportTableWidget->clear();
    _ui->reportTableWidget->setRowCount(0);
    _ui->reportTableWidget->setColumnCount(0);

    if (!_hasHierarchy || _animations.isEmpty()) {
        _ui->reportTableWidget->setRowCount(0);
        _ui->reportTableWidget->setColumnCount(0);
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    const QByteArray name_bytes = _renderObjectName.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        return;
    }

    const HTreeClass *htree = render_obj->Get_HTree();
    if (!htree) {
        render_obj->Release_Ref();
        return;
    }

    QVector<int> anim_indices;
    const QList<QListWidgetItem *> selected_items = _ui->mixingListWidget->selectedItems();
    if (!selected_items.isEmpty()) {
        for (auto *item : selected_items) {
            const int row = _ui->mixingListWidget->row(item);
            if (row >= 0 && row < _animations.size()) {
                anim_indices.append(row);
            }
        }
        std::sort(anim_indices.begin(), anim_indices.end());
    } else {
        anim_indices.reserve(_animations.size());
        for (int i = 0; i < _animations.size(); ++i) {
            anim_indices.append(i);
        }
    }

    const int column_count = anim_indices.size() + 1;
    _ui->reportTableWidget->setColumnCount(column_count);
    QStringList headers;
    headers << "Bone Name";
    for (int index : anim_indices) {
        headers << AnimName(_animations[index]);
    }
    _ui->reportTableWidget->setHorizontalHeaderLabels(headers);

    const int bone_count = render_obj->Get_Num_Bones();
    int row = 0;
    for (int bone_index = 1; bone_index < bone_count; ++bone_index) {
        const char *bone_name = htree->Get_Bone_Name(bone_index);
        if (!bone_name) {
            continue;
        }

        _ui->reportTableWidget->insertRow(row);
        auto *bone_item = new QTableWidgetItem(QString::fromLatin1(bone_name));
        _ui->reportTableWidget->setItem(row, 0, bone_item);

        for (int column = 0; column < anim_indices.size(); ++column) {
            const int anim_index = anim_indices[column];
            HAnimClass *anim = _animations[anim_index];
            if (anim && anim->Is_Node_Motion_Present(bone_index)) {
                const QString channels = makeChannelString(bone_index, anim);
                auto *cell = new QTableWidgetItem(channels);
                _ui->reportTableWidget->setItem(row, column + 1, cell);
            }
        }
        ++row;
    }

    _ui->reportTableWidget->resizeColumnsToContents();
    render_obj->Release_Ref();
}

void AdvancedAnimationDialog::onTabChanged(int index)
{
    Q_UNUSED(index);
    updateReport();
}

void AdvancedAnimationDialog::loadAnimations()
{
    _animations.clear();
    _hasHierarchy = false;
    _hierarchyName.clear();

    if (_renderObjectName.isEmpty()) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    const QByteArray name_bytes = _renderObjectName.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        return;
    }

    const HTreeClass *htree = render_obj->Get_HTree();
    if (!htree || !htree->Get_Name()) {
        render_obj->Release_Ref();
        return;
    }

    _hasHierarchy = true;
    _hierarchyName = QString::fromLatin1(htree->Get_Name());
    render_obj->Release_Ref();

    AssetIterator *iterator = asset_manager->Create_HAnim_Iterator();
    if (!iterator) {
        return;
    }

    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        const char *anim_name = iterator->Current_Item_Name();
        if (!anim_name || !anim_name[0]) {
            continue;
        }

        HAnimClass *anim = asset_manager->Get_HAnim(anim_name);
        if (!anim) {
            continue;
        }

        const char *hname = anim->Get_HName();
        if (hname && _hierarchyName.compare(QString::fromLatin1(hname), Qt::CaseInsensitive) == 0) {
            _animations.append(anim);
            if (_animations.size() >= kMaxReportAnims) {
                QMessageBox::warning(this,
                                     "Advanced Animation",
                                     QString("Only %1 animations are supported in this report."
                                             " More are loaded and will be ignored.")
                                         .arg(kMaxReportAnims));
                break;
            }
            continue;
        }

        anim->Release_Ref();
    }

    delete iterator;

    std::sort(_animations.begin(), _animations.end(), [](HAnimClass *a, HAnimClass *b) {
        return QString::compare(AnimName(a), AnimName(b), Qt::CaseInsensitive) < 0;
    });
}

void AdvancedAnimationDialog::populateMixingList()
{
    _ui->mixingListWidget->clear();

    if (!_hasHierarchy) {
        _ui->mixingListWidget->addItem("No hierarchy available for this object.");
        _ui->mixingListWidget->setEnabled(false);
        return;
    }

    if (_animations.isEmpty()) {
        _ui->mixingListWidget->addItem("No animations available.");
        _ui->mixingListWidget->setEnabled(false);
        return;
    }

    _ui->mixingListWidget->setEnabled(true);
    for (auto *anim : _animations) {
        _ui->mixingListWidget->addItem(AnimName(anim));
    }
}

QString AdvancedAnimationDialog::makeChannelString(int boneIndex, HAnimClass *anim) const
{
    QString channels;
    if (!anim) {
        return channels;
    }

    if (anim->Has_X_Translation(boneIndex)) {
        channels += 'X';
    }
    if (anim->Has_Y_Translation(boneIndex)) {
        channels += 'Y';
    }
    if (anim->Has_Z_Translation(boneIndex)) {
        channels += 'Z';
    }
    if (anim->Has_Rotation(boneIndex)) {
        channels += 'Q';
    }
    if (anim->Has_Visibility(boneIndex)) {
        channels += 'V';
    }

    return channels;
}
