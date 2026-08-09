#include "AnimationSettingsDialog.h"

#include "W3DViewport.h"
#include "ui_AnimationSettingsDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSlider>

#include <algorithm>

AnimationSettingsDialog::AnimationSettingsDialog(W3DViewport &viewport, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AnimationSettingsDialog)
    , _viewport(&viewport)
{
    _ui->setupUi(this);

    const int initial_percent =
        std::clamp(static_cast<int>(viewport.animationSpeed() * 100.0f + 0.5f), 1, 200);
    _ui->speedSlider->setValue(initial_percent);
    _ui->blendCheckBox->setChecked(_viewport->animationBlend());
    updateSpeed(initial_percent);

    connect(_ui->speedSlider, &QSlider::valueChanged, this, [this](int percent) {
        updateSpeed(percent);
        _viewport->setAnimationSpeed(static_cast<float>(percent) / 100.0f);
    });
    connect(_ui->blendCheckBox, &QCheckBox::toggled,
            _viewport, &W3DViewport::setAnimationBlend);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AnimationSettingsDialog::~AnimationSettingsDialog()
{
    delete _ui;
}

void AnimationSettingsDialog::updateSpeed(int percent)
{
    const float speed = static_cast<float>(percent) / 100.0f;
    _ui->speedValueLabel->setText(QString("Speed: %1x").arg(speed, 0, 'f', 2));
}
