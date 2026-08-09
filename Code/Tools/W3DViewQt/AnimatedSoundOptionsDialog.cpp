#include "AnimatedSoundOptionsDialog.h"

#include "ui_AnimatedSoundOptionsDialog.h"

#include "animatedsoundmgr.h"
#include "chunkio.h"
#include "definitionmgr.h"
#include "ffactory.h"
#include "wwdebug.h"
#include "wwfile.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

namespace {
QString NormalizePath(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return QString();
    }

    return QDir::cleanPath(path.trimmed());
}

QString StartDirectoryForFile(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return QDir::currentPath();
    }

    const QFileInfo info(path);
    if (info.exists()) {
        return info.absolutePath();
    }

    return QFileInfo(path).absolutePath();
}
}

AnimatedSoundOptionsDialog::AnimatedSoundOptionsDialog(const QString &definitionLibraryPath,
                                                       const QString &iniPath,
                                                       const QString &dataPath,
                                                       QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AnimatedSoundOptionsDialog)
{
    _ui->setupUi(this);
    _ui->definitionLibraryEdit->setText(QDir::toNativeSeparators(definitionLibraryPath));
    _ui->iniEdit->setText(QDir::toNativeSeparators(iniPath));
    _ui->dataPathEdit->setText(QDir::toNativeSeparators(dataPath));

    connect(_ui->definitionBrowseButton, &QPushButton::clicked, this,
            &AnimatedSoundOptionsDialog::browseDefinitionLibrary);
    connect(_ui->iniBrowseButton, &QPushButton::clicked, this,
            &AnimatedSoundOptionsDialog::browseIniPath);
    connect(_ui->dataBrowseButton, &QPushButton::clicked, this,
            &AnimatedSoundOptionsDialog::browseDataPath);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AnimatedSoundOptionsDialog::~AnimatedSoundOptionsDialog()
{
    delete _ui;
}

QString AnimatedSoundOptionsDialog::definitionLibraryPath() const
{
    return _ui->definitionLibraryEdit->text().trimmed();
}

QString AnimatedSoundOptionsDialog::iniPath() const
{
    return _ui->iniEdit->text().trimmed();
}

QString AnimatedSoundOptionsDialog::dataPath() const
{
    return _ui->dataPathEdit->text().trimmed();
}

void AnimatedSoundOptionsDialog::LoadAnimatedSoundSettings()
{
    DefinitionMgrClass::Free_Definitions();

    QSettings settings;
    const QString definition_path = NormalizePath(settings.value("Config/SoundDefLibPath").toString());
    const QString ini_path = NormalizePath(settings.value("Config/AnimSoundINIPath").toString());
    const QString data_path = NormalizePath(settings.value("Config/AnimSoundDataPath").toString());

    if (_TheFileFactory && !definition_path.isEmpty()) {
        const QByteArray native = QDir::toNativeSeparators(definition_path).toLocal8Bit();
        FileClass *file = _TheFileFactory->Get_File(native.constData());
        if (file != nullptr) {
            file->Open(FileClass::READ);
            ChunkLoadClass cload(file);
            SaveLoadSystemClass::Load(cload);
            file->Close();
            _TheFileFactory->Return_File(file);
        } else {
            WWDEBUG_SAY(("Failed to load file %s\n", native.constData()));
        }
    }

    AnimatedSoundMgrClass::Shutdown();
    if (ini_path.isEmpty()) {
        AnimatedSoundMgrClass::Initialize("");
    } else {
        const QByteArray native = QDir::toNativeSeparators(ini_path).toLocal8Bit();
        AnimatedSoundMgrClass::Initialize(native.constData());
    }

    if (_TheSimpleFileFactory && !data_path.isEmpty()) {
        const QByteArray native = QDir::toNativeSeparators(data_path).toLocal8Bit();
        _TheSimpleFileFactory->Append_Sub_Directory(native.constData());
    }
}

void AnimatedSoundOptionsDialog::browseDefinitionLibrary()
{
    const QString start = _ui->definitionLibraryEdit->text();
    const QString initial_dir = StartDirectoryForFile(start);
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Sound Preset Library",
        initial_dir,
        "Definition Database Files (*.ddb)");
    if (!path.isEmpty()) {
        _ui->definitionLibraryEdit->setText(QDir::toNativeSeparators(path));
    }
}

void AnimatedSoundOptionsDialog::browseIniPath()
{
    const QString start = _ui->iniEdit->text();
    const QString initial_dir = StartDirectoryForFile(start);
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Animated Sound INI",
        initial_dir,
        "INI Files (*.ini)");
    if (!path.isEmpty()) {
        _ui->iniEdit->setText(QDir::toNativeSeparators(path));
    }
}

void AnimatedSoundOptionsDialog::browseDataPath()
{
    const QString start = _ui->dataPathEdit->text();
    const QString dir = QFileDialog::getExistingDirectory(this, "Pick Sound Path", start);
    if (!dir.isEmpty()) {
        _ui->dataPathEdit->setText(QDir::toNativeSeparators(dir));
    }
}
