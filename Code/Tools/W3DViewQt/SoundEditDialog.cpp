#include "SoundEditDialog.h"

#include "PlaySoundDialog.h"
#include "ui_SoundEditDialog.h"

#include "AudibleSound.h"
#include "Sound3D.h"
#include "WWAudio.h"
#include "assetmgr.h"
#include "soundrobj.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>

namespace {
constexpr float kDefaultDropOff = 100.0f;
constexpr float kDefaultMaxVol = 10.0f;
constexpr float kDefaultPriority = 0.5f;
constexpr float kDefaultVolume = 1.0f;
constexpr int kMaxSoundObjectNameLength = 15;

const AudibleSoundDefinitionClass *findPrototypeSoundDefinition(
    const SoundRenderObjClass *renderObject)
{
    auto *assetManager = WW3DAssetManager::Get_Instance();
    const char *objectName = renderObject ? renderObject->Get_Name() : nullptr;
    if (!assetManager || !objectName || objectName[0] == '\0') {
        return nullptr;
    }

    auto *prototype = dynamic_cast<SoundRenderObjPrototypeClass *>(
        assetManager->Find_Prototype(objectName));
    SoundRenderObjDefClass *renderDefinition =
        prototype ? prototype->Peek_Definition() : nullptr;
    return renderDefinition ? renderDefinition->Peek_Sound_Definition() : nullptr;
}
}

SoundEditDialog::SoundEditDialog(SoundRenderObjClass *sound, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::SoundEditDialog)
{
    _ui->setupUi(this);

    if (sound) {
        _sound = sound;
        _sound->Add_Ref();
    } else {
        _sound = new SoundRenderObjClass;
    }

    if (_sound && _sound->Get_Name()) {
        _oldName = QString::fromLatin1(_sound->Get_Name());
    }

    connect(_ui->browseButton, &QPushButton::clicked, this, &SoundEditDialog::browseSoundFile);
    connect(_ui->playButton, &QPushButton::clicked, this, &SoundEditDialog::playSound);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &SoundEditDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &SoundEditDialog::reject);
    connect(_ui->radio2d, &QRadioButton::toggled, this, &SoundEditDialog::toggleSoundType);
    connect(_ui->radio3d, &QRadioButton::toggled, this, &SoundEditDialog::toggleSoundType);

    loadFromSound();
    updateEnableState();
}

SoundEditDialog::~SoundEditDialog()
{
    if (_sound) {
        _sound->Release_Ref();
        _sound = nullptr;
    }
    delete _ui;
}

SoundRenderObjClass *SoundEditDialog::sound() const
{
    if (_sound) {
        _sound->Add_Ref();
    }
    return _sound;
}

QString SoundEditDialog::oldName() const
{
    return _oldName;
}

void SoundEditDialog::accept()
{
    if (!_sound) {
        QDialog::reject();
        return;
    }

    const QString name = _ui->nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Sound Object", "Invalid object name. Please enter a new name.");
        return;
    }

    const QByteArray name_bytes = name.toLatin1();
    if (name_bytes.size() > kMaxSoundObjectNameLength) {
        QMessageBox::warning(
            this,
            "Sound Object",
            QString("Sound object names are limited to %1 characters.")
                .arg(kMaxSoundObjectNameLength));
        return;
    }

    auto *audio = WWAudioClass::Get_Instance();
    if (!audio) {
        QMessageBox::warning(this, "Sound Object", "Audio system is not available.");
        return;
    }

    const QString filename = _ui->fileEdit->text().trimmed();
    const QString file_name_only = QFileInfo(filename).fileName();
    if (file_name_only.isEmpty()) {
        QMessageBox::warning(this, "Sound Object", "Invalid sound filename.");
        return;
    }

    AudibleSoundClass *sound = nullptr;
    const bool is_3d = _ui->radio3d->isChecked();
    const QByteArray file_bytes = file_name_only.toLatin1();
    if (is_3d) {
        sound = audio->Create_3D_Sound(file_bytes.constData());
    } else {
        sound = audio->Create_Sound_Effect(file_bytes.constData());
    }

    // Create_3D_Sound can return a pseudo-3D object with no buffer when the
    // file could not be resolved. Treat that as creation failure instead of
    // serializing an object whose definition has an empty filename.
    if (sound && (!sound->Get_Filename() || sound->Get_Filename()[0] == '\0')) {
        sound->Release_Ref();
        sound = nullptr;
    }

    if (!sound) {
        QMessageBox::warning(
            this,
            "Sound Object",
            QString("Failed to create sound object from: %1").arg(file_name_only));
        return;
    }

    const float priority = _ui->prioritySlider->value() / 100.0f;
    const float volume = _ui->volumeSlider->value() / 100.0f;
    sound->Set_Priority(priority);
    sound->Set_Volume(volume);

    const int loop_count = _ui->infiniteLoops->isChecked() ? 0 : 1;
    sound->Set_Loop_Count(loop_count);

    const bool is_music = _ui->radioMusic->isChecked();
    sound->Set_Type(is_music ? AudibleSoundClass::TYPE_MUSIC
                             : AudibleSoundClass::TYPE_SOUND_EFFECT);

    float drop_off = static_cast<float>(_ui->dropOffEdit->value());
    float max_vol = static_cast<float>(_ui->maxVolEdit->value());
    float trigger = static_cast<float>(_ui->triggerRadiusEdit->value());

    if (is_3d) {
        sound->Set_DropOff_Radius(drop_off);
        auto *sound_3d = sound->As_Sound3DClass();
        if (sound_3d) {
            sound_3d->Set_Max_Vol_Radius(max_vol);
        }
    } else {
        sound->Set_DropOff_Radius(trigger);
    }

    AudibleSoundDefinitionClass definition;
    definition.Initialize_From_Sound(sound);
    sound->Release_Ref();

    _sound->Set_Sound(&definition);

    if (_ui->stopWhenHidden->isChecked()) {
        _sound->Set_Flags(SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN);
    } else {
        _sound->Set_Flags(0);
    }

    _sound->Set_Name(name_bytes.constData());

    QDialog::accept();
}

void SoundEditDialog::browseSoundFile()
{
    const QString start = _ui->fileEdit->text();
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Select Sound File",
        start,
        "All Sound Files (*.wav *.mp3);;WAV Files (*.wav);;MP3 Files (*.mp3)");
    if (!path.isEmpty()) {
        _ui->fileEdit->setText(QFileInfo(path).fileName());
    }
}

void SoundEditDialog::toggleSoundType()
{
    updateEnableState();
}

void SoundEditDialog::playSound()
{
    const QString filename = _ui->fileEdit->text().trimmed();
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "Play Sound", "No sound file specified.");
        return;
    }

    PlaySoundDialog dialog(filename, this);
    if (dialog.isReady()) {
        dialog.exec();
    }
}

void SoundEditDialog::loadFromSound()
{
    if (!_sound) {
        return;
    }

    const char *name = _sound->Get_Name();
    if (name) {
        _ui->nameEdit->setText(QString::fromLatin1(name));
    }

    bool stop_on_hide = _sound->Get_Flag(SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN);
    float drop_off_radius = kDefaultDropOff;
    float max_vol_radius = kDefaultMaxVol;
    float priority = kDefaultPriority;
    bool is_3d = true;
    bool is_music = false;
    int loop_count = 1;
    float volume = kDefaultVolume;
    QString filename;

    AudibleSoundClass *sound = _sound->Peek_Sound();
    const AudibleSoundDefinitionClass *definition =
        sound ? sound->Get_Definition() : nullptr;
    if (!definition) {
        definition = findPrototypeSoundDefinition(_sound);
    }
    if (sound) {
        const char *runtime_filename = sound->Get_Filename();
        if (runtime_filename && runtime_filename[0] != '\0') {
            filename = QString::fromLocal8Bit(runtime_filename);
        } else if (definition) {
            const char *definition_filename = definition->Get_Filename();
            if (definition_filename && definition_filename[0] != '\0') {
                filename = QString::fromLocal8Bit(definition_filename);
            }
        }
        drop_off_radius = sound->Get_DropOff_Radius();
        priority = sound->Peek_Priority();
        is_3d = sound->As_Sound3DClass() != nullptr;
        is_music = sound->Get_Type() == AudibleSoundClass::TYPE_MUSIC;
        loop_count = sound->Get_Loop_Count();
        volume = sound->Get_Volume();

        auto *sound_3d = sound->As_Sound3DClass();
        if (sound_3d) {
            max_vol_radius = sound_3d->Get_Max_Vol_Radius();
        }
    } else if (definition) {
        const char *definition_filename = definition->Get_Filename();
        if (definition_filename && definition_filename[0] != '\0') {
            filename = QString::fromLocal8Bit(definition_filename);
        }
        drop_off_radius = definition->Get_DropOff_Radius();
        max_vol_radius = definition->Get_Max_Vol_Radius();
        priority = definition->Get_Priority();
        is_3d = definition->Is_3D();
        is_music = definition->Get_Type() == AudibleSoundClass::TYPE_MUSIC;
        loop_count = definition->Get_Loop_Count();
        volume = definition->Get_Volume();
    }

    _ui->fileEdit->setText(filename);
    _ui->infiniteLoops->setChecked(loop_count == 0);
    _ui->radio3d->setChecked(is_3d);
    _ui->radio2d->setChecked(!is_3d);
    _ui->radioMusic->setChecked(is_music);
    _ui->radioEffect->setChecked(!is_music);
    _ui->stopWhenHidden->setChecked(stop_on_hide);
    _ui->volumeSlider->setValue(static_cast<int>(volume * 100.0f));
    _ui->prioritySlider->setValue(static_cast<int>(priority * 100.0f));
    _ui->dropOffEdit->setValue(drop_off_radius);
    _ui->maxVolEdit->setValue(max_vol_radius);
    _ui->triggerRadiusEdit->setValue(drop_off_radius);
}

void SoundEditDialog::updateEnableState()
{
    const bool enable_3d = _ui->radio3d->isChecked();
    _ui->maxVolEdit->setEnabled(enable_3d);
    _ui->dropOffEdit->setEnabled(enable_3d);
    _ui->triggerRadiusEdit->setEnabled(!enable_3d);
}
