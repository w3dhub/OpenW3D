/*
**  Command & Conquer Renegade(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStringList>
#include <functional>
#include <iostream>
#include "w3d_file.h"
#include "wdump_core.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("OpenW3D"));
    QCoreApplication::setApplicationName(QStringLiteral("wdump_qt"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Qt-based WDump viewer"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption dumpTexturesOpt(QStringList() << "t" << "dump-textures", QStringLiteral("Dump texture names to stdout (headless only)."));
    QCommandLineOption noWindowOpt(QStringList() << "q" << "no-window", QStringLiteral("Run without opening a window (headless)."));
    parser.addOption(dumpTexturesOpt);
    parser.addOption(noWindowOpt);
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("Path to a W3D/WLT/WHT/WHA/WTM file to open."));

    parser.process(app);

    const bool headless = parser.isSet(noWindowOpt);
    const bool dumpTextures = parser.isSet(dumpTexturesOpt);
    const auto files = parser.positionalArguments();

    if (headless)
    {
        if (files.isEmpty())
        {
            std::cerr << "No input file specified for headless mode.\n";
            return 1;
        }

        wdump::ChunkFile file;
        if (!file.load(files.first().toStdString()))
        {
            std::cerr << "Failed to load file: " << files.first().toStdString() << "\n";
            return 1;
        }

        if (dumpTextures)
        {
            std::function<void(const wdump::Chunk &)> walk = [&](const wdump::Chunk &chunk) {
                if (chunk.id == W3D_CHUNK_TEXTURE_NAME && !chunk.data.empty())
                {
                    std::cout << reinterpret_cast<const char *>(chunk.data.data()) << "\n";
                }
                for (const auto &child : chunk.children)
                {
                    walk(*child);
                }
            };
            for (const auto &root : file.roots())
            {
                walk(*root);
            }
        }

        return 0;
    }

    MainWindow w;
    if (!files.isEmpty())
    {
        w.loadFile(files.first());
    }
    w.show();

    return app.exec();
}
