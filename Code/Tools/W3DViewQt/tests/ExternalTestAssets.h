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
