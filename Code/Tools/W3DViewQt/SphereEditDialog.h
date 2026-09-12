#pragma once

#include <QDialog>
#include <QString>
#include <functional>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QTableWidget;
class SphereRenderObjClass;

namespace Ui {
class SphereEditDialog;
}

class SphereEditDialog final : public QDialog
{
    Q_OBJECT

public:
    using ApplyHandler = std::function<bool(SphereRenderObjClass &, const QString &)>;

    explicit SphereEditDialog(SphereRenderObjClass *sphere, QWidget *parent = nullptr);
    ~SphereEditDialog() override;

    SphereRenderObjClass *sphere() const;
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
    void loadFromSphere();
    bool updateSphereFromUi(bool showWarnings);
    bool commitPendingChanges();
    void updateApplyButton();
    int findShaderIndex() const;

    Ui::SphereEditDialog *_ui = nullptr;
    SphereRenderObjClass *_sphere = nullptr;
    SphereRenderObjClass *_lastAppliedSphere = nullptr;
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

    QTableWidget *_colorKeysTable = nullptr;
    QTableWidget *_alphaKeysTable = nullptr;
    QTableWidget *_vectorKeysTable = nullptr;
    QCheckBox *_useVectorCheck = nullptr;
    QCheckBox *_invertVectorCheck = nullptr;

    QDoubleSpinBox *_sizeXSpin = nullptr;
    QDoubleSpinBox *_sizeYSpin = nullptr;
    QDoubleSpinBox *_sizeZSpin = nullptr;
    QTableWidget *_scaleKeysTable = nullptr;
};
