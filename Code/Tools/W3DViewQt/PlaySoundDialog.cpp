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

#include "PlaySoundDialog.h"

#include "ui_PlaySoundDialog.h"

#include "AudibleSound.h"
#include "WWAudio.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>

PlaySoundDialog::PlaySoundDialog(const QString &filename, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::PlaySoundDialog)
    , _filename(filename)
{
    _ui->setupUi(this);
    _ui->soundFileLabel->setText(QString("Sound file: %1").arg(_filename));

    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(_ui->playButton, &QPushButton::clicked, this, &PlaySoundDialog::playSound);
    connect(_ui->stopButton, &QPushButton::clicked, this, &PlaySoundDialog::stopSound);

    createSound();
}

PlaySoundDialog::~PlaySoundDialog()
{
    stopSound();
    if (_sound) {
        _sound->Release_Ref();
        _sound = nullptr;
    }
    delete _ui;
}

bool PlaySoundDialog::isReady() const
{
    return _sound != nullptr;
}

bool PlaySoundDialog::createSound()
{
    const QString filename = _filename.trimmed();
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "Play Sound", "No sound file specified.");
        return false;
    }

    auto *audio = WWAudioClass::Get_Instance();
    if (!audio) {
        QMessageBox::warning(this, "Play Sound", "Audio system is not available.");
        return false;
    }

    // Keep an explicitly selected path intact for preview. QFile::encodeName
    // provides the narrow, native filename representation required by WWAudio.
    const QString native_filename = QDir::toNativeSeparators(filename);
    const QByteArray filename_bytes = QFile::encodeName(native_filename);
    _sound = audio->Create_Sound_Effect(filename_bytes.constData());
    if (!_sound) {
        QMessageBox::warning(this, "Play Sound", QString("Cannot find sound file: %1").arg(filename));
        return false;
    }

    playSound();
    return true;
}

void PlaySoundDialog::playSound()
{
    if (_sound) {
        _sound->Stop();
        _sound->Play();
    }
}

void PlaySoundDialog::stopSound()
{
    if (_sound) {
        _sound->Stop();
    }
}
