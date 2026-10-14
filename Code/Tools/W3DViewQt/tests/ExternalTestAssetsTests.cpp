#include "ExternalTestAssets.h"
#include "ViewerAssetManager.h"
#include "chunkio.h"
#include "htree.h"
#include "w3d_file.h"

#include "mixfile.h"
#include "ramfile.h"
#include "wwmath.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest/QTest>

#include <cstring>

namespace {
bool writeFile(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

bool writeArchive(const QString &path, const QList<QPair<QByteArray, QByteArray>> &entries)
{
    {
        MixFileCreator archive(QFile::encodeName(path).constData());
        for (const auto &entry : entries) {
            QByteArray bytes = entry.second;
            RAMFileClass file(bytes.data(), int(bytes.size()));
            if (!file.Open(FileClass::READ)) return false;
            archive.Add_File(entry.first.constData(), &file);
        }
    }
    return QFileInfo::exists(path);
}

QByteArray readAsset(const char *name)
{
    file_auto_ptr file(_TheFileFactory, name);
    if (!file->Is_Available() || !file->Open(FileClass::READ)) return {};
    QByteArray bytes(file->Size(), Qt::Uninitialized);
    return file->Read(bytes.data(), int(bytes.size())) == bytes.size() ? bytes : QByteArray();
}
} // namespace

class ExternalTestAssetsTests final : public QObject
{
    Q_OBJECT
private slots:
    void gameArchivesAndDependencies_data();
    void gameArchivesAndDependencies();
    void engineLoadsArchivedHierarchy();
    void extractedDirectoriesStillWork();
    void invalidInputDoesNotReplaceFactories();
};

void ExternalTestAssetsTests::gameArchivesAndDependencies_data()
{
    QTest::addColumn<bool>("dataDirectory");
    QTest::newRow("install-root") << false;
    QTest::newRow("data-directory") << true;
}

void ExternalTestAssetsTests::gameArchivesAndDependencies()
{
    QFETCH(bool, dataDirectory);
    QTemporaryDir game;
    QVERIFY(game.isValid());
    QVERIFY(QDir(game.path()).mkdir("Data"));
    const QString data = game.filePath("Data");
    const QString mainArchive = QDir(data).filePath("Always.dat");
    const QString extraArchive = QDir(data).filePath("level.mix");
    const QByteArray audio(220001, char(0x83));
    QVERIFY(writeArchive(mainArchive, {{"MODEL.W3D", "base-model"},
                                      {"override.txt", "base"},
                                      {"dependency.wav", audio}}));
    QVERIFY(writeArchive(QDir(data).filePath("Always2.dat"), {{"MODEL.W3D", "patched-model"}}));
    QVERIFY(writeArchive(extraArchive, {{"texture.tga", "texture-from-another-archive"}}));
    QVERIFY(writeFile(QDir(data).filePath("override.txt"), "loose"));
    const QByteArray originalArchive = readFile(mainArchive);
    FileFactoryClass *previous = _TheFileFactory;
    SimpleFileFactoryClass *previousSimple = _TheSimpleFileFactory;
    QString staged;

    {
        ExternalTestAssets assets(dataDirectory ? data : game.path());
        QVERIFY2(assets.initialize(), qPrintable(assets.error()));
        QCOMPARE(assets.archives().size(), 3);
        QCOMPARE(readAsset("model.w3d"), QByteArray("patched-model"));
        QCOMPARE(readAsset("override.txt"), QByteArray("loose"));
        QCOMPARE(readAsset("texture.tga"), QByteArray("texture-from-another-archive"));
        QCOMPARE(readAsset("dependency.wav"), audio);

        staged = assets.filePath("model.w3d");
        QVERIFY2(!staged.isEmpty(), qPrintable(assets.error()));
        QCOMPARE(readFile(staged), QByteArray("patched-model"));
        QCOMPARE(readFile(assets.filePath("dependency.wav")), audio);
        QCOMPARE(assets.filePath("model.w3d"), staged);
        QVERIFY(!QFileInfo::exists(QDir(data).filePath("model.w3d")));
        QVERIFY(assets.filePath("missing.w3d").isEmpty());
        QVERIFY(assets.error().contains("missing.w3d"));
        QVERIFY(assets.filePath("../outside.w3d").isEmpty());
        QVERIFY(assets.filePath("C:\\outside.w3d").isEmpty());

        // A missing absolute path must not resolve to an archive entry by basename.
        const QByteArray absent = QFile::encodeName(game.filePath("absent/model.w3d"));
        QCOMPARE(readAsset(absent.constData()), QByteArray());
    }
    QCOMPARE(_TheFileFactory, previous);
    QCOMPARE(_TheSimpleFileFactory, previousSimple);
    QVERIFY(!QFileInfo::exists(staged));
    QCOMPARE(readFile(mainArchive), originalArchive);
}

void ExternalTestAssetsTests::engineLoadsArchivedHierarchy()
{
    QByteArray bytes(4096, '\0');
    RAMFileClass file(bytes.data(), int(bytes.size()));
    QVERIFY(file.Open(FileClass::WRITE));
    ChunkSaveClass save(&file);
    W3dHierarchyStruct header{};
    header.Version = W3D_CURRENT_HTREE_VERSION;
    std::strcpy(header.Name, "MIX_TEST");
    header.NumPivots = 1;
    W3dPivotStruct pivot{};
    std::strcpy(pivot.Name, "ROOTTRANSFORM");
    pivot.ParentIdx = static_cast<uint32>(-1);
    pivot.Rotation.Q[3] = 1.0f;
    QVERIFY(save.Begin_Chunk(W3D_CHUNK_HIERARCHY));
    QVERIFY(save.Begin_Chunk(W3D_CHUNK_HIERARCHY_HEADER));
    QCOMPARE(save.Write(&header, sizeof(header)), sizeof(header));
    QVERIFY(save.End_Chunk());
    QVERIFY(save.Begin_Chunk(W3D_CHUNK_PIVOTS));
    QCOMPARE(save.Write(&pivot, sizeof(pivot)), sizeof(pivot));
    QVERIFY(save.End_Chunk());
    QVERIFY(save.End_Chunk());
    bytes.resize(file.Size());

    QTemporaryDir game;
    QVERIFY(game.isValid());
    QVERIFY(writeArchive(game.filePath("Always.dat"), {{"s_mix_test.w3d", bytes}}));
    ExternalTestAssets assets(game.path());
    QVERIFY2(assets.initialize(), qPrintable(assets.error()));
    ViewerAssetManager manager;
    QVERIFY(manager.Load_3D_Assets("s_mix_test.w3d"));
    const HTreeClass *tree = manager.Get_HTree("MIX_TEST");
    QVERIFY(tree);
    QCOMPARE(tree->Num_Pivots(), 1);
    QVERIFY(!QFileInfo::exists(game.filePath("s_mix_test.w3d")));
}

void ExternalTestAssetsTests::extractedDirectoriesStillWork()
{
    QTemporaryDir files;
    QVERIFY(files.isValid());
    QVERIFY(QDir(files.path()).mkdir("w3d"));
    QVERIFY(QDir(files.path()).mkdir("textures"));
    QVERIFY(writeFile(files.filePath("w3d/model.w3d"), "model"));
    QVERIFY(writeFile(files.filePath("textures/picture.tga"), "picture"));
    ExternalTestAssets assets(files.path());
    QVERIFY2(assets.initialize(), qPrintable(assets.error()));
    QCOMPARE(assets.archives().size(), 0);
    QCOMPARE(readFile(assets.filePath("model.w3d")), QByteArray("model"));
    QCOMPARE(readFile(assets.filePath("picture.tga")), QByteArray("picture"));
}

void ExternalTestAssetsTests::invalidInputDoesNotReplaceFactories()
{
    QTemporaryDir files;
    QVERIFY(files.isValid());
    FileFactoryClass *previous = _TheFileFactory;
    {
        ExternalTestAssets missing(files.filePath("does-not-exist"));
        QVERIFY(!missing.initialize());
        QVERIFY(missing.error().contains("does-not-exist"));
        QCOMPARE(_TheFileFactory, previous);
    }
    QVERIFY(writeFile(files.filePath("broken.mix"), "MIX1-truncated"));
    {
        ExternalTestAssets invalid(files.path());
        QVERIFY(!invalid.initialize());
        QVERIFY(invalid.error().contains("broken.mix"));
        QCOMPARE(_TheFileFactory, previous);
    }
    {
        ExternalTestAssets unconfigured(QString{});
        QVERIFY(unconfigured.initialize());
        QVERIFY(!unconfigured.configured());
        QCOMPARE(_TheFileFactory, previous);
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    WWMath::Init();
    int result;
    {
        ExternalTestAssetsTests tests;
        result = QTest::qExec(&tests, argc, argv);
    }
    WWMath::Shutdown();
    return result;
}

#include "ExternalTestAssetsTests.moc"
