#include "AggregateNameDialog.h"

#include "ui_AggregateNameDialog.h"

#include "w3d_file.h"

#include <QDialogButtonBox>

AggregateNameDialog::AggregateNameDialog(const QString &title,
                                         const QString &defaultName,
                                         QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AggregateNameDialog)
{
    _ui->setupUi(this);
    setWindowTitle(title);
    _ui->nameLineEdit->setMaxLength(W3D_NAME_LEN - 1);
    _ui->nameLineEdit->setText(defaultName);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AggregateNameDialog::~AggregateNameDialog()
{
    delete _ui;
}

QString AggregateNameDialog::name() const
{
    return _ui->nameLineEdit->text();
}
