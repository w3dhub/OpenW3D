#include "SceneLightDialog.h"
#include "W3DViewport.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QSlider>
#include <QtTest/QTest>

#include <cmath>

namespace {
constexpr float kFloatTolerance = 0.0001f;

bool fuzzyEqual(float actual, float expected)
{
    return std::fabs(actual - expected) <= kFloatTolerance;
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

void compareState(const W3DViewport::SceneLightState &actual,
                  const W3DViewport::SceneLightState &expected)
{
    QVERIFY2(fuzzyEqual(actual.diffuse, expected.diffuse), "Diffuse color did not round-trip");
    QVERIFY2(fuzzyEqual(actual.specular, expected.specular), "Specular color did not round-trip");
    QVERIFY2(fuzzyEqual(actual.orientation, expected.orientation),
             "Orientation did not round-trip");
    QVERIFY2(fuzzyEqual(actual.distance, expected.distance), "Distance did not round-trip");
    QVERIFY2(fuzzyEqual(actual.intensity, expected.intensity), "Intensity did not round-trip");
    QVERIFY2(fuzzyEqual(actual.attenuationStart, expected.attenuationStart),
             "Attenuation start did not round-trip");
    QVERIFY2(fuzzyEqual(actual.attenuationEnd, expected.attenuationEnd),
             "Attenuation end did not round-trip");
    QCOMPARE(actual.attenuationEnabled, expected.attenuationEnabled);
    QCOMPARE(actual.orientationExplicit, expected.orientationExplicit);
    QCOMPARE(actual.distanceExplicit, expected.distanceExplicit);
}
} // namespace

class SceneLightTests final : public QObject
{
    Q_OBJECT

private slots:
    void stateRoundTripPreservesIndependentChannelsAndFlags();
    void cancelRestoresCompleteInitialState();
    void channelSelectionUpdatesOnlySelectedChannel();
    void grayscaleHonorsChannelSelection();
    void designerFormExposesRequiredControlsAndRanges();
};

void SceneLightTests::stateRoundTripPreservesIndependentChannelsAndFlags()
{
    W3DViewport viewport;

    W3DViewport::SceneLightState first;
    first.diffuse = Vector3(0.25f, 0.50f, 0.75f);
    first.specular = Vector3(0.75f, 0.25f, 0.50f);
    first.orientation = Quaternion(0.10f, 0.20f, 0.30f, 0.90f);
    first.distance = 125.5f;
    first.intensity = 0.65f;
    first.attenuationStart = 15.25f;
    first.attenuationEnd = 240.75f;
    first.attenuationEnabled = true;
    first.orientationExplicit = true;
    first.distanceExplicit = false;

    viewport.setSceneLightState(first);
    compareState(viewport.sceneLightState(), first);

    W3DViewport::SceneLightState second = first;
    second.diffuse = Vector3(0.60f, 0.40f, 0.20f);
    second.specular = Vector3(0.10f, 0.30f, 0.90f);
    second.orientation = Quaternion(-0.20f, 0.30f, -0.10f, 0.90f);
    second.distance = 42.0f;
    second.attenuationEnabled = false;
    second.orientationExplicit = false;
    second.distanceExplicit = true;

    viewport.setSceneLightState(second);
    compareState(viewport.sceneLightState(), second);
}

void SceneLightTests::cancelRestoresCompleteInitialState()
{
    W3DViewport viewport;
    W3DViewport::SceneLightState initial;
    initial.diffuse = Vector3(0.25f, 0.50f, 0.75f);
    initial.specular = Vector3(0.75f, 0.50f, 0.25f);
    initial.orientation = Quaternion(0.15f, -0.25f, 0.05f, 0.95f);
    initial.distance = 12.5f;
    initial.intensity = 0.60f;
    initial.attenuationStart = 7.5f;
    initial.attenuationEnd = 80.0f;
    initial.attenuationEnabled = false;
    initial.orientationExplicit = false;
    initial.distanceExplicit = false;
    viewport.setSceneLightState(initial);

    SceneLightDialog dialog(viewport);
    auto *redSlider = dialog.findChild<QSlider *>("redSlider");
    auto *greenSlider = dialog.findChild<QSlider *>("greenSlider");
    auto *blueSlider = dialog.findChild<QSlider *>("blueSlider");
    auto *specularButton = dialog.findChild<QRadioButton *>("specularRadioButton");
    auto *intensitySlider = dialog.findChild<QSlider *>("intensitySlider");
    auto *distanceSpinBox = dialog.findChild<QDoubleSpinBox *>("distanceSpinBox");
    auto *attenuationGroupBox = dialog.findChild<QGroupBox *>("attenuationGroupBox");
    auto *attenuationStartSpinBox =
        dialog.findChild<QDoubleSpinBox *>("attenuationStartSpinBox");
    auto *attenuationEndSpinBox = dialog.findChild<QDoubleSpinBox *>("attenuationEndSpinBox");

    QVERIFY(redSlider);
    QVERIFY(greenSlider);
    QVERIFY(blueSlider);
    QVERIFY(specularButton);
    QVERIFY(intensitySlider);
    QVERIFY(distanceSpinBox);
    QVERIFY(attenuationGroupBox);
    QVERIFY(attenuationStartSpinBox);
    QVERIFY(attenuationEndSpinBox);

    redSlider->setValue(90);
    greenSlider->setValue(80);
    blueSlider->setValue(70);
    specularButton->setChecked(true);
    redSlider->setValue(10);
    greenSlider->setValue(20);
    blueSlider->setValue(30);
    intensitySlider->setValue(35);
    distanceSpinBox->setValue(456.75);
    attenuationGroupBox->setChecked(true);
    attenuationStartSpinBox->setValue(100.0);
    attenuationEndSpinBox->setValue(900.0);

    // A viewport drag can update orientation while this modeless editing state is live.
    viewport.setSceneLightOrientation(Quaternion(-0.30f, 0.20f, 0.10f, 0.90f));
    const W3DViewport::SceneLightState edited = viewport.sceneLightState();
    QVERIFY(edited.orientationExplicit);
    QVERIFY(edited.distanceExplicit);
    QVERIFY(!fuzzyEqual(edited.diffuse, initial.diffuse));
    QVERIFY(!fuzzyEqual(edited.specular, initial.specular));

    dialog.reject();

    compareState(viewport.sceneLightState(), initial);
}

void SceneLightTests::channelSelectionUpdatesOnlySelectedChannel()
{
    W3DViewport viewport;
    W3DViewport::SceneLightState initial;
    initial.diffuse = Vector3(0.25f, 0.50f, 0.75f);
    initial.specular = Vector3(0.75f, 0.50f, 0.25f);
    viewport.setSceneLightState(initial);

    SceneLightDialog dialog(viewport);
    auto *redSlider = dialog.findChild<QSlider *>("redSlider");
    auto *greenSlider = dialog.findChild<QSlider *>("greenSlider");
    auto *diffuseButton = dialog.findChild<QRadioButton *>("diffuseRadioButton");
    auto *specularButton = dialog.findChild<QRadioButton *>("specularRadioButton");
    QVERIFY(redSlider);
    QVERIFY(greenSlider);
    QVERIFY(diffuseButton);
    QVERIFY(specularButton);
    QVERIFY(diffuseButton->isChecked());

    redSlider->setValue(40);
    QVERIFY(fuzzyEqual(viewport.sceneLightDiffuse(), Vector3(0.40f, 0.50f, 0.75f)));
    QVERIFY(fuzzyEqual(viewport.sceneLightSpecular(), initial.specular));

    const Vector3 diffuseAfterEdit = viewport.sceneLightDiffuse();
    specularButton->setChecked(true);
    greenSlider->setValue(60);
    QVERIFY(fuzzyEqual(viewport.sceneLightDiffuse(), diffuseAfterEdit));
    QVERIFY(fuzzyEqual(viewport.sceneLightSpecular(), Vector3(0.75f, 0.60f, 0.25f)));
}

void SceneLightTests::grayscaleHonorsChannelSelection()
{
    W3DViewport viewport;
    W3DViewport::SceneLightState initial;
    initial.diffuse = Vector3(0.25f, 0.50f, 0.75f);
    initial.specular = Vector3(0.75f, 0.50f, 0.25f);
    viewport.setSceneLightState(initial);

    SceneLightDialog dialog(viewport);
    auto *redSlider = dialog.findChild<QSlider *>("redSlider");
    auto *grayscaleCheckBox = dialog.findChild<QCheckBox *>("grayscaleCheckBox");
    auto *specularButton = dialog.findChild<QRadioButton *>("specularRadioButton");
    auto *bothButton = dialog.findChild<QRadioButton *>("bothRadioButton");
    QVERIFY(redSlider);
    QVERIFY(grayscaleCheckBox);
    QVERIFY(specularButton);
    QVERIFY(bothButton);
    QVERIFY(!grayscaleCheckBox->isChecked());

    grayscaleCheckBox->setChecked(true);
    QVERIFY(fuzzyEqual(viewport.sceneLightDiffuse(), Vector3(0.25f, 0.25f, 0.25f)));
    QVERIFY(fuzzyEqual(viewport.sceneLightSpecular(), initial.specular));

    const Vector3 diffuseGrayscale = viewport.sceneLightDiffuse();
    grayscaleCheckBox->setChecked(false);
    specularButton->setChecked(true);
    grayscaleCheckBox->setChecked(true);
    QVERIFY(fuzzyEqual(viewport.sceneLightDiffuse(), diffuseGrayscale));
    QVERIFY(fuzzyEqual(viewport.sceneLightSpecular(), Vector3(0.75f, 0.75f, 0.75f)));

    grayscaleCheckBox->setChecked(false);
    bothButton->setChecked(true);
    redSlider->setValue(40);
    grayscaleCheckBox->setChecked(true);
    QVERIFY(fuzzyEqual(viewport.sceneLightDiffuse(), Vector3(0.40f, 0.40f, 0.40f)));
    QVERIFY(fuzzyEqual(viewport.sceneLightSpecular(), Vector3(0.40f, 0.40f, 0.40f)));
}

void SceneLightTests::designerFormExposesRequiredControlsAndRanges()
{
    W3DViewport viewport;
    SceneLightDialog dialog(viewport);

    const char *requiredObjects[] = {
        "channelGroupBox",
        "diffuseRadioButton",
        "specularRadioButton",
        "bothRadioButton",
        "redSlider",
        "greenSlider",
        "blueSlider",
        "grayscaleCheckBox",
        "intensitySlider",
        "distanceSpinBox",
        "attenuationGroupBox",
        "attenuationStartLabel",
        "attenuationStartSpinBox",
        "attenuationEndLabel",
        "attenuationEndSpinBox",
        "repositionHintLabel",
        "buttonBox",
    };

    for (const char *objectName : requiredObjects) {
        QVERIFY2(dialog.findChild<QObject *>(objectName), objectName);
    }

    const char *colorSliderNames[] = {"redSlider", "greenSlider", "blueSlider"};
    for (const char *objectName : colorSliderNames) {
        auto *slider = dialog.findChild<QSlider *>(objectName);
        QVERIFY(slider);
        QCOMPARE(slider->minimum(), 0);
        QCOMPARE(slider->maximum(), 100);
    }

    auto *intensitySlider = dialog.findChild<QSlider *>("intensitySlider");
    QVERIFY(intensitySlider);
    QCOMPARE(intensitySlider->minimum(), 0);
    QCOMPARE(intensitySlider->maximum(), 100);

    const char *spinBoxNames[] = {
        "distanceSpinBox",
        "attenuationStartSpinBox",
        "attenuationEndSpinBox",
    };
    for (const char *objectName : spinBoxNames) {
        auto *spinBox = dialog.findChild<QDoubleSpinBox *>(objectName);
        QVERIFY(spinBox);
        QCOMPARE(spinBox->minimum(), 0.0);
        QCOMPARE(spinBox->maximum(), 1000000.0);
        QCOMPARE(spinBox->decimals(), 2);
        QVERIFY(std::fabs(spinBox->singleStep() - 0.01) <= 0.000001);
    }

    auto *attenuationGroupBox = dialog.findChild<QGroupBox *>("attenuationGroupBox");
    QVERIFY(attenuationGroupBox);
    QVERIFY(attenuationGroupBox->isCheckable());

    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(buttonBox);
    QVERIFY(buttonBox->standardButtons().testFlag(QDialogButtonBox::Ok));
    QVERIFY(buttonBox->standardButtons().testFlag(QDialogButtonBox::Cancel));
}

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }

    QApplication application(argc, argv);
    SceneLightTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "SceneLightTests.moc"
