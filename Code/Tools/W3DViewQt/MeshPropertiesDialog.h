#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class MeshPropertiesDialog;
}

class MeshPropertiesDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit MeshPropertiesDialog(const QString &meshName, QWidget *parent = nullptr);
    ~MeshPropertiesDialog() override;

private:
    void setErrorState(const QString &message);

    Ui::MeshPropertiesDialog *_ui = nullptr;
};
