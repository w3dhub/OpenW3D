#include "RingEditDialog.h"

#include "ui_RingEditDialog.h"

#include "KeyframeTableUtils.h"

#include "assetmgr.h"
#include "ringobj.h"
#include "shader.h"
#include "texture.h"
#include "vector2.h"
#include "vector3.h"

#include <QAbstractItemView>
#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <algorithm>
#include <optional>
#include <utility>

namespace {
struct ShaderPreset {
    const char *label;
    ShaderClass shader;
};

ShaderPreset BuildPreset(const char *label, const ShaderClass &shader)
{
    ShaderPreset preset{label, shader};
    return preset;
}

const ShaderPreset *ShaderPresets(int &count)
{
    static ShaderPreset presets[] = {
        BuildPreset("Additive", ShaderClass::_PresetAdditiveShader),
        BuildPreset("Alpha", ShaderClass::_PresetAlphaShader),
        BuildPreset("Opaque", ShaderClass::_PresetOpaqueShader),
        BuildPreset("Multiplicative", ShaderClass::_PresetMultiplicativeShader),
    };

    count = static_cast<int>(sizeof(presets) / sizeof(presets[0]));
    return presets;
}

bool ShaderMatches(const ShaderClass &a, const ShaderClass &b)
{
    return a.Get_Bits() == b.Get_Bits();
}

void ConfigureKeyframeTable(QTableWidget *table)
{
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(false);
    table->setShowGrid(true);
}

QVector<QVector<double>> SortedRows(const QTableWidget *table)
{
    QVector<QVector<double>> rows = GetKeyframeRows(table);
    std::sort(rows.begin(), rows.end(), [](const QVector<double> &a, const QVector<double> &b) {
        const double time_a = a.isEmpty() ? 0.0 : a[0];
        const double time_b = b.isEmpty() ? 0.0 : b[0];
        return time_a < time_b;
    });
    return rows;
}

std::optional<double> PromptKeyTime(QWidget *parent, const QString &title)
{
    bool ok = false;
    const double time = QInputDialog::getDouble(parent, title, "Time (0-1):", 0.0, 0.0, 1.0, 3, &ok);
    if (!ok) {
        return std::nullopt;
    }
    return time;
}

RingColorChannelClass BuildColorChannel(QTableWidget *table, const Vector3 &fallback)
{
    RingColorChannelClass channel;
    channel.Reset();

    const QVector<QVector<double>> rows = SortedRows(table);
    if (rows.isEmpty()) {
        channel.Add_Key(fallback, 0.0f);
        return channel;
    }

    for (const QVector<double> &row : rows) {
        if (row.size() < 4) {
            continue;
        }
        channel.Add_Key(Vector3(row[1], row[2], row[3]), static_cast<float>(row[0]));
    }

    return channel;
}

RingAlphaChannelClass BuildAlphaChannel(QTableWidget *table, float fallback)
{
    RingAlphaChannelClass channel;
    channel.Reset();

    const QVector<QVector<double>> rows = SortedRows(table);
    if (rows.isEmpty()) {
        channel.Add_Key(fallback, 0.0f);
        return channel;
    }

    for (const QVector<double> &row : rows) {
        if (row.size() < 2) {
            continue;
        }
        channel.Add_Key(static_cast<float>(row[1]), static_cast<float>(row[0]));
    }

    return channel;
}

RingScaleChannelClass BuildScaleChannel(QTableWidget *table, const Vector2 &fallback)
{
    RingScaleChannelClass channel;
    channel.Reset();

    const QVector<QVector<double>> rows = SortedRows(table);
    if (rows.isEmpty()) {
        channel.Add_Key(fallback, 0.0f);
        return channel;
    }

    for (const QVector<double> &row : rows) {
        if (row.size() < 3) {
            continue;
        }
        channel.Add_Key(Vector2(row[1], row[2]), static_cast<float>(row[0]));
    }

    return channel;
}
}

RingEditDialog::RingEditDialog(RingRenderObjClass *ring, QWidget *parent)
    : QDialog(parent),
      _ui(new Ui::RingEditDialog)
{
    const bool is_new_ring = ring == nullptr;
    _ui->setupUi(this);

    _nameEdit = _ui->nameEdit;
    _textureEdit = _ui->textureEdit;
    _lifetimeSpin = _ui->lifetimeSpin;
    _shaderCombo = _ui->shaderCombo;
    _cameraAlignCheck = _ui->cameraAlignCheck;
    _loopCheck = _ui->loopCheck;
    _tilingSpin = _ui->tilingSpin;
    _colorKeysTable = _ui->colorKeysTable;
    _alphaKeysTable = _ui->alphaKeysTable;
    _innerXSpin = _ui->innerXSpin;
    _innerYSpin = _ui->innerYSpin;
    _outerXSpin = _ui->outerXSpin;
    _outerYSpin = _ui->outerYSpin;
    _innerScaleTable = _ui->innerScaleTable;
    _outerScaleTable = _ui->outerScaleTable;

    ConfigureKeyframeTable(_colorKeysTable);
    ConfigureKeyframeTable(_alphaKeysTable);
    ConfigureKeyframeTable(_innerScaleTable);
    ConfigureKeyframeTable(_outerScaleTable);

    if (ring) {
        _ring = ring;
        _ring->Add_Ref();
    } else {
        _ring = new RingRenderObjClass;
        _ring->Set_Name("Ring");
    }

    if (_ring && _ring->Get_Name()) {
        _oldName = QString::fromLatin1(_ring->Get_Name());
    }
    _registeredName = is_new_ring ? QString() : _oldName;
    _initialApplyRequired = is_new_ring;
    if (_ring) {
        _lastAppliedRing = new RingRenderObjClass(*_ring);
    }

    int preset_count = 0;
    const ShaderPreset *presets = ShaderPresets(preset_count);
    for (int i = 0; i < preset_count; ++i) {
        _shaderCombo->addItem(presets[i].label, i);
    }

    const QVector<KeyframeColumnSpec> color_specs = {
        {0.0, 1.0, 3},
        {0.0, 1.0, 3},
        {0.0, 1.0, 3},
        {0.0, 1.0, 3},
    };
    const QVector<KeyframeColumnSpec> alpha_specs = {
        {0.0, 1.0, 3},
        {0.0, 1.0, 3},
    };
    const QVector<KeyframeColumnSpec> scale_specs = {
        {0.0, 1.0, 3},
        {0.0, 10000.0, 3},
        {0.0, 10000.0, 3},
    };
    connect(_ui->browseButton, &QPushButton::clicked, this, &RingEditDialog::browseTexture);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &RingEditDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &RingEditDialog::reject);
    if (QPushButton *apply_button = _ui->buttonBox->button(QDialogButtonBox::Apply)) {
        connect(apply_button, &QPushButton::clicked, this, &RingEditDialog::apply);
    }

    auto *color_add = _ui->colorAddButton;
    auto *color_remove = _ui->colorRemoveButton;
    auto *color_sort = _ui->colorSortButton;
    auto *alpha_add = _ui->alphaAddButton;
    auto *alpha_remove = _ui->alphaRemoveButton;
    auto *alpha_sort = _ui->alphaSortButton;
    auto *inner_add = _ui->innerAddButton;
    auto *inner_remove = _ui->innerRemoveButton;
    auto *inner_sort = _ui->innerSortButton;
    auto *outer_add = _ui->outerAddButton;
    auto *outer_remove = _ui->outerRemoveButton;
    auto *outer_sort = _ui->outerSortButton;

    connect(color_add, &QPushButton::clicked, this, [this, color_specs]() {
        const auto time = PromptKeyTime(this, "Add Color Key");
        if (!time) {
            return;
        }
        RingColorChannelClass channel = BuildColorChannel(_colorKeysTable, _ring->Get_Color());
        const Vector3 value = channel.Evaluate(static_cast<float>(*time));
        AddKeyframeRow(_colorKeysTable, {*time, value.X, value.Y, value.Z}, color_specs);
        SortKeyframeRows(_colorKeysTable, color_specs);
        connectEditorSignals();
        editorChanged();
    });
    connect(color_remove, &QPushButton::clicked, this, [this]() {
        RemoveSelectedKeyframeRows(_colorKeysTable);
        connectEditorSignals();
        editorChanged();
    });
    connect(color_sort, &QPushButton::clicked, this, [this, color_specs]() {
        SortKeyframeRows(_colorKeysTable, color_specs);
        connectEditorSignals();
        editorChanged();
    });

    connect(alpha_add, &QPushButton::clicked, this, [this, alpha_specs]() {
        const auto time = PromptKeyTime(this, "Add Opacity Key");
        if (!time) {
            return;
        }
        RingAlphaChannelClass channel = BuildAlphaChannel(_alphaKeysTable, _ring->Get_Alpha());
        const float value = channel.Evaluate(static_cast<float>(*time));
        AddKeyframeRow(_alphaKeysTable, {*time, value}, alpha_specs);
        SortKeyframeRows(_alphaKeysTable, alpha_specs);
        connectEditorSignals();
        editorChanged();
    });
    connect(alpha_remove, &QPushButton::clicked, this, [this]() {
        RemoveSelectedKeyframeRows(_alphaKeysTable);
        connectEditorSignals();
        editorChanged();
    });
    connect(alpha_sort, &QPushButton::clicked, this, [this, alpha_specs]() {
        SortKeyframeRows(_alphaKeysTable, alpha_specs);
        connectEditorSignals();
        editorChanged();
    });

    connect(inner_add, &QPushButton::clicked, this, [this, scale_specs]() {
        const auto time = PromptKeyTime(this, "Add Inner Scale Key");
        if (!time) {
            return;
        }
        RingScaleChannelClass channel = BuildScaleChannel(_innerScaleTable, _ring->Get_Inner_Scale());
        const Vector2 value = channel.Evaluate(static_cast<float>(*time));
        AddKeyframeRow(_innerScaleTable, {*time, value.X, value.Y}, scale_specs);
        SortKeyframeRows(_innerScaleTable, scale_specs);
        connectEditorSignals();
        editorChanged();
    });
    connect(inner_remove, &QPushButton::clicked, this, [this]() {
        RemoveSelectedKeyframeRows(_innerScaleTable);
        connectEditorSignals();
        editorChanged();
    });
    connect(inner_sort, &QPushButton::clicked, this, [this, scale_specs]() {
        SortKeyframeRows(_innerScaleTable, scale_specs);
        connectEditorSignals();
        editorChanged();
    });

    connect(outer_add, &QPushButton::clicked, this, [this, scale_specs]() {
        const auto time = PromptKeyTime(this, "Add Outer Scale Key");
        if (!time) {
            return;
        }
        RingScaleChannelClass channel = BuildScaleChannel(_outerScaleTable, _ring->Get_Outer_Scale());
        const Vector2 value = channel.Evaluate(static_cast<float>(*time));
        AddKeyframeRow(_outerScaleTable, {*time, value.X, value.Y}, scale_specs);
        SortKeyframeRows(_outerScaleTable, scale_specs);
        connectEditorSignals();
        editorChanged();
    });
    connect(outer_remove, &QPushButton::clicked, this, [this]() {
        RemoveSelectedKeyframeRows(_outerScaleTable);
        connectEditorSignals();
        editorChanged();
    });
    connect(outer_sort, &QPushButton::clicked, this, [this, scale_specs]() {
        SortKeyframeRows(_outerScaleTable, scale_specs);
        connectEditorSignals();
        editorChanged();
    });

    loadFromRing();
    connectEditorSignals();
    updateApplyButton();
}
RingEditDialog::~RingEditDialog()
{
    if (_lastAppliedRing) {
        _lastAppliedRing->Release_Ref();
        _lastAppliedRing = nullptr;
    }
    if (_ring) {
        _ring->Release_Ref();
        _ring = nullptr;
    }
    delete _ui;
}

RingRenderObjClass *RingEditDialog::ring() const
{
    if (_ring) {
        _ring->Add_Ref();
    }
    return _ring;
}

QString RingEditDialog::oldName() const
{
    return _oldName;
}

QString RingEditDialog::registeredName() const
{
    return _registeredName;
}

void RingEditDialog::setApplyHandler(ApplyHandler handler,
                                     const QString &registeredName,
                                     bool initialApplyRequired)
{
    _applyHandler = std::move(handler);
    _registeredName = registeredName;
    _initialApplyRequired = initialApplyRequired;
    updateApplyButton();
}

void RingEditDialog::connectEditorSignals()
{
    if (_nameEdit) {
        connect(_nameEdit,
                &QLineEdit::textChanged,
                this,
                &RingEditDialog::editorChanged,
                Qt::UniqueConnection);
    }
    if (_textureEdit) {
        connect(_textureEdit,
                &QLineEdit::textChanged,
                this,
                &RingEditDialog::editorChanged,
                Qt::UniqueConnection);
    }
    if (_shaderCombo) {
        connect(_shaderCombo,
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &RingEditDialog::editorChanged,
                Qt::UniqueConnection);
    }
    for (QCheckBox *check_box : {_cameraAlignCheck, _loopCheck}) {
        if (check_box) {
            connect(check_box,
                    &QCheckBox::toggled,
                    this,
                    &RingEditDialog::editorChanged,
                    Qt::UniqueConnection);
        }
    }
    const auto double_spin_boxes = findChildren<QDoubleSpinBox *>();
    for (QDoubleSpinBox *spin_box : double_spin_boxes) {
        connect(spin_box,
                qOverload<double>(&QDoubleSpinBox::valueChanged),
                this,
                &RingEditDialog::editorChanged,
                Qt::UniqueConnection);
    }
    const auto spin_boxes = findChildren<QSpinBox *>();
    for (QSpinBox *spin_box : spin_boxes) {
        connect(spin_box,
                qOverload<int>(&QSpinBox::valueChanged),
                this,
                &RingEditDialog::editorChanged,
                Qt::UniqueConnection);
    }
}

bool RingEditDialog::updateRingFromUi(bool showWarnings)
{
    if (!_ring) {
        return false;
    }

    const QString name = _nameEdit ? _nameEdit->text().trimmed() : QString();
    if (name.isEmpty()) {
        if (showWarnings) {
            QMessageBox::warning(this, "Ring", "Invalid ring name. Please enter a new name.");
        }
        return !showWarnings;
    }

    TextureClass *texture = nullptr;
    bool can_update_texture = false;
    const QString texture_path = _textureEdit ? _textureEdit->text().trimmed() : QString();
    if (texture_path.isEmpty()) {
        can_update_texture = true;
    } else {
        const QString file_name_only = QFileInfo(texture_path).fileName();
        if (file_name_only.isEmpty()) {
            if (showWarnings) {
                QMessageBox::warning(this, "Ring", "Invalid texture filename.");
            }
            return !showWarnings;
        }

        auto *asset_manager = WW3DAssetManager::Get_Instance();
        if (!asset_manager) {
            if (showWarnings) {
                QMessageBox::warning(this, "Ring", "WW3D asset manager is not available.");
                return false;
            }
        } else {
            const QByteArray texture_bytes = file_name_only.toLatin1();
            texture = asset_manager->Get_Texture(texture_bytes.constData());
            can_update_texture = true;
        }
    }

    if (can_update_texture) {
        _ring->Set_Texture(texture);
        if (texture) {
            texture->Release_Ref();
        }
    }

    int preset_count = 0;
    const ShaderPreset *presets = ShaderPresets(preset_count);
    const int shader_index = _shaderCombo ? _shaderCombo->currentData().toInt() : -1;
    if (shader_index >= 0 && shader_index < preset_count) {
        ShaderClass shader = presets[shader_index].shader;
        _ring->Set_Shader(shader);
    } else if (_lastAppliedRing) {
        ShaderClass shader = _lastAppliedRing->Get_Shader();
        _ring->Set_Shader(shader);
    }

    const float lifetime = _lifetimeSpin ? static_cast<float>(_lifetimeSpin->value()) : 0.0f;
    _ring->Set_Animation_Duration(lifetime);

    if (_tilingSpin) {
        _ring->Set_Texture_Tiling(_tilingSpin->value());
    }

    if (_cameraAlignCheck) {
        _ring->Set_Flag(RingRenderObjClass::USE_CAMERA_ALIGN, _cameraAlignCheck->isChecked());
    }
    if (_loopCheck) {
        _ring->Set_Flag(RingRenderObjClass::USE_ANIMATION_LOOP, _loopCheck->isChecked());
    }

    const RingColorChannelClass color_channel = BuildColorChannel(_colorKeysTable, _ring->Get_Color());
    const RingAlphaChannelClass alpha_channel = BuildAlphaChannel(_alphaKeysTable, _ring->Get_Alpha());
    _ring->Set_Color_Channel(color_channel);
    _ring->Set_Alpha_Channel(alpha_channel);

    const float inner_x = _innerXSpin ? static_cast<float>(_innerXSpin->value()) : 0.0f;
    const float inner_y = _innerYSpin ? static_cast<float>(_innerYSpin->value()) : 0.0f;
    const float outer_x = _outerXSpin ? static_cast<float>(_outerXSpin->value()) : 0.0f;
    const float outer_y = _outerYSpin ? static_cast<float>(_outerYSpin->value()) : 0.0f;
    _ring->Set_Inner_Extent(Vector2(inner_x, inner_y));
    _ring->Set_Outer_Extent(Vector2(outer_x, outer_y));

    const RingScaleChannelClass inner_scale = BuildScaleChannel(_innerScaleTable, _ring->Get_Inner_Scale());
    const RingScaleChannelClass outer_scale = BuildScaleChannel(_outerScaleTable, _ring->Get_Outer_Scale());
    _ring->Set_Inner_Scale_Channel(inner_scale);
    _ring->Set_Outer_Scale_Channel(outer_scale);

    const QByteArray name_bytes = name.toLatin1();
    _ring->Set_Name(name_bytes.constData());
    _ring->Restart_Animation();

    return true;
}

bool RingEditDialog::commitPendingChanges()
{
    const bool has_pending_changes = _initialApplyRequired || _dirty;
    if (!updateRingFromUi(true)) {
        return false;
    }

    if (!has_pending_changes) {
        return true;
    }

    if (_applyHandler && !_applyHandler(*_ring, _registeredName)) {
        return false;
    }

    if (const char *name = _ring->Get_Name()) {
        _registeredName = QString::fromLatin1(name);
    }
    if (_lastAppliedRing) {
        *_lastAppliedRing = *_ring;
    }
    _dirty = false;
    _initialApplyRequired = false;
    updateApplyButton();
    return true;
}

void RingEditDialog::apply()
{
    commitPendingChanges();
}

void RingEditDialog::accept()
{
    if (!commitPendingChanges()) {
        return;
    }

    QDialog::accept();
}

void RingEditDialog::reject()
{
    if (_ring && _lastAppliedRing) {
        *_ring = *_lastAppliedRing;
        _ring->Restart_Animation();
    }
    QDialog::reject();
}

void RingEditDialog::editorChanged()
{
    _dirty = true;
    updateRingFromUi(false);
    updateApplyButton();
}

void RingEditDialog::updateApplyButton()
{
    if (!_ui || !_ui->buttonBox) {
        return;
    }
    if (QPushButton *apply_button = _ui->buttonBox->button(QDialogButtonBox::Apply)) {
        apply_button->setEnabled(_initialApplyRequired || _dirty);
    }
}

void RingEditDialog::browseTexture()
{
    const QString start = _textureEdit ? _textureEdit->text() : QString();
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Select Texture",
        start,
        "Texture Files (*.tga);;All Files (*.*)");
    if (!path.isEmpty() && _textureEdit) {
        _textureEdit->setText(path);
    }
}

void RingEditDialog::loadFromRing()
{
    if (!_ring) {
        return;
    }

    if (_nameEdit) {
        const char *name = _ring->Get_Name();
        _nameEdit->setText(name ? QString::fromLatin1(name) : QString());
    }

    if (_textureEdit) {
        TextureClass *texture = _ring->Peek_Texture();
        if (texture) {
            const StringClass &name = texture->Get_Texture_Name();
            const QByteArray name_bytes(name.Peek_Buffer(), static_cast<int>(name.Get_Length()));
            _textureEdit->setText(QString::fromLatin1(name_bytes));
        }
    }

    if (_lifetimeSpin) {
        _lifetimeSpin->setValue(_ring->Get_Animation_Duration());
    }

    if (_shaderCombo) {
        int shader_index = findShaderIndex();
        if (shader_index < 0) {
            _shaderCombo->addItem("Custom (preserved)", -1);
            shader_index = _shaderCombo->count() - 1;
        }
        _shaderCombo->setCurrentIndex(shader_index);
    }

    if (_tilingSpin) {
        _tilingSpin->setValue(_ring->Get_Texture_Tiling());
    }

    const unsigned int flags = _ring->Get_Flags();
    if (_cameraAlignCheck) {
        _cameraAlignCheck->setChecked((flags & RingRenderObjClass::USE_CAMERA_ALIGN) != 0);
    }
    if (_loopCheck) {
        _loopCheck->setChecked((flags & RingRenderObjClass::USE_ANIMATION_LOOP) != 0);
    }

    if (_innerXSpin && _innerYSpin) {
        const Vector2 inner = _ring->Get_Inner_Extent();
        _innerXSpin->setValue(inner.X);
        _innerYSpin->setValue(inner.Y);
    }

    if (_outerXSpin && _outerYSpin) {
        const Vector2 outer = _ring->Get_Outer_Extent();
        _outerXSpin->setValue(outer.X);
        _outerYSpin->setValue(outer.Y);
    }

    RingColorChannelClass color_channel = _ring->Get_Color_Channel();
    if (color_channel.Get_Key_Count() == 0) {
        color_channel.Add_Key(_ring->Get_Color(), 0.0f);
    }
    QVector<QVector<double>> color_rows;
    for (int i = 0; i < color_channel.Get_Key_Count(); ++i) {
        const auto &key = color_channel.Get_Key(i);
        const Vector3 value = key.Get_Value();
        color_rows.push_back({key.Get_Time(), value.X, value.Y, value.Z});
    }
    SetKeyframeRows(_colorKeysTable,
                    color_rows,
                    QVector<KeyframeColumnSpec>{{0.0, 1.0, 3},
                                                {0.0, 1.0, 3},
                                                {0.0, 1.0, 3},
                                                {0.0, 1.0, 3}});

    RingAlphaChannelClass alpha_channel = _ring->Get_Alpha_Channel();
    if (alpha_channel.Get_Key_Count() == 0) {
        alpha_channel.Add_Key(_ring->Get_Alpha(), 0.0f);
    }
    QVector<QVector<double>> alpha_rows;
    for (int i = 0; i < alpha_channel.Get_Key_Count(); ++i) {
        const auto &key = alpha_channel.Get_Key(i);
        alpha_rows.push_back({key.Get_Time(), key.Get_Value()});
    }
    SetKeyframeRows(_alphaKeysTable,
                    alpha_rows,
                    QVector<KeyframeColumnSpec>{{0.0, 1.0, 3}, {0.0, 1.0, 3}});

    RingScaleChannelClass inner_scale = _ring->Get_Inner_Scale_Channel();
    if (inner_scale.Get_Key_Count() == 0) {
        inner_scale.Add_Key(_ring->Get_Inner_Scale(), 0.0f);
    }
    QVector<QVector<double>> inner_rows;
    for (int i = 0; i < inner_scale.Get_Key_Count(); ++i) {
        const auto &key = inner_scale.Get_Key(i);
        const Vector2 value = key.Get_Value();
        inner_rows.push_back({key.Get_Time(), value.X, value.Y});
    }
    SetKeyframeRows(_innerScaleTable,
                    inner_rows,
                    QVector<KeyframeColumnSpec>{{0.0, 1.0, 3},
                                                {0.0, 10000.0, 3},
                                                {0.0, 10000.0, 3}});

    RingScaleChannelClass outer_scale = _ring->Get_Outer_Scale_Channel();
    if (outer_scale.Get_Key_Count() == 0) {
        outer_scale.Add_Key(_ring->Get_Outer_Scale(), 0.0f);
    }
    QVector<QVector<double>> outer_rows;
    for (int i = 0; i < outer_scale.Get_Key_Count(); ++i) {
        const auto &key = outer_scale.Get_Key(i);
        const Vector2 value = key.Get_Value();
        outer_rows.push_back({key.Get_Time(), value.X, value.Y});
    }
    SetKeyframeRows(_outerScaleTable,
                    outer_rows,
                    QVector<KeyframeColumnSpec>{{0.0, 1.0, 3},
                                                {0.0, 10000.0, 3},
                                                {0.0, 10000.0, 3}});
}

int RingEditDialog::findShaderIndex() const
{
    if (!_ring) {
        return -1;
    }

    int preset_count = 0;
    const ShaderPreset *presets = ShaderPresets(preset_count);
    const ShaderClass &shader = _ring->Get_Shader();
    for (int index = 0; index < preset_count; ++index) {
        if (ShaderMatches(presets[index].shader, shader)) {
            return index;
        }
    }

    return -1;
}
