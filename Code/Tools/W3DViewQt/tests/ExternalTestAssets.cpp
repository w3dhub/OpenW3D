#include "ExternalTestAssets.h"

#include "mixfile.h"
#include "rawfile.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <algorithm>
#include <limits>
#include <utility>

namespace {
bool validArchive(const QString &path, QString &error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        error = file.errorString();
        return false;
    }
    // The engine reader uses signed int offsets. Check the directory before it
    // allocates an index or constructs biased views into the archive.
    if (file.size() < 20 || file.size() > std::numeric_limits<int>::max()
        || file.read(4) != "MIX1") {
        error = "Expected a Renegade MIX1 archive smaller than 2 GiB";
        return false;
    }
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    quint32 indexOffset, namesOffset, reserved;
    stream >> indexOffset >> namesOffset >> reserved;
    if (indexOffset < 16 || indexOffset > file.size() - 4 || !file.seek(indexOffset)) {
        error = "Invalid MIX index offset";
        return false;
    }
    quint32 count;
    stream >> count;
    if (count > (file.size() - indexOffset - 4) / 12) {
        error = "Truncated MIX index";
        return false;
    }
    quint32 previousCrc = 0;
    for (quint32 index = 0; index < count; ++index) {
        quint32 crc, offset, size;
        stream >> crc >> offset >> size;
        if (stream.status() != QDataStream::Ok || (index && crc < previousCrc)
            || offset < 16 || quint64(offset) + size > quint64(file.size())) {
            error = "Invalid MIX entry";
            return false;
        }
        previousCrc = crc;
    }
    return true;
}

int archivePriority(const QString &path)
{
    const QString name = QFileInfo(path).fileName().toLower();
    if (name == "always2.dat") return 0;
    if (name == "always.dbs") return 1;
    if (name == "always.dat") return 2;
    return 3;
}
} // namespace

QString ExternalTestAssets::configuredDirectory()
{
    const QString game = qEnvironmentVariable("W3DVIEW_GAME_DIR").trimmed();
    return game.isEmpty() ? qEnvironmentVariable("W3DVIEW_EXTERNAL_ASSET_DIR").trimmed() : game;
}

ExternalTestAssets::ExternalTestAssets(QString directory) : _directory(std::move(directory)) {}

ExternalTestAssets::~ExternalTestAssets()
{
    if (_installed) {
        _TheFileFactory = _previousFactory;
        _TheSimpleFileFactory = _previousSimpleFactory;
    }
}

bool ExternalTestAssets::initialize()
{
    if (!configured() || _installed) return true;
    const QFileInfo root(_directory);
    if (!root.isDir()) {
        _error = QString("Asset directory does not exist: %1").arg(_directory);
        return false;
    }
    if (!_staging.isValid()) {
        _error = "Could not create a temporary asset directory";
        return false;
    }
    _directory = root.absoluteFilePath();
    QStringList directories;
    const auto append = [&](const QString &path) {
        const QString canonical = QFileInfo(path).canonicalFilePath();
        if (!canonical.isEmpty() && QFileInfo(canonical).isDir()
            && !directories.contains(canonical, Qt::CaseInsensitive))
            directories.append(canonical);
    };
    // Accept the install root, Data itself, and older extracted-asset layouts.
    append(QDir(_directory).filePath("Data"));
    append(QDir(_directory).filePath("data"));
    append(_directory);
    const QStringList roots = directories;
    for (const QString &directory : roots) {
        for (const char *child : {"Always", "w3d", "textures"})
            append(QDir(directory).filePath(QString::fromLatin1(child)));
    }

    _archives.clear();
    _archivePaths.clear();
    for (const QString &directory : roots) {
        const QFileInfoList entries = QDir(directory).entryInfoList(QDir::Files, QDir::Name);
        QStringList paths;
        for (const QFileInfo &entry : entries) {
            const QString name = entry.fileName().toLower();
            if (entry.suffix().compare("mix", Qt::CaseInsensitive) == 0
                || name == "always.dat" || name == "always2.dat" || name == "always.dbs")
                paths.append(entry.absoluteFilePath());
        }
        std::stable_sort(paths.begin(), paths.end(), [](const QString &a, const QString &b) {
            return archivePriority(a) < archivePriority(b);
        });
        for (const QString &path : paths) {
            QString detail;
            if (!validArchive(path, detail)) {
                _error = QString("Cannot read archive %1: %2").arg(path, detail);
                return false;
            }
            const QByteArray native = QFile::encodeName(QDir::toNativeSeparators(path));
            auto archive = std::make_unique<MixFileFactoryClass>(native.constData(), &_archiveFiles);
            if (!archive->Is_Valid()) {
                _error = QString("The engine could not open archive: %1").arg(path);
                return false;
            }
            _archivePaths.append(path);
            _archives.push_back(std::move(archive));
        }
    }

    _previousFactory = _TheFileFactory;
    _previousSimpleFactory = _TheSimpleFileFactory;
    if (_previousSimpleFactory) {
        StringClass paths;
        _previousSimpleFactory->Get_Sub_Directory(paths);
        Set_Sub_Directory(paths.Peek_Buffer());
    }
    for (const QString &directory : directories)
        Append_Sub_Directory(QFile::encodeName(QDir::toNativeSeparators(directory)).constData());
    _TheFileFactory = this;
    _TheSimpleFileFactory = this;
    _installed = true;
    qInfo().noquote() << "External assets:" << _directory << "-"
                      << _archivePaths.size() << "MIX archives";
    for (const QString &path : _archivePaths)
        qInfo().noquote() << "  Archive:" << path;
    return true;
}

FileClass *ExternalTestAssets::Get_File(const char *filename)
{
    FileClass *loose = SimpleFileFactoryClass::Get_File(filename);
    const QString requested = QString::fromLocal8Bit(filename);
    if ((loose && loose->Is_Available()) || QFileInfo(requested).isAbsolute())
        return loose;
    const QByteArray name = QFileInfo(QString(requested).replace('\\', '/')).fileName().toLocal8Bit();
    for (const auto &archive : _archives) {
        if (FileClass *file = archive->Get_File(name.constData())) {
            Return_File(loose);
            return file;
        }
    }
    return loose;
}

QString ExternalTestAssets::filePath(const QString &name)
{
    _error.clear();
    if (!_installed || name.isEmpty() || name == "." || name == ".."
        || name.contains('/') || name.contains('\\') || name.contains(':')) {
        _error = QString("Invalid external asset request: %1").arg(name);
        return {};
    }
    const QString destination = _staging.filePath(name);
    if (QFileInfo::exists(destination)) return destination;

    const QByteArray native = name.toLocal8Bit();
    file_auto_ptr source(this, native.constData());
    if (!source->Is_Available() || !source->Open(FileClass::READ)) {
        _error = QString("Missing required asset '%1' in %2 (%3 archives). "
                         "Use a complete Renegade installation with the original assets.")
                     .arg(name, _directory).arg(_archives.size());
        return {};
    }
    QSaveFile output(destination);
    if (!output.open(QIODevice::WriteOnly)) {
        _error = output.errorString();
        return {};
    }
    int remaining = source->Size();
    while (remaining > 0) {
        char buffer[65536];
        const int amount = std::min(remaining, int(sizeof(buffer)));
        if (source->Read(buffer, amount) != amount || output.write(buffer, amount) != amount) {
            _error = QString("Could not copy required asset: %1").arg(name);
            return {};
        }
        remaining -= amount;
    }
    if (!output.commit()) {
        _error = output.errorString();
        return {};
    }
    return destination;
}
