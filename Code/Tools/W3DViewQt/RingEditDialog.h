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
#include <functional>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QTableWidget;
class RingRenderObjClass;

namespace Ui {
class RingEditDialog;
}

class RingEditDialog final : public QDialog
{
    Q_OBJECT

public:
    using ApplyHandler = std::function<bool(RingRenderObjClass &, const QString &)>;

    explicit RingEditDialog(RingRenderObjClass *ring, QWidget *parent = nullptr);
    ~RingEditDialog() override;

    RingRenderObjClass *ring() const;
    QString oldName() const;
    QString registeredName() const;
    void setApplyHandler(ApplyHandler handler,
                         const QString &registeredName,
                         bool initialApplyRequired = false);

protected:
    void accept() override;
    void reject() override;

private slots:
    void apply();
    void browseTexture();
    void editorChanged();

private:
    void connectEditorSignals();
    void loadFromRing();
    bool updateRingFromUi(bool showWarnings);
    bool commitPendingChanges();
    void updateApplyButton();
    int findShaderIndex() const;

    Ui::RingEditDialog *_ui = nullptr;
    RingRenderObjClass *_ring = nullptr;
    RingRenderObjClass *_lastAppliedRing = nullptr;
    QString _oldName;
    QString _registeredName;
    ApplyHandler _applyHandler;
    bool _dirty = false;
    bool _initialApplyRequired = false;

    QLineEdit *_nameEdit = nullptr;
    QLineEdit *_textureEdit = nullptr;
    QDoubleSpinBox *_lifetimeSpin = nullptr;
    QComboBox *_shaderCombo = nullptr;
    QCheckBox *_cameraAlignCheck = nullptr;
    QCheckBox *_loopCheck = nullptr;
    QSpinBox *_tilingSpin = nullptr;

    QTableWidget *_colorKeysTable = nullptr;
    QTableWidget *_alphaKeysTable = nullptr;

    QDoubleSpinBox *_innerXSpin = nullptr;
    QDoubleSpinBox *_innerYSpin = nullptr;
    QDoubleSpinBox *_outerXSpin = nullptr;
    QDoubleSpinBox *_outerYSpin = nullptr;
    QTableWidget *_innerScaleTable = nullptr;
    QTableWidget *_outerScaleTable = nullptr;
};
