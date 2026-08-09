#include "W3DExportUtils.h"

#include "chunkio.h"
#include "ramfile.h"
#include "rawfile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest/QTest>

#include <array>

namespace
{
constexpr std::uint32_t ExpectedChunk = 0x13572468U;
constexpr std::uint32_t WrongChunk = 0x24681357U;

bool WriteBytes(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(bytes) == bytes.size();
}

QByteArray ReadBytes(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

bool WriteSingleChunk(ChunkSaveClass &chunk_save,
                      std::uint32_t chunk_id,
                      const QByteArray &payload)
{
    if (!chunk_save.Begin_Chunk(chunk_id)) {
        return false;
    }

    const bool payload_written =
        chunk_save.Write(payload.constData(), static_cast<std::size_t>(payload.size())) ==
        static_cast<std::uint32_t>(payload.size());
    const bool chunk_closed = chunk_save.End_Chunk();
    return payload_written && chunk_closed;
}

QStringList DirectoryEntries(const QString &path)
{
    return QDir(path).entryList(QDir::AllEntries | QDir::Hidden | QDir::System |
                                    QDir::NoDotAndDotDot,
                                QDir::Name);
}
}

class W3DExportUtilsTests : public QObject
{
    Q_OBJECT

private slots:
    void failedWriterPreservesExistingFile();
    void unbalancedWriterPreservesExistingFile();
    void wrongTopLevelChunkPreservesExistingFile();
    void successfulExportAtomicallyReplacesAndParses();
    void nonexistentParentFailsWithoutCreatingAnything();
    void chunkWriterRetainsShortWriteFailure();
};

void W3DExportUtilsTests::failedWriterPreservesExistingFile()
{
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());

    const QString target = QDir(temporary_directory.path()).filePath("asset.w3d");
    const QByteArray sentinel("original-sentinel");
    QVERIFY(WriteBytes(target, sentinel));

    bool serialization_succeeded = false;
    QString error_message;
    const bool saved = W3DExportUtils::SaveChunkFileAtomically(
        target,
        ExpectedChunk,
        [&](ChunkSaveClass &chunk_save) {
            serialization_succeeded =
                WriteSingleChunk(chunk_save, ExpectedChunk, QByteArray("replacement"));
            return false;
        },
        &error_message);

    QVERIFY(serialization_succeeded);
    QVERIFY(!saved);
    QVERIFY(error_message.contains("writer", Qt::CaseInsensitive));
    QCOMPARE(ReadBytes(target), sentinel);
    QCOMPARE(DirectoryEntries(temporary_directory.path()), QStringList{"asset.w3d"});
}

void W3DExportUtilsTests::unbalancedWriterPreservesExistingFile()
{
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());

    const QString target = QDir(temporary_directory.path()).filePath("asset.w3d");
    const QByteArray sentinel("original-sentinel");
    QVERIFY(WriteBytes(target, sentinel));

    QString error_message;
    const bool saved = W3DExportUtils::SaveChunkFileAtomically(
        target,
        ExpectedChunk,
        [](ChunkSaveClass &chunk_save) { return chunk_save.Begin_Chunk(ExpectedChunk); },
        &error_message);

    QVERIFY(!saved);
    QVERIFY(error_message.contains("unbalanced", Qt::CaseInsensitive));
    QCOMPARE(ReadBytes(target), sentinel);
    QCOMPARE(DirectoryEntries(temporary_directory.path()), QStringList{"asset.w3d"});
}

void W3DExportUtilsTests::wrongTopLevelChunkPreservesExistingFile()
{
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());

    const QString target = QDir(temporary_directory.path()).filePath("asset.w3d");
    const QByteArray sentinel("original-sentinel");
    QVERIFY(WriteBytes(target, sentinel));

    QString error_message;
    const bool saved = W3DExportUtils::SaveChunkFileAtomically(
        target,
        ExpectedChunk,
        [](ChunkSaveClass &chunk_save) {
            return WriteSingleChunk(chunk_save, WrongChunk, QByteArray("wrong chunk"));
        },
        &error_message);

    QVERIFY(!saved);
    QVERIFY(error_message.contains("top-level chunk", Qt::CaseInsensitive));
    QCOMPARE(ReadBytes(target), sentinel);
    QCOMPARE(DirectoryEntries(temporary_directory.path()), QStringList{"asset.w3d"});
}

void W3DExportUtilsTests::successfulExportAtomicallyReplacesAndParses()
{
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());

    const QString target = QDir(temporary_directory.path()).filePath("asset.w3d");
    QVERIFY(WriteBytes(target, QByteArray("original-sentinel")));
    const QByteArray payload("replacement-payload");

    QString error_message("stale error");
    const bool saved = W3DExportUtils::SaveChunkFileAtomically(
        target,
        ExpectedChunk,
        [&](ChunkSaveClass &chunk_save) {
            return WriteSingleChunk(chunk_save, ExpectedChunk, payload);
        },
        &error_message);

    QVERIFY2(saved, qPrintable(error_message));
    QVERIFY(error_message.isEmpty());

    const QByteArray native_target =
        QFile::encodeName(QDir::toNativeSeparators(QFileInfo(target).absoluteFilePath()));
    RawFileClass file(native_target.constData());
    QVERIFY(file.Open(FileClass::READ));
    const int file_size = file.Size();

    ChunkLoadClass chunk_load(&file);
    QVERIFY(chunk_load.Open_Chunk());
    QCOMPARE(static_cast<std::uint32_t>(chunk_load.Cur_Chunk_ID()), ExpectedChunk);
    QCOMPARE(static_cast<std::uint32_t>(chunk_load.Cur_Chunk_Length()),
             static_cast<std::uint32_t>(payload.size()));

    QByteArray parsed_payload(payload.size(), '\0');
    QCOMPARE(static_cast<std::uint32_t>(
                 chunk_load.Read(parsed_payload.data(), static_cast<std::size_t>(parsed_payload.size()))),
             static_cast<std::uint32_t>(payload.size()));
    QCOMPARE(parsed_payload, payload);
    QVERIFY(chunk_load.Close_Chunk());
    QCOMPARE(file.Tell(), file_size);
    file.Close();

    QCOMPARE(DirectoryEntries(temporary_directory.path()), QStringList{"asset.w3d"});
}

void W3DExportUtilsTests::nonexistentParentFailsWithoutCreatingAnything()
{
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());

    const QString missing_directory =
        QDir(temporary_directory.path()).filePath("missing-parent");
    const QString target = QDir(missing_directory).filePath("asset.w3d");

    QString error_message;
    const bool saved = W3DExportUtils::SaveChunkFileAtomically(
        target,
        ExpectedChunk,
        [](ChunkSaveClass &chunk_save) {
            return WriteSingleChunk(chunk_save, ExpectedChunk, QByteArray("payload"));
        },
        &error_message);

    QVERIFY(!saved);
    QVERIFY(error_message.contains("directory", Qt::CaseInsensitive));
    QVERIFY(!QFileInfo::exists(target));
    QVERIFY(!QFileInfo::exists(missing_directory));
    QVERIFY(DirectoryEntries(temporary_directory.path()).isEmpty());
}

void W3DExportUtilsTests::chunkWriterRetainsShortWriteFailure()
{
    std::array<unsigned char, 12> storage{};
    RAMFileClass file(storage.data(), static_cast<int>(storage.size()));
    QVERIFY(file.Open(FileClass::WRITE));

    ChunkSaveClass chunkSave(&file);
    QVERIFY(chunkSave.Begin_Chunk(ExpectedChunk));

    const std::array<unsigned char, 8> payload{};
    QCOMPARE(chunkSave.Write(payload.data(), payload.size()), std::uint32_t{0});
    QVERIFY(chunkSave.Has_Write_Error());

    // Rewriting the top header still fits, but a successful structural close must
    // not erase the earlier short-write failure.
    QVERIFY(!chunkSave.End_Chunk());
    QCOMPARE(chunkSave.Cur_Chunk_Depth(), 0);
    QVERIFY(chunkSave.Has_Write_Error());
    file.Close();
}

QTEST_APPLESS_MAIN(W3DExportUtilsTests)

#include "W3DExportUtilsTests.moc"
