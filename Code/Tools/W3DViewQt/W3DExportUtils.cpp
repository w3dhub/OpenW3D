#include "W3DExportUtils.h"

#include "chunkio.h"
#include "rawfile.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTemporaryFile>

#include <exception>
#include <utility>

namespace
{
bool Fail(QString *error_message, const QString &message)
{
    if (error_message) {
        *error_message = message;
    }
    return false;
}

QByteArray NativeFileName(const QString &path)
{
    return QFile::encodeName(QDir::toNativeSeparators(path));
}

class TemporaryFileCleanup
{
public:
    explicit TemporaryFileCleanup(QString path)
        : path_(std::move(path))
    {
    }

    ~TemporaryFileCleanup()
    {
        if (!path_.isEmpty()) {
            QFile::remove(path_);
        }
    }

    TemporaryFileCleanup(const TemporaryFileCleanup &) = delete;
    TemporaryFileCleanup &operator=(const TemporaryFileCleanup &) = delete;

private:
    QString path_;
};

bool ValidateStagedFile(const QString &path,
                        std::uint32_t expected_top_level_chunk,
                        QString *error_message)
{
    const QByteArray native_path = NativeFileName(path);
    RawFileClass file(native_path.constData());
    if (!file.Open(FileClass::READ)) {
        return Fail(error_message,
                    QStringLiteral("Could not reopen the temporary W3D export for validation."));
    }

    const auto validation_failure = [&](const QString &message) {
        file.Close();
        return Fail(error_message, message);
    };

    const int file_size = file.Size();
    if (file_size < static_cast<int>(sizeof(ChunkHeader))) {
        return validation_failure(
            QStringLiteral("The temporary W3D export is too small to contain a chunk header."));
    }

    ChunkLoadClass chunk_load(&file);
    if (!chunk_load.Open_Chunk()) {
        return validation_failure(
            QStringLiteral("The temporary W3D export does not contain a readable top-level chunk."));
    }

    const std::uint32_t actual_top_level_chunk = chunk_load.Cur_Chunk_ID();
    if (actual_top_level_chunk != expected_top_level_chunk) {
        return validation_failure(
            QStringLiteral("The temporary W3D export has top-level chunk 0x%1; expected 0x%2.")
                .arg(static_cast<qulonglong>(actual_top_level_chunk), 8, 16, QLatin1Char('0'))
                .arg(static_cast<qulonglong>(expected_top_level_chunk), 8, 16, QLatin1Char('0')));
    }

    const quint64 declared_file_size =
        static_cast<quint64>(chunk_load.Cur_Chunk_Length()) + sizeof(ChunkHeader);
    if (declared_file_size != static_cast<quint64>(file_size)) {
        return validation_failure(
            QStringLiteral("The temporary W3D export's top-level chunk does not span the entire file."));
    }

    if (!chunk_load.Close_Chunk()) {
        return validation_failure(
            QStringLiteral("The temporary W3D export's top-level chunk could not be closed."));
    }

    if (file.Tell() != file_size) {
        return validation_failure(
            QStringLiteral("The temporary W3D export did not end at the top-level chunk boundary."));
    }

    file.Close();
    return true;
}
}

namespace W3DExportUtils
{
bool SaveChunkFileAtomically(const QString &target_path,
                             std::uint32_t expected_top_level_chunk,
                             const ChunkWriter &writer,
                             QString *error_message)
{
    if (error_message) {
        error_message->clear();
    }

    if (target_path.isEmpty()) {
        return Fail(error_message, QStringLiteral("No W3D export filename was provided."));
    }
    if (!writer) {
        return Fail(error_message, QStringLiteral("No W3D export writer was provided."));
    }

    const QFileInfo target_info(target_path);
    if (target_info.fileName().isEmpty()) {
        return Fail(error_message,
                    QStringLiteral("The W3D export path does not contain a filename."));
    }

    const QString absolute_target_path = target_info.absoluteFilePath();
    const QDir target_directory = target_info.absoluteDir();
    if (!target_directory.exists()) {
        return Fail(error_message,
                    QStringLiteral("The W3D export directory does not exist: %1")
                        .arg(QDir::toNativeSeparators(target_directory.absolutePath())));
    }

    const QString temporary_template = target_directory.filePath(
        QStringLiteral(".%1.w3dview-XXXXXX.tmp").arg(target_info.fileName()));
    QString staged_path;
    {
        QTemporaryFile staged_file(temporary_template);
        staged_file.setAutoRemove(true);
        if (!staged_file.open()) {
            return Fail(error_message,
                        QStringLiteral("Could not create a temporary W3D export beside %1: %2")
                            .arg(QDir::toNativeSeparators(absolute_target_path),
                                 staged_file.errorString()));
        }

        staged_path = staged_file.fileName();
        staged_file.setAutoRemove(false);
        staged_file.close();
    }
    const TemporaryFileCleanup staged_file_cleanup(staged_path);

    const QByteArray native_staged_path = NativeFileName(staged_path);
    RawFileClass raw_file(native_staged_path.constData());
    if (!raw_file.Open(FileClass::WRITE)) {
        return Fail(error_message,
                    QStringLiteral("Could not open the temporary W3D export for writing: %1")
                        .arg(QDir::toNativeSeparators(staged_path)));
    }

    bool writer_succeeded = false;
    bool writer_threw = false;
    bool chunk_write_error = false;
    QString writer_exception;
    int chunk_depth = 0;
    {
        ChunkSaveClass chunk_save(&raw_file);
        try {
            writer_succeeded = writer(chunk_save);
        } catch (const std::exception &exception) {
            writer_threw = true;
            writer_exception = QString::fromLocal8Bit(exception.what());
        } catch (...) {
            writer_threw = true;
        }
        chunk_depth = chunk_save.Cur_Chunk_Depth();
        chunk_write_error = chunk_save.Has_Write_Error();
    }
    raw_file.Close();

    if (writer_threw) {
        const QString detail = writer_exception.isEmpty()
            ? QStringLiteral("unknown exception")
            : writer_exception;
        return Fail(error_message,
                    QStringLiteral("The W3D export writer raised an exception: %1").arg(detail));
    }
    if (chunk_write_error) {
        return Fail(error_message,
                    QStringLiteral("The W3D export writer encountered a failed write or invalid chunk operation."));
    }
    if (!writer_succeeded) {
        return Fail(error_message, QStringLiteral("The W3D export writer reported a failure."));
    }
    if (chunk_depth != 0) {
        return Fail(error_message,
                    QStringLiteral("The W3D export writer left %1 chunk(s) unbalanced.")
                        .arg(chunk_depth));
    }

    if (!ValidateStagedFile(staged_path, expected_top_level_chunk, error_message)) {
        return false;
    }

    QFile staged_input(staged_path);
    if (!staged_input.open(QIODevice::ReadOnly)) {
        return Fail(error_message,
                    QStringLiteral("Could not read the validated temporary W3D export: %1")
                        .arg(staged_input.errorString()));
    }

    QSaveFile destination(absolute_target_path);
    destination.setDirectWriteFallback(false);
    if (!destination.open(QIODevice::WriteOnly)) {
        return Fail(error_message,
                    QStringLiteral("Could not prepare %1 for atomic replacement: %2")
                        .arg(QDir::toNativeSeparators(absolute_target_path),
                             destination.errorString()));
    }

    constexpr qint64 copy_block_size = 64 * 1024;
    while (true) {
        const QByteArray block = staged_input.read(copy_block_size);
        if (block.isEmpty()) {
            if (staged_input.error() != QFileDevice::NoError) {
                const QString detail = staged_input.errorString();
                destination.cancelWriting();
                return Fail(error_message,
                            QStringLiteral("Could not read the temporary W3D export: %1")
                                .arg(detail));
            }
            break;
        }

        qint64 offset = 0;
        while (offset < block.size()) {
            const qint64 written =
                destination.write(block.constData() + offset, block.size() - offset);
            if (written <= 0) {
                const QString detail = destination.errorString();
                destination.cancelWriting();
                return Fail(error_message,
                            QStringLiteral("Could not write the atomic W3D replacement: %1")
                                .arg(detail));
            }
            offset += written;
        }
    }

    if (!destination.commit()) {
        return Fail(error_message,
                    QStringLiteral("Could not atomically replace %1: %2")
                        .arg(QDir::toNativeSeparators(absolute_target_path),
                             destination.errorString()));
    }

    return true;
}
}
