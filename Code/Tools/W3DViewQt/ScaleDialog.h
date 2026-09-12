#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class ScaleDialog;
}

class ScaleDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ScaleDialog(double scale, const QString &prompt, QWidget *parent = nullptr);
    ~ScaleDialog() override;

    double scale() const;

protected:
    void accept() override;

private:
    Ui::ScaleDialog *_ui = nullptr;
};
