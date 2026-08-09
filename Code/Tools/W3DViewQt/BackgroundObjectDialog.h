#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class BackgroundObjectDialog;
}

class BackgroundObjectDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit BackgroundObjectDialog(const QString &currentName, QWidget *parent = nullptr);
    ~BackgroundObjectDialog() override;

    QString selectedName() const;

private slots:
    void onSelectionChanged();
    void onClear();

private:
    Ui::BackgroundObjectDialog *_ui = nullptr;
};
