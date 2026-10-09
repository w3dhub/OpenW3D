#include "ScaleDialog.h"

#include "ui_ScaleDialog.h"

#include <QDialogButtonBox>
#include <QMessageBox>

ScaleDialog::ScaleDialog(double scale, const QString &prompt, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::ScaleDialog)
{
    _ui->setupUi(this);
    _ui->promptLabel->setText(prompt);
    _ui->scaleSpinBox->setValue(scale);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &ScaleDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ScaleDialog::~ScaleDialog()
{
    delete _ui;
}

double ScaleDialog::scale() const
{
    return _ui->scaleSpinBox->value();
}

void ScaleDialog::accept()
{
    if (_ui->scaleSpinBox->value() <= 0.0) {
        QMessageBox::information(this, "Invalid Scale", "Scale must be a value greater than zero.");
        return;
    }

    QDialog::accept();
}
