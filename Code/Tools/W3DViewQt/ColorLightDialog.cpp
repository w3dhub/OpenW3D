#include "ColorLightDialog.h"

#include "ui_ColorLightDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>

#include <utility>

ColorLightDialog::ColorLightDialog(const QString &title,
                                   const Vector3 &initialColor,
                                   ApplyCallback applyCallback,
                                   QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::ColorLightDialog)
    , _initialColor(initialColor)
    , _applyCallback(std::move(applyCallback))
{
    _ui->setupUi(this);
    setWindowTitle(title);

    _ui->redSlider->setValue(static_cast<int>(_initialColor.X * 100.0f));
    _ui->greenSlider->setValue(static_cast<int>(_initialColor.Y * 100.0f));
    _ui->blueSlider->setValue(static_cast<int>(_initialColor.Z * 100.0f));
    _ui->grayscaleCheckBox->setChecked(
        _initialColor.X == _initialColor.Y && _initialColor.X == _initialColor.Z);
    updateValueLabels();

    connect(_ui->redSlider, &QSlider::valueChanged, this,
            [this](int value) { colorSliderChanged(_ui->redSlider, value); });
    connect(_ui->greenSlider, &QSlider::valueChanged, this,
            [this](int value) { colorSliderChanged(_ui->greenSlider, value); });
    connect(_ui->blueSlider, &QSlider::valueChanged, this,
            [this](int value) { colorSliderChanged(_ui->blueSlider, value); });
    connect(_ui->grayscaleCheckBox, &QCheckBox::toggled, this, [this](bool enabled) {
        if (!enabled) {
            return;
        }

        const int value = _ui->redSlider->value();
        const QSignalBlocker greenBlocker(_ui->greenSlider);
        const QSignalBlocker blueBlocker(_ui->blueSlider);
        _ui->greenSlider->setValue(value);
        _ui->blueSlider->setValue(value);
        updateValueLabels();
        applySelectedColor();
    });

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &ColorLightDialog::reject);
}

ColorLightDialog::~ColorLightDialog()
{
    delete _ui;
}

Vector3 ColorLightDialog::selectedColor() const
{
    return Vector3(static_cast<float>(_ui->redSlider->value()) / 100.0f,
                   static_cast<float>(_ui->greenSlider->value()) / 100.0f,
                   static_cast<float>(_ui->blueSlider->value()) / 100.0f);
}

void ColorLightDialog::reject()
{
    if (_applyCallback) {
        _applyCallback(_initialColor);
    }

    QDialog::reject();
}

void ColorLightDialog::colorSliderChanged(QSlider *source, int value)
{
    if (_ui->grayscaleCheckBox->isChecked()) {
        const QSignalBlocker redBlocker(_ui->redSlider);
        const QSignalBlocker greenBlocker(_ui->greenSlider);
        const QSignalBlocker blueBlocker(_ui->blueSlider);
        if (source != _ui->redSlider) {
            _ui->redSlider->setValue(value);
        }
        if (source != _ui->greenSlider) {
            _ui->greenSlider->setValue(value);
        }
        if (source != _ui->blueSlider) {
            _ui->blueSlider->setValue(value);
        }
    }

    updateValueLabels();
    applySelectedColor();
}

void ColorLightDialog::applySelectedColor()
{
    if (_applyCallback) {
        _applyCallback(selectedColor());
    }
}

void ColorLightDialog::updateValueLabels()
{
    _ui->redValueLabel->setText(QString::number(_ui->redSlider->value()));
    _ui->greenValueLabel->setText(QString::number(_ui->greenSlider->value()));
    _ui->blueValueLabel->setText(QString::number(_ui->blueSlider->value()));
}
