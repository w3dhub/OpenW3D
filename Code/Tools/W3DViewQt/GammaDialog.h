#pragma once

#include <QDialog>

namespace Ui {
class GammaDialog;
}

class GammaDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit GammaDialog(QWidget *parent = nullptr);
    ~GammaDialog() override;

private slots:
    void onGammaChanged(int value);

private:
    void applyGamma(int value);

    Ui::GammaDialog *_ui = nullptr;
    int _currentGamma = 10;
};
