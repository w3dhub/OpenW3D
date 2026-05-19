#pragma once

#include <QString>
#include <QStringList>

class QSettings;

namespace qtcommon {

QString NormalizeRecentFilePath(const QString &path);

QStringList ReadRecentFiles(const QSettings &settings,
                            const QString &key = QStringLiteral("recentFiles"),
                            int maxEntries = 10);

void WriteRecentFiles(QSettings &settings,
                      const QStringList &files,
                      const QString &key = QStringLiteral("recentFiles"),
                      int maxEntries = 10);

QStringList AddRecentFile(const QStringList &files, const QString &path, int maxEntries = 10);

QStringList RemoveRecentFile(const QStringList &files, const QString &path);

} // namespace qtcommon
