#include "CameraDistanceDialog.h"

#include "ui_CameraDistanceDialog.h"

#include <QDialogButtonBox>

CameraDistanceDialog::CameraDistanceDialog(float distance, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::CameraDistanceDialog)
{
    _ui->setupUi(this);
    _ui->distanceSpinBox->setValue(distance);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

CameraDistanceDialog::~CameraDistanceDialog()
{
    delete _ui;
}

float CameraDistanceDialog::distance() const
{
    return static_cast<float>(_ui->distanceSpinBox->value());
}
