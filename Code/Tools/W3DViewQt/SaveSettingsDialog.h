#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class SaveSettingsDialog;
}

class SaveSettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SaveSettingsDialog(QWidget *parent = nullptr);
    ~SaveSettingsDialog() override;

    QString selectedPath() const;
    bool saveLighting() const;
    bool saveBackground() const;

private slots:
    void browse();
    void updateOkEnabled();

private:
    Ui::SaveSettingsDialog *_ui = nullptr;
};
