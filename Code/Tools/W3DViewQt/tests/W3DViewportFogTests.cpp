#include "W3DViewport.h"

#include "assetmgr.h"
#include "ffactory.h"
#include "sphereobj.h"
#include "wwmath.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QTemporaryDir>
#include <QtTest/QTest>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
class FileFactorySearchPathGuard final
{
public:
    FileFactorySearchPathGuard()
    {
        if (_TheSimpleFileFactory) {
            _TheSimpleFileFactory->Get_Sub_Directory(_original);
        }
    }

    ~FileFactorySearchPathGuard()
    {
        if (_TheSimpleFileFactory) {
            _TheSimpleFileFactory->Set_Sub_Directory(_original.Peek_Buffer());
        }
    }

    void append(const QString &path)
    {
        if (_TheSimpleFileFactory && !path.isEmpty()) {
            const QByteArray native = QDir::toNativeSeparators(path).toLocal8Bit();
            _TheSimpleFileFactory->Append_Sub_Directory(native.constData());
        }
    }

private:
    StringClass _original;
};

QString firstExisting(const QDir &root, std::initializer_list<const char *> relativePaths)
{
    for (const char *relativePath : relativePaths) {
        const QString path = root.filePath(QString::fromLatin1(relativePath));
        if (QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
    return {};
}

QString captureFrame(W3DViewport &viewport, const QString &outputDirectory, const QString &name)
{
    const QString base = QDir(outputDirectory).filePath(name);
    const int number = viewport.captureScreenshot(base);
    if (number <= 0) {
        return {};
    }
    return QStringLiteral("%1%2.tga").arg(base).arg(number, 2, 10, QLatin1Char('0'));
}

QByteArray frameHash(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256);
}

bool writeUncompressedTga(const QImage &source, const QString &path)
{
    const QImage image = source.convertToFormat(QImage::Format_ARGB32);
    if (image.isNull() || image.width() > 65535 || image.height() > 65535) {
        return false;
    }
    QByteArray header(18, '\0');
    header[2] = 2;
    header[12] = static_cast<char>(image.width() & 0xff);
    header[13] = static_cast<char>((image.width() >> 8) & 0xff);
    header[14] = static_cast<char>(image.height() & 0xff);
    header[15] = static_cast<char>((image.height() >> 8) & 0xff);
    header[16] = 32;
    header[17] = 0x28;
    QFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate) || output.write(header) != 18) {
        return false;
    }
    QByteArray row(image.width() * 4, '\0');
    for (int y = 0; y < image.height(); ++y) {
        const QRgb *pixels = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            row[x * 4] = static_cast<char>(qBlue(pixels[x]));
            row[x * 4 + 1] = static_cast<char>(qGreen(pixels[x]));
            row[x * 4 + 2] = static_cast<char>(qRed(pixels[x]));
            row[x * 4 + 3] = static_cast<char>(qAlpha(pixels[x]));
        }
        if (output.write(row) != row.size()) {
            return false;
        }
    }
    return true;
}

QByteArray readTgaPixels(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray bytes = file.readAll();
    if (bytes.size() < 18 || static_cast<unsigned char>(bytes[2]) != 2) {
        return {};
    }
    const int width = static_cast<unsigned char>(bytes[12])
        | (static_cast<unsigned char>(bytes[13]) << 8);
    const int height = static_cast<unsigned char>(bytes[14])
        | (static_cast<unsigned char>(bytes[15]) << 8);
    const int bytesPerPixel = static_cast<unsigned char>(bytes[16]) / 8;
    const int offset = 18 + static_cast<unsigned char>(bytes[0]);
    const int pixelBytes = width * height * bytesPerPixel;
    if (width <= 0 || height <= 0 || (bytesPerPixel != 3 && bytesPerPixel != 4)
        || offset + pixelBytes > bytes.size()) {
        return {};
    }
    return bytes.mid(offset, pixelBytes);
}

double meanPixelDifference(const QByteArray &first, const QByteArray &second)
{
    if (first.isEmpty() || first.size() != second.size()) {
        return 0.0;
    }
    quint64 difference = 0;
    for (qsizetype index = 0; index < first.size(); ++index) {
        difference += std::abs(static_cast<int>(static_cast<unsigned char>(first[index]))
                               - static_cast<int>(static_cast<unsigned char>(second[index])));
    }
    return static_cast<double>(difference) / first.size();
}
} // namespace

class W3DViewportFogTests final : public QObject
{
    Q_OBJECT

private slots:
    void manualClipPlanesRecalculateFogRange();
    void appliedBackgroundsProduceDistinctFrames();
};

void W3DViewportFogTests::manualClipPlanesRecalculateFogRange()
{
#ifndef _WIN32
    QSKIP("The W3D viewport native regression requires Windows and Direct3D.");
#else
    if (QGuiApplication::platformName().compare(QStringLiteral("windows"),
                                                Qt::CaseInsensitive) != 0) {
        QSKIP("The W3D viewport native regression requires the Qt Windows platform plugin.");
    }

    W3DViewport viewport;
    viewport.setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                            Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnBottomHint);
    viewport.setAttribute(Qt::WA_ShowWithoutActivating);
    viewport.resize(320, 240);
    viewport.move(-3200, -3200);

    viewport.setManualClipPlanesEnabled(true);
    viewport.setCameraClipPlanes(10000.0f, 20000.0f);

    const HWND hwnd = reinterpret_cast<HWND>(viewport.winId());
    QVERIFY(hwnd != nullptr);
    QVERIFY(::SetWindowPos(hwnd,
                           HWND_BOTTOM,
                           -3200,
                           -3200,
                           320,
                           240,
                           SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING));

    viewport.show();
    QVERIFY(::SetWindowPos(hwnd,
                           HWND_BOTTOM,
                           -3200,
                           -3200,
                           0,
                           0,
                           SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER |
                               SWP_NOSENDCHANGING));
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    float cameraNear = 0.0f;
    float cameraFar = 0.0f;
    viewport.cameraClipPlanes(cameraNear, cameraFar);
    QCOMPARE(cameraNear, 10000.0f);
    QCOMPARE(cameraFar, 20000.0f);

    float fogNear = 0.0f;
    float fogFar = 0.0f;
    QVERIFY2(viewport.sceneFogRange(fogNear, fogFar),
             "The native viewport did not initialize its W3D scene.");
    QCOMPARE(fogNear, 10000.0f);
    QCOMPARE(fogFar, 10200.0f);

    viewport.setManualClipPlanesEnabled(false);
    auto *sphere = new SphereRenderObjClass;
    sphere->Set_Extent(Vector3(10.0f, 10.0f, 10.0f));
    viewport.setRenderObject(sphere);
    sphere->Release_Ref();

    viewport.cameraClipPlanes(cameraNear, cameraFar);
    QVERIFY(cameraNear < 10000.0f);
    QVERIFY(viewport.sceneFogRange(fogNear, fogFar));
    QCOMPARE(fogNear, cameraNear);

    viewport.setManualClipPlanesEnabled(true);
    viewport.cameraClipPlanes(cameraNear, cameraFar);
    QCOMPARE(cameraNear, 10000.0f);
    QCOMPARE(cameraFar, 20000.0f);
    QVERIFY(viewport.sceneFogRange(fogNear, fogFar));
    QCOMPARE(fogNear, 10000.0f);
    QCOMPARE(fogFar, 10200.0f);

    viewport.setCameraClipPlanes(5000.0f, 6000.0f);
    viewport.cameraClipPlanes(cameraNear, cameraFar);
    QCOMPARE(cameraNear, 5000.0f);
    QCOMPARE(cameraFar, 6000.0f);
    QVERIFY(viewport.sceneFogRange(fogNear, fogFar));
    QCOMPARE(fogNear, 5000.0f);
    QCOMPARE(fogFar, 5200.0f);

    viewport.setRenderObject(nullptr);
    viewport.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
#endif
}

void W3DViewportFogTests::appliedBackgroundsProduceDistinctFrames()
{
#ifndef _WIN32
    QSKIP("The applied-background regression requires Windows and Direct3D.");
#else
    const QString externalRootPath = qEnvironmentVariable("W3DVIEW_EXTERNAL_ASSET_DIR");
    if (externalRootPath.isEmpty()) {
        QSKIP("Set W3DVIEW_EXTERNAL_ASSET_DIR to run the external applied-background regression.");
    }
    if (QGuiApplication::platformName().compare(QStringLiteral("windows"),
                                                Qt::CaseInsensitive) != 0) {
        QSKIP("The applied-background regression requires the Qt Windows platform plugin.");
    }

    const QDir externalRoot(externalRootPath);
    const QString bitmapPath = firstExisting(
        externalRoot, {"textures/mct_screen-fx.tga", "Always/mct_screen-fx.tga"});
    const QString modelPath = firstExisting(
        externalRoot, {"w3d/c_chicken.w3d", "Always/c_chicken.w3d"});
    QVERIFY2(!bitmapPath.isEmpty(), "The supplied mct_screen-fx.tga was not found.");
    QVERIFY2(!modelPath.isEmpty(), "The supplied c_chicken.w3d was not found.");

    // This dump's mct_screen-fx.tga contains BMP bytes. Preserve the source and
    // stage a valid uncompressed TGA for the legacy extension-driven loader.
    QTemporaryDir bitmapStaging;
    QVERIFY(bitmapStaging.isValid());
    QString appliedBitmapPath = bitmapPath;
    QFile bitmapSource(bitmapPath);
    QVERIFY(bitmapSource.open(QIODevice::ReadOnly));
    if (bitmapSource.read(2) == QByteArrayLiteral("BM")) {
        appliedBitmapPath = QDir(bitmapStaging.path()).filePath("mct_screen-fx.tga");
        bitmapSource.close();
        QVERIFY2(writeUncompressedTga(QImage(bitmapPath), appliedBitmapPath),
                 "Could not stage the mislabeled bitmap as TGA without modifying the source.");
    }

    QTemporaryDir temporaryOutput;
    QString outputDirectory = qEnvironmentVariable("W3DVIEW_VALIDATION_OUTPUT_DIR");
    if (outputDirectory.isEmpty()) {
        QVERIFY(temporaryOutput.isValid());
        outputDirectory = temporaryOutput.path();
    } else {
        QVERIFY2(QDir().mkpath(outputDirectory), "Could not create the validation output directory.");
    }

    FileFactorySearchPathGuard searchPaths;
    searchPaths.append(QFileInfo(bitmapPath).absolutePath());
    searchPaths.append(QFileInfo(modelPath).absolutePath());
    searchPaths.append(externalRoot.filePath("textures"));
    searchPaths.append(externalRoot.filePath("Always"));

    auto *assetManager = WW3DAssetManager::Get_Instance();
    QVERIFY(assetManager);
    const QByteArray modelNative = QDir::toNativeSeparators(modelPath).toLocal8Bit();
    QVERIFY2(assetManager->Load_3D_Assets(modelNative.constData()), "c_chicken.w3d failed to load.");
    QVERIFY2(assetManager->Render_Obj_Exists("C_CHICKEN"), "C_CHICKEN was not registered.");

    W3DViewport viewport;
    viewport.setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                            Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnBottomHint);
    viewport.setAttribute(Qt::WA_ShowWithoutActivating);
    viewport.resize(320, 240);
    viewport.move(-3200, -3200);
    const HWND hwnd = reinterpret_cast<HWND>(viewport.winId());
    QVERIFY(hwnd);
    QVERIFY(::SetWindowPos(hwnd, HWND_BOTTOM, -3200, -3200, 320, 240,
                           SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING));
    viewport.show();
    QVERIFY(::SetWindowPos(hwnd, HWND_BOTTOM, -3200, -3200, 0, 0,
                           SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER |
                               SWP_NOSENDCHANGING));
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    viewport.setBackgroundBitmap({});
    viewport.setBackgroundObjectName({});
    viewport.setBackgroundColor(Vector3(0.05f, 0.15f, 0.35f));
    const QString solidPath = captureFrame(viewport, outputDirectory, "background-solid-");

    viewport.setBackgroundBitmap(appliedBitmapPath);
    const QString bitmapOutputPath = captureFrame(viewport, outputDirectory, "background-bitmap-");

    viewport.setBackgroundBitmap({});
    viewport.setBackgroundObjectName(QStringLiteral("C_CHICKEN"));
    const QString objectPath = captureFrame(viewport, outputDirectory, "background-object-");

    const QStringList paths{solidPath, bitmapOutputPath, objectPath};
    QList<QByteArray> pixels;
    QList<QByteArray> hashes;
    for (const QString &path : paths) {
        QVERIFY2(!path.isEmpty(), "Screenshot capture failed.");
        QVERIFY2(QFileInfo(path).size() > 18, qPrintable(QString("Empty screenshot: %1").arg(path)));
        const QByteArray framePixels = readTgaPixels(path);
        QVERIFY2(!framePixels.isEmpty(), qPrintable(QString("Unreadable screenshot: %1").arg(path)));
        pixels.append(framePixels);
        hashes.append(frameHash(path));
        qInfo().noquote() << "W3DVIEW_BACKGROUND_EVIDENCE" << path
                          << hashes.last().toHex();
    }

    QVERIFY(hashes[0] != hashes[1]);
    QVERIFY(hashes[0] != hashes[2]);
    QVERIFY(hashes[1] != hashes[2]);
    QVERIFY2(meanPixelDifference(pixels[0], pixels[1]) >= 0.05,
             "Solid-color and bitmap frames were not materially distinct.");
    QVERIFY2(meanPixelDifference(pixels[0], pixels[2]) >= 0.05,
             "Solid-color and background-object frames were not materially distinct.");
    QVERIFY2(meanPixelDifference(pixels[1], pixels[2]) >= 0.05,
             "Bitmap and background-object frames were not materially distinct.");

    viewport.setBackgroundObjectName({});
    viewport.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
#endif
}

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    WWMath::Init();

    int result = 0;
    {
        WW3DAssetManager assetManager;
        assetManager.Set_WW3D_Load_On_Demand(true);
        assetManager.Set_Activate_Fog_On_Load(true);

        W3DViewportFogTests tests;
        result = QTest::qExec(&tests, argc, argv);
    }

    WWMath::Shutdown();
    return result;
}

#include "W3DViewportFogTests.moc"
