#include "RecentFiles.h"

#include <QDir>
#include <QSettings>

namespace {

Qt::CaseSensitivity RecentFilePathCase()
{
#if defined(Q_OS_WIN)
    return Qt::CaseInsensitive;
#else
    return Qt::CaseSensitive;
#endif
}

bool PathsMatch(const QString &lhs, const QString &rhs)
{
    return lhs.compare(rhs, RecentFilePathCase()) == 0;
}

QString NormalizePath(const QString &path)
{
    const QString normalizedSeparators = QDir::fromNativeSeparators(path.trimmed());
    if (normalizedSeparators.isEmpty()) {
        return QString();
    }
    return QDir::cleanPath(normalizedSeparators);
}

QStringList CompactRecentFiles(const QStringList &files, int maxEntries)
{
    QStringList compact;
    for (const QString &entry : files) {
        const QString normalized = NormalizePath(entry);
        if (normalized.isEmpty()) {
            continue;
        }

        bool seen = false;
        for (const QString &existing : compact) {
            if (PathsMatch(existing, normalized)) {
                seen = true;
                break;
            }
        }
        if (seen) {
            continue;
        }

        compact.push_back(normalized);
        if (maxEntries > 0 && compact.size() >= maxEntries) {
            break;
        }
    }

    return compact;
}

} // namespace

namespace qtcommon {

QString NormalizeRecentFilePath(const QString &path)
{
    return NormalizePath(path);
}

QStringList ReadRecentFiles(const QSettings &settings, const QString &key, int maxEntries)
{
    const QStringList files = settings.value(key).toStringList();
    return CompactRecentFiles(files, maxEntries);
}

void WriteRecentFiles(QSettings &settings, const QStringList &files, const QString &key, int maxEntries)
{
    settings.setValue(key, CompactRecentFiles(files, maxEntries));
}

QStringList AddRecentFile(const QStringList &files, const QString &path, int maxEntries)
{
    const QString normalized = NormalizeRecentFilePath(path);
    if (normalized.isEmpty()) {
        return CompactRecentFiles(files, maxEntries);
    }

    QStringList updated;
    updated.push_back(normalized);

    for (const QString &entry : files) {
        const QString normalizedEntry = NormalizeRecentFilePath(entry);
        if (normalizedEntry.isEmpty() || PathsMatch(normalizedEntry, normalized)) {
            continue;
        }
        updated.push_back(normalizedEntry);
    }

    if (maxEntries > 0 && updated.size() > maxEntries) {
        updated = updated.mid(0, maxEntries);
    }

    return updated;
}

QStringList RemoveRecentFile(const QStringList &files, const QString &path)
{
    const QString normalized = NormalizeRecentFilePath(path);
    if (normalized.isEmpty()) {
        return CompactRecentFiles(files, -1);
    }

    QStringList updated;
    for (const QString &entry : files) {
        const QString normalizedEntry = NormalizeRecentFilePath(entry);
        if (normalizedEntry.isEmpty() || PathsMatch(normalizedEntry, normalized)) {
            continue;
        }
        updated.push_back(normalizedEntry);
    }
    return updated;
}

} // namespace qtcommon
