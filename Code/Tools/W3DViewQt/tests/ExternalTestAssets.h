#pragma once

#include "ffactory.h"

#include <QStringList>
#include <QTemporaryDir>

#include <memory>
#include <vector>

class MixFileFactoryClass;

// Installs archive lookup only for the lifetime of an external-asset test process.
// Qt APIs receive temporary files; engine dependencies are read directly from MIX.
class ExternalTestAssets final : public SimpleFileFactoryClass
{
public:
    static QString configuredDirectory();
    explicit ExternalTestAssets(QString directory = configuredDirectory());
    ~ExternalTestAssets() override;

    bool initialize();
    bool configured() const { return !_directory.isEmpty(); }
    QString filePath(const QString &name);
    const QString &error() const { return _error; }
    const QStringList &archives() const { return _archivePaths; }

    FileClass *Get_File(const char *filename) override;

private:
    QString _directory;
    QString _error;
    QStringList _archivePaths;
    QTemporaryDir _staging;
    RawFileFactoryClass _archiveFiles;
    std::vector<std::unique_ptr<MixFileFactoryClass>> _archives;
    FileFactoryClass *_previousFactory = nullptr;
    SimpleFileFactoryClass *_previousSimpleFactory = nullptr;
    bool _installed = false;
};
