#include "GammaDialog.h"

#include "ui_GammaDialog.h"

#include "dx8wrapper.h"

#include <QDialogButtonBox>
#include <QSettings>
#include <QSlider>

GammaDialog::GammaDialog(QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::GammaDialog)
{
    _ui->setupUi(this);

    connect(_ui->gammaSlider, &QSlider::valueChanged, this, &GammaDialog::onGammaChanged);

    QSettings settings;
    int gamma = settings.value("Config/Gamma", 10).toInt();
    if (gamma < 10) {
        gamma = 10;
    }
    if (gamma > 30) {
        gamma = 30;
    }
    _ui->gammaSlider->setValue(gamma);
    _currentGamma = gamma;
    onGammaChanged(gamma);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        QSettings settings;
        settings.setValue("Config/Gamma", _currentGamma);
        accept();
    });
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

GammaDialog::~GammaDialog()
{
    delete _ui;
}

void GammaDialog::onGammaChanged(int value)
{
    _currentGamma = value;
    _ui->gammaValueLabel->setText(QString("Gamma: %1").arg(value / 10.0f, 0, 'f', 2));
    applyGamma(value);
}

void GammaDialog::applyGamma(int value)
{
    DX8Wrapper::Set_Gamma(value / 10.0f, 0.0f, 1.0f);
}
