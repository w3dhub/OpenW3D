#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class AggregateNameDialog;
}

class AggregateNameDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AggregateNameDialog(const QString &title,
                                 const QString &defaultName = QString(),
                                 QWidget *parent = nullptr);
    ~AggregateNameDialog() override;

    QString name() const;

private:
    Ui::AggregateNameDialog *_ui = nullptr;
};
