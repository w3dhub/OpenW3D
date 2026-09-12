#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class BackgroundBitmapDialog;
}

class BackgroundBitmapDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit BackgroundBitmapDialog(const QString &currentPath, QWidget *parent = nullptr);
    ~BackgroundBitmapDialog() override;

    QString selectedPath() const;

private slots:
    void browse();
    void clearPath();

private:
    Ui::BackgroundBitmapDialog *_ui = nullptr;
};
