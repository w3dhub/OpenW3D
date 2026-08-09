#include "MainWindow.h"
#include "SaveSettingsDialog.h"
#include "W3DViewport.h"

#include "assetmgr.h"
#include "wwmath.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QLineEdit>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest/QTest>

#include <cmath>

namespace {
constexpr float kTolerance = 0.0001f;

struct LightingState {
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 specular;
    Quaternion orientation;
    float distance = 0.0f;
    float intensity = 0.0f;
    float attenuationStart = 0.0f;
    float attenuationEnd = 0.0f;
    bool attenuationEnabled = false;
};

struct BackgroundState {
    Vector3 color;
    QString bitmap;
    bool fogEnabled = false;
};

bool fuzzyEqual(float actual, float expected)
{
    return std::fabs(actual - expected) <= kTolerance;
}

bool fuzzyEqual(const Vector3 &actual, const Vector3 &expected)
{
    return fuzzyEqual(actual.X, expected.X) && fuzzyEqual(actual.Y, expected.Y)
        && fuzzyEqual(actual.Z, expected.Z);
}

bool fuzzyEqual(const Quaternion &actual, const Quaternion &expected)
{
    return fuzzyEqual(actual.X, expected.X) && fuzzyEqual(actual.Y, expected.Y)
        && fuzzyEqual(actual.Z, expected.Z) && fuzzyEqual(actual.W, expected.W);
}

void setLighting(W3DViewport &viewport, const LightingState &state)
{
    viewport.setAmbientLight(state.ambient);
    viewport.setSceneLightDiffuse(state.diffuse);
    viewport.setSceneLightSpecular(state.specular);
    viewport.setSceneLightOrientation(state.orientation);
    viewport.setSceneLightDistance(state.distance);
    viewport.setSceneLightIntensity(state.intensity);
    viewport.setSceneLightAttenuation(
        state.attenuationStart, state.attenuationEnd, state.attenuationEnabled);
}

void compareLighting(const W3DViewport &viewport, const LightingState &expected)
{
    QVERIFY(fuzzyEqual(viewport.ambientLight(), expected.ambient));
    QVERIFY(fuzzyEqual(viewport.sceneLightDiffuse(), expected.diffuse));
    QVERIFY(fuzzyEqual(viewport.sceneLightSpecular(), expected.specular));
    QVERIFY(fuzzyEqual(viewport.sceneLightOrientation(), expected.orientation));
    QVERIFY(fuzzyEqual(viewport.sceneLightDistance(), expected.distance));
    QVERIFY(fuzzyEqual(viewport.sceneLightIntensity(), expected.intensity));

    float attenuationStart = 0.0f;
    float attenuationEnd = 0.0f;
    bool attenuationEnabled = false;
    viewport.sceneLightAttenuation(
        attenuationStart, attenuationEnd, attenuationEnabled);
    QVERIFY(fuzzyEqual(attenuationStart, expected.attenuationStart));
    QVERIFY(fuzzyEqual(attenuationEnd, expected.attenuationEnd));
    QCOMPARE(attenuationEnabled, expected.attenuationEnabled);
}

void setBackground(W3DViewport &viewport, const BackgroundState &state)
{
    viewport.setBackgroundColor(state.color);
    viewport.setBackgroundBitmap(state.bitmap);
    viewport.setFogEnabled(state.fogEnabled);
}

void compareBackground(const W3DViewport &viewport, const BackgroundState &expected)
{
    QVERIFY(fuzzyEqual(viewport.backgroundColor(), expected.color));
    QCOMPARE(viewport.backgroundBitmap(), expected.bitmap);
    QCOMPARE(viewport.isFogEnabled(), expected.fogEnabled);
}

void writeLighting(QSettings &settings, const LightingState &state)
{
    settings.setValue("AmbientLightR", state.ambient.X);
    settings.setValue("AmbientLightG", state.ambient.Y);
    settings.setValue("AmbientLightB", state.ambient.Z);
    settings.setValue("SceneLightR", state.diffuse.X);
    settings.setValue("SceneLightG", state.diffuse.Y);
    settings.setValue("SceneLightB", state.diffuse.Z);
    settings.setValue("SceneLightDiffuseR", state.diffuse.X);
    settings.setValue("SceneLightDiffuseG", state.diffuse.Y);
    settings.setValue("SceneLightDiffuseB", state.diffuse.Z);
    settings.setValue("SceneLightSpecularR", state.specular.X);
    settings.setValue("SceneLightSpecularG", state.specular.Y);
    settings.setValue("SceneLightSpecularB", state.specular.Z);
    settings.setValue("SceneLightX", state.orientation.X);
    settings.setValue("SceneLightY", state.orientation.Y);
    settings.setValue("SceneLightZ", state.orientation.Z);
    settings.setValue("SceneLightW", state.orientation.W);
    settings.setValue("SceneLightDistance", state.distance);
    settings.setValue("SceneLightIntensity", state.intensity);
    settings.setValue("SceneLightAttenStart", state.attenuationStart);
    settings.setValue("SceneLightAttenEnd", state.attenuationEnd);
    settings.setValue("SceneLightAttenOn", state.attenuationEnabled ? 1 : 0);
}

void writeBackground(QSettings &settings, const BackgroundState &state)
{
    settings.setValue("BackgroundR", state.color.X);
    settings.setValue("BackgroundG", state.color.Y);
    settings.setValue("BackgroundB", state.color.Z);
    settings.setValue("BackgroundBMP", state.bitmap);
    settings.setValue("FogEnabled", state.fogEnabled);
}

bool saveThroughDialog(W3DViewMainWindow &window,
                       const QString &path,
                       bool saveLighting,
                       bool saveBackground,
                       QString &failure)
{
    QAction *saveAction = window.findChild<QAction *>("actionSaveSettings");
    if (!saveAction) {
        failure = "The Save Settings action was not found";
        return false;
    }

    bool interacted = false;
    QTimer::singleShot(0, &window, [&]() {
        auto *dialog = qobject_cast<SaveSettingsDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            failure = "The Save Settings dialog did not become active";
            if (QWidget *modal = QApplication::activeModalWidget()) {
                modal->close();
            }
            return;
        }

        QLineEdit *pathEdit = dialog->findChild<QLineEdit *>("pathLineEdit");
        QCheckBox *lighting = dialog->findChild<QCheckBox *>("lightingCheckBox");
        QCheckBox *background = dialog->findChild<QCheckBox *>("backgroundCheckBox");
        if (!pathEdit || !lighting || !background) {
            failure = "The Save Settings dialog controls were not found";
            dialog->reject();
            return;
        }

        pathEdit->setText(path);
        lighting->setChecked(saveLighting);
        background->setChecked(saveBackground);
        interacted = true;
        dialog->accept();
    });

    saveAction->trigger();
    return interacted && failure.isEmpty();
}
} // namespace

class SettingsSaveMaskTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void backgroundOnlyPreservesAndLoadsExistingLightingKeys();
    void lightingOnlyPreservesAndLoadsExistingBackgroundKeys();

private:
    QTemporaryDir _settingsDirectory;
};

void SettingsSaveMaskTests::initTestCase()
{
    QVERIFY2(_settingsDirectory.isValid(), "Could not create an isolated settings directory");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(
        QSettings::IniFormat, QSettings::UserScope, _settingsDirectory.path());
    QCoreApplication::setOrganizationName("OpenW3DTests");
    QCoreApplication::setApplicationName("W3DViewQtSettingsSaveMaskTests");
}

void SettingsSaveMaskTests::backgroundOnlyPreservesAndLoadsExistingLightingKeys()
{
    const LightingState preservedLighting{
        Vector3(0.11f, 0.22f, 0.33f),
        Vector3(0.44f, 0.55f, 0.66f),
        Vector3(0.77f, 0.88f, 0.99f),
        Quaternion(0.10f, 0.20f, 0.30f, 0.90f),
        123.0f,
        0.65f,
        12.0f,
        456.0f,
        true};
    const BackgroundState savedBackground{
        Vector3(0.15f, 0.35f, 0.55f), QStringLiteral("saved-background.tga"), true};
    const LightingState unsavedLighting{
        Vector3(0.91f, 0.82f, 0.73f),
        Vector3(0.64f, 0.55f, 0.46f),
        Vector3(0.37f, 0.28f, 0.19f),
        Quaternion(-0.10f, 0.15f, -0.20f, 0.95f),
        987.0f,
        0.25f,
        98.0f,
        765.0f,
        false};

    const QString path = QDir(_settingsDirectory.path()).filePath("background-only.dat");
    {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup("Settings");
        writeLighting(settings, preservedLighting);
        settings.endGroup();
        settings.sync();
        QCOMPARE(settings.status(), QSettings::NoError);
    }

    W3DViewMainWindow window;
    W3DViewport *viewport = window.findChild<W3DViewport *>("viewport");
    QVERIFY(viewport);
    setLighting(*viewport, unsavedLighting);
    setBackground(*viewport, savedBackground);

    QString failure;
    QVERIFY2(saveThroughDialog(window, path, false, true, failure), qPrintable(failure));

    setLighting(*viewport, unsavedLighting);
    setBackground(*viewport, BackgroundState{Vector3(0.9f, 0.8f, 0.7f), "changed.tga", false});
    QVERIFY(window.loadSettingsPath(path));
    compareLighting(*viewport, preservedLighting);
    compareBackground(*viewport, savedBackground);
}

void SettingsSaveMaskTests::lightingOnlyPreservesAndLoadsExistingBackgroundKeys()
{
    const BackgroundState preservedBackground{
        Vector3(0.12f, 0.34f, 0.56f), QStringLiteral("preserved-background.dds"), true};
    const LightingState savedLighting{
        Vector3(0.13f, 0.24f, 0.35f),
        Vector3(0.46f, 0.57f, 0.68f),
        Vector3(0.79f, 0.81f, 0.92f),
        Quaternion(0.15f, -0.25f, 0.05f, 0.95f),
        321.0f,
        0.75f,
        21.0f,
        654.0f,
        true};

    const QString path = QDir(_settingsDirectory.path()).filePath("lighting-only.dat");
    {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup("Settings");
        writeBackground(settings, preservedBackground);
        settings.endGroup();
        settings.sync();
        QCOMPARE(settings.status(), QSettings::NoError);
    }

    W3DViewMainWindow window;
    W3DViewport *viewport = window.findChild<W3DViewport *>("viewport");
    QVERIFY(viewport);
    setLighting(*viewport, savedLighting);
    setBackground(*viewport, BackgroundState{Vector3(0.9f, 0.7f, 0.5f), "unsaved.tga", false});

    QString failure;
    QVERIFY2(saveThroughDialog(window, path, true, false, failure), qPrintable(failure));

    setLighting(*viewport,
                LightingState{Vector3(0.9f, 0.8f, 0.7f),
                              Vector3(0.6f, 0.5f, 0.4f),
                              Vector3(0.3f, 0.2f, 0.1f),
                              Quaternion(-0.1f, 0.2f, -0.3f, 0.9f),
                              999.0f,
                              0.1f,
                              99.0f,
                              999.0f,
                              false});
    setBackground(*viewport, BackgroundState{Vector3(0.8f, 0.6f, 0.4f), "changed.dds", false});
    QVERIFY(window.loadSettingsPath(path));
    compareLighting(*viewport, savedLighting);
    compareBackground(*viewport, preservedBackground);
}

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }

    QApplication application(argc, argv);
    WWMath::Init();
    int result = 0;
    {
        WW3DAssetManager assetManager;
        assetManager.Set_WW3D_Load_On_Demand(true);
        SettingsSaveMaskTests tests;
        result = QTest::qExec(&tests, argc, argv);
    }
    WWMath::Shutdown();
    return result;
}

#include "SettingsSaveMaskTests.moc"
