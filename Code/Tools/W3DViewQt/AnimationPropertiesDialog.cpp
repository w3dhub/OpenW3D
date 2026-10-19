/*
** Copyright 2026 OpenW3D contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed WITHOUT ANY WARRANTY; without even the
** implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "AnimationPropertiesDialog.h"

#include "ui_AnimationPropertiesDialog.h"

#include "assetmgr.h"
#include "hanim.h"

#include <QDialogButtonBox>

AnimationPropertiesDialog::AnimationPropertiesDialog(const QString &animationName, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AnimationPropertiesDialog)
{
    _ui->setupUi(this);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (animationName.isEmpty()) {
        setErrorState("No animation selected.");
        return;
    }

    _ui->descriptionLabel->setText(QString("Animation: %1").arg(animationName));

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        setErrorState("WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = animationName.toLatin1();
    HAnimClass *animation = asset_manager->Get_HAnim(name_bytes.constData());
    if (!animation) {
        setErrorState("Failed to load animation.");
        return;
    }

    _ui->frameCountValue->setText(QString::number(animation->Get_Num_Frames()));
    _ui->frameRateValue->setText(QString("%1 fps").arg(animation->Get_Frame_Rate(), 0, 'f', 2));
    _ui->totalTimeValue->setText(QString("%1 seconds").arg(animation->Get_Total_Time(), 0, 'f', 3));

    const char *hier_name = animation->Get_HName();
    if (hier_name) {
        _ui->hierarchyNameValue->setText(QString::fromLatin1(hier_name));
    } else {
        _ui->hierarchyNameValue->setText("");
    }

    animation->Release_Ref();
}

AnimationPropertiesDialog::~AnimationPropertiesDialog()
{
    delete _ui;
}

void AnimationPropertiesDialog::setErrorState(const QString &message)
{
    _ui->descriptionLabel->setText(message);
    _ui->frameCountValue->setText("n/a");
    _ui->frameRateValue->setText("n/a");
    _ui->totalTimeValue->setText("n/a");
    _ui->hierarchyNameValue->setText("n/a");
}
