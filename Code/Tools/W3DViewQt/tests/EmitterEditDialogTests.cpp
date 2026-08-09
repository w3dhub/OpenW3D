#include "EmitterEditDialog.h"

#include "chunkio.h"
#include "part_ldr.h"
#include "ramfile.h"
#include "rawfile.h"
#include "shader.h"
#include "v3_rnd.h"
#include "vector2.h"
#include "vector3.h"
#include "w3d_file.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QtTest/QTest>

#include <cmath>
#include <array>
#include <cstdint>
#include <memory>

namespace {
constexpr float kFloatTolerance = 0.0001f;
constexpr std::uint32_t kUnknownLineFlag = 0x00010000u;

bool fuzzyEqual(float actual, float expected)
{
    return std::fabs(actual - expected) <= kFloatTolerance;
}

bool fuzzyEqual(const Vector2 &actual, const Vector2 &expected)
{
    return fuzzyEqual(actual.X, expected.X) && fuzzyEqual(actual.Y, expected.Y);
}

bool fuzzyEqual(const Vector3 &actual, const Vector3 &expected)
{
    return fuzzyEqual(actual.X, expected.X) && fuzzyEqual(actual.Y, expected.Y)
        && fuzzyEqual(actual.Z, expected.Z);
}

void compareValue(float actual, float expected)
{
    QVERIFY(fuzzyEqual(actual, expected));
}

void compareValue(const Vector3 &actual, const Vector3 &expected)
{
    QVERIFY(fuzzyEqual(actual, expected));
}

template<typename T>
struct OwnedProperty {
    ParticlePropertyStruct<T> value{};

    ~OwnedProperty()
    {
        delete[] value.KeyTimes;
        delete[] value.Values;
    }

    OwnedProperty(const OwnedProperty &) = delete;
    OwnedProperty &operator=(const OwnedProperty &) = delete;
    OwnedProperty() = default;
};

template<typename T>
using PropertyGetter = void (ParticleEmitterDefClass::*)(ParticlePropertyStruct<T> &) const;

template<typename T>
void compareProperty(const ParticleEmitterDefClass &actual,
                     const ParticleEmitterDefClass &expected,
                     PropertyGetter<T> getter)
{
    OwnedProperty<T> actualProperty;
    OwnedProperty<T> expectedProperty;
    (actual.*getter)(actualProperty.value);
    (expected.*getter)(expectedProperty.value);

    compareValue(actualProperty.value.Start, expectedProperty.value.Start);
    compareValue(actualProperty.value.Rand, expectedProperty.value.Rand);
    QCOMPARE(actualProperty.value.NumKeyFrames, expectedProperty.value.NumKeyFrames);
    for (unsigned int index = 0; index < actualProperty.value.NumKeyFrames; ++index) {
        QVERIFY(fuzzyEqual(actualProperty.value.KeyTimes[index], expectedProperty.value.KeyTimes[index]));
        compareValue(actualProperty.value.Values[index], expectedProperty.value.Values[index]);
    }
}

template<typename T>
void comparePropertyWithScaledTimes(const ParticleEmitterDefClass &actual,
                                    const ParticleEmitterDefClass &original,
                                    PropertyGetter<T> getter,
                                    float scale)
{
    OwnedProperty<T> actualProperty;
    OwnedProperty<T> originalProperty;
    (actual.*getter)(actualProperty.value);
    (original.*getter)(originalProperty.value);

    compareValue(actualProperty.value.Start, originalProperty.value.Start);
    compareValue(actualProperty.value.Rand, originalProperty.value.Rand);
    QCOMPARE(actualProperty.value.NumKeyFrames, originalProperty.value.NumKeyFrames);
    for (unsigned int index = 0; index < actualProperty.value.NumKeyFrames; ++index) {
        QVERIFY(fuzzyEqual(actualProperty.value.KeyTimes[index],
                           originalProperty.value.KeyTimes[index] * scale));
        compareValue(actualProperty.value.Values[index], originalProperty.value.Values[index]);
    }
}

void compareRandomizer(Vector3Randomizer *actualRaw, Vector3Randomizer *expectedRaw)
{
    std::unique_ptr<Vector3Randomizer> actual(actualRaw);
    std::unique_ptr<Vector3Randomizer> expected(expectedRaw);
    QCOMPARE(actual != nullptr, expected != nullptr);
    if (!actual || !expected) {
        return;
    }

    QCOMPARE(actual->Class_ID(), expected->Class_ID());
    switch (actual->Class_ID()) {
        case Vector3Randomizer::CLASSID_SOLIDBOX:
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidBoxRandomizer *>(actual.get())->Get_Extents(),
                               static_cast<Vector3SolidBoxRandomizer *>(expected.get())->Get_Extents()));
            break;
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidSphereRandomizer *>(actual.get())->Get_Radius(),
                               static_cast<Vector3SolidSphereRandomizer *>(expected.get())->Get_Radius()));
            break;
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            QVERIFY(fuzzyEqual(static_cast<Vector3HollowSphereRandomizer *>(actual.get())->Get_Radius(),
                               static_cast<Vector3HollowSphereRandomizer *>(expected.get())->Get_Radius()));
            break;
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidCylinderRandomizer *>(actual.get())->Get_Height(),
                               static_cast<Vector3SolidCylinderRandomizer *>(expected.get())->Get_Height()));
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidCylinderRandomizer *>(actual.get())->Get_Radius(),
                               static_cast<Vector3SolidCylinderRandomizer *>(expected.get())->Get_Radius()));
            break;
        default:
            QFAIL("Unexpected randomizer class");
    }
}

void compareDefinitions(const ParticleEmitterDefClass &actual,
                        const ParticleEmitterDefClass &expected)
{
    QCOMPARE(QByteArray(actual.Get_Name()), QByteArray(expected.Get_Name()));
    QCOMPARE(QByteArray(actual.Get_Texture_Filename()), QByteArray(expected.Get_Texture_Filename()));
    QVERIFY(fuzzyEqual(actual.Get_Lifetime(), expected.Get_Lifetime()));
    QVERIFY(fuzzyEqual(actual.Get_Emission_Rate(), expected.Get_Emission_Rate()));
    QVERIFY(fuzzyEqual(actual.Get_Max_Emissions(), expected.Get_Max_Emissions()));
    QVERIFY(fuzzyEqual(actual.Get_Fade_Time(), expected.Get_Fade_Time()));
    QVERIFY(fuzzyEqual(actual.Get_Gravity(), expected.Get_Gravity()));
    QVERIFY(fuzzyEqual(actual.Get_Elasticity(), expected.Get_Elasticity()));
    QVERIFY(fuzzyEqual(actual.Get_Velocity(), expected.Get_Velocity()));
    QVERIFY(fuzzyEqual(actual.Get_Acceleration(), expected.Get_Acceleration()));
    QCOMPARE(actual.Get_Burst_Size(), expected.Get_Burst_Size());
    QVERIFY(fuzzyEqual(actual.Get_Outward_Vel(), expected.Get_Outward_Vel()));
    QVERIFY(fuzzyEqual(actual.Get_Vel_Inherit(), expected.Get_Vel_Inherit()));
    QCOMPARE(actual.Get_Render_Mode(), expected.Get_Render_Mode());
    QCOMPARE(actual.Get_Frame_Mode(), expected.Get_Frame_Mode());

    ShaderClass actualShader;
    ShaderClass expectedShader;
    actual.Get_Shader(actualShader);
    expected.Get_Shader(expectedShader);
    QCOMPARE(actualShader.Get_Bits(), expectedShader.Get_Bits());

    compareRandomizer(actual.Get_Creation_Volume(), expected.Get_Creation_Volume());
    compareRandomizer(actual.Get_Velocity_Random(), expected.Get_Velocity_Random());
    compareProperty(actual, expected, &ParticleEmitterDefClass::Get_Color_Keyframes);
    compareProperty(actual, expected, &ParticleEmitterDefClass::Get_Opacity_Keyframes);
    compareProperty(actual, expected, &ParticleEmitterDefClass::Get_Size_Keyframes);
    compareProperty(actual, expected, &ParticleEmitterDefClass::Get_Rotation_Keyframes);
    compareProperty(actual, expected, &ParticleEmitterDefClass::Get_Frame_Keyframes);
    compareProperty(actual, expected, &ParticleEmitterDefClass::Get_Blur_Time_Keyframes);
    QVERIFY(fuzzyEqual(actual.Get_Initial_Orientation_Random(),
                       expected.Get_Initial_Orientation_Random()));

    QCOMPARE(QByteArray(actual.Get_User_String()), QByteArray(expected.Get_User_String()));
    QCOMPARE(actual.Get_User_Type(), expected.Get_User_Type());

    const W3dEmitterLinePropertiesStruct *actualLine = actual.Get_Line_Properties();
    const W3dEmitterLinePropertiesStruct *expectedLine = expected.Get_Line_Properties();
    QCOMPARE(actualLine->Flags, expectedLine->Flags);
    QCOMPARE(actualLine->SubdivisionLevel, expectedLine->SubdivisionLevel);
    QVERIFY(fuzzyEqual(actualLine->NoiseAmplitude, expectedLine->NoiseAmplitude));
    QVERIFY(fuzzyEqual(actualLine->MergeAbortFactor, expectedLine->MergeAbortFactor));
    QVERIFY(fuzzyEqual(actualLine->TextureTileFactor, expectedLine->TextureTileFactor));
    QVERIFY(fuzzyEqual(actual.Get_UV_Offset_Rate(), expected.Get_UV_Offset_Rate()));
    for (int index = 0; index < 9; ++index) {
        QCOMPARE(actualLine->Reserved[index], expectedLine->Reserved[index]);
    }
}

void setFixtureKeyframes(ParticleEmitterDefClass &definition)
{
    float times[] = {1.25f, 4.5f};

    Vector3 colorValues[] = {Vector3(0.2f, 0.3f, 0.4f), Vector3(0.8f, 0.7f, 0.6f)};
    ParticlePropertyStruct<Vector3> color{Vector3(0.1f, 0.2f, 0.3f),
                                          Vector3(0.01f, 0.02f, 0.03f),
                                          2,
                                          times,
                                          colorValues};
    definition.Set_Color_Keyframes(color);

    float opacityValues[] = {0.75f, 0.25f};
    ParticlePropertyStruct<float> opacity{0.9f, 0.08f, 2, times, opacityValues};
    definition.Set_Opacity_Keyframes(opacity);

    float sizeValues[] = {1.5f, 3.5f};
    ParticlePropertyStruct<float> size{0.5f, 0.2f, 2, times, sizeValues};
    definition.Set_Size_Keyframes(size);

    float rotationValues[] = {0.5f, -0.25f};
    ParticlePropertyStruct<float> rotation{0.1f, 0.05f, 2, times, rotationValues};
    definition.Set_Rotation_Keyframes(rotation, 0.33f);

    float frameValues[] = {2.0f, 7.0f};
    ParticlePropertyStruct<float> frame{1.0f, 0.5f, 2, times, frameValues};
    definition.Set_Frame_Keyframes(frame);

    float blurValues[] = {0.04f, 0.15f};
    ParticlePropertyStruct<float> blur{0.02f, 0.01f, 2, times, blurValues};
    definition.Set_Blur_Time_Keyframes(blur);
}

ParticleEmitterDefClass makeFixtureDefinition()
{
    ParticleEmitterDefClass definition;
    definition.Set_Name("EmitterFixture");
    definition.Set_Texture_Filename("fixture.dds");
    definition.Set_User_String("fixture user data");
    definition.Set_User_Type(77);
    definition.Set_Lifetime(10.0f);
    definition.Set_Emission_Rate(12.5f);
    definition.Set_Max_Emissions(321.0f);
    definition.Set_Fade_Time(0.75f);
    definition.Set_Gravity(-9.25f);
    definition.Set_Elasticity(0.42f);
    definition.Set_Velocity(Vector3(1.25f, -2.5f, 3.75f));
    definition.Set_Acceleration(Vector3(-0.5f, 0.25f, 0.75f));
    definition.Set_Burst_Size(7);
    definition.Set_Outward_Vel(4.25f);
    definition.Set_Vel_Inherit(0.35f);
    definition.Set_Render_Mode(W3D_EMITTER_RENDER_MODE_LINE);
    definition.Set_Frame_Mode(37);

    ShaderClass customShader = ShaderClass::_PresetAlphaSpriteShader;
    // PASS_ALWAYS survives the W3dShaderStruct conversion but intentionally
    // differs from every sprite preset offered by the dialog.
    customShader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    definition.Set_Shader(customShader);

    definition.Set_Creation_Volume(new Vector3SolidBoxRandomizer(Vector3(1.1f, 2.2f, 3.3f)));
    definition.Set_Velocity_Random(new Vector3SolidCylinderRandomizer(4.4f, 5.5f));
    setFixtureKeyframes(definition);

    auto *line = const_cast<W3dEmitterLinePropertiesStruct *>(definition.Get_Line_Properties());
    line->Flags = kUnknownLineFlag | W3D_ELINE_MERGE_INTERSECTIONS | W3D_ELINE_FREEZE_RANDOM
                  | W3D_ELINE_DISABLE_SORTING | W3D_ELINE_END_CAPS
                  | (W3D_ELINE_TILED_TEXTURE_MAP << W3D_ELINE_TEXTURE_MAP_MODE_OFFSET);
    line->SubdivisionLevel = 5;
    line->NoiseAmplitude = 1.75f;
    line->MergeAbortFactor = 0.45f;
    line->TextureTileFactor = 2.25f;
    line->UPerSec = -0.125f;
    line->VPerSec = 0.875f;
    for (int index = 0; index < 9; ++index) {
        line->Reserved[index] = 0xA0000000u + static_cast<std::uint32_t>(index);
    }

    return definition;
}

std::unique_ptr<ParticleEmitterDefClass> acceptDialog(EmitterEditDialog &dialog)
{
    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    if (!buttonBox) {
        return nullptr;
    }
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    if (!okButton) {
        return nullptr;
    }
    okButton->click();
    QApplication::processEvents();
    if (dialog.result() != QDialog::Accepted) {
        return nullptr;
    }
    return std::unique_ptr<ParticleEmitterDefClass>(dialog.definition());
}

void verifyRandomizer(Vector3Randomizer *raw,
                      int classId,
                      float first,
                      float second,
                      float third)
{
    std::unique_ptr<Vector3Randomizer> randomizer(raw);
    QVERIFY(randomizer);
    QCOMPARE(static_cast<int>(randomizer->Class_ID()), classId);
    switch (randomizer->Class_ID()) {
        case Vector3Randomizer::CLASSID_SOLIDBOX: {
            const Vector3 extents = static_cast<Vector3SolidBoxRandomizer *>(randomizer.get())->Get_Extents();
            QVERIFY(fuzzyEqual(extents, Vector3(first, second, third)));
            break;
        }
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidSphereRandomizer *>(randomizer.get())->Get_Radius(), first));
            break;
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            QVERIFY(fuzzyEqual(static_cast<Vector3HollowSphereRandomizer *>(randomizer.get())->Get_Radius(), first));
            break;
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidCylinderRandomizer *>(randomizer.get())->Get_Height(), first));
            QVERIFY(fuzzyEqual(static_cast<Vector3SolidCylinderRandomizer *>(randomizer.get())->Get_Radius(), second));
            break;
        default:
            QFAIL("Unexpected randomizer class");
    }
}
} // namespace

class EmitterEditDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void noOpRoundTripPreservesAllObservableData();
    void unrelatedEditPreservesCustomShader();
    void componentEditDoesNotChangeSiblingFields();
    void randomizerEditsRoundTrip_data();
    void randomizerEditsRoundTrip();
    void lifetimeChangeRescalesEveryKeyframeChannel();
    void lineFlagEditPreservesUnknownBitsAndReservedData();
    void userStringEditPreservesWhitespace();
    void applyWithoutCloseAdvancesRegisteredName();
    void cancelAfterApplyPreservesLastAppliedDefinition();
    void okDoesNotRepeatCleanApply();
    void serializerRejectsTruncatedBlurChunk();
};

void EmitterEditDialogTests::noOpRoundTripPreservesAllObservableData()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, original);
}

void EmitterEditDialogTests::unrelatedEditPreservesCustomShader()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *shaderCombo = dialog.findChild<QComboBox *>("shaderCombo");
    auto *gravitySpin = dialog.findChild<QDoubleSpinBox *>("gravitySpin");
    QVERIFY(shaderCombo);
    QVERIFY(gravitySpin);
    QVERIFY(shaderCombo->currentText().startsWith("Custom"));
    gravitySpin->setValue(-3.5);

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    ShaderClass originalShader;
    ShaderClass resultShader;
    original.Get_Shader(originalShader);
    result->Get_Shader(resultShader);
    QCOMPARE(resultShader.Get_Bits(), originalShader.Get_Bits());
    QVERIFY(fuzzyEqual(result->Get_Gravity(), -3.5f));
}

void EmitterEditDialogTests::componentEditDoesNotChangeSiblingFields()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *velocityXSpin = dialog.findChild<QDoubleSpinBox *>("velocityXSpin");
    auto *colorStartRSpin = dialog.findChild<QDoubleSpinBox *>("colorStartRSpin");
    QVERIFY(velocityXSpin);
    QVERIFY(colorStartRSpin);
    velocityXSpin->setValue(9.5);
    colorStartRSpin->setValue(0.95);

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    const Vector3 velocity = result->Get_Velocity();
    const Vector3 originalVelocity = original.Get_Velocity();
    QVERIFY(fuzzyEqual(velocity.X, 9.5f));
    QVERIFY(fuzzyEqual(velocity.Y, originalVelocity.Y));
    QVERIFY(fuzzyEqual(velocity.Z, originalVelocity.Z));

    OwnedProperty<Vector3> colors;
    OwnedProperty<Vector3> originalColors;
    result->Get_Color_Keyframes(colors.value);
    original.Get_Color_Keyframes(originalColors.value);
    QVERIFY(fuzzyEqual(colors.value.Start.X, 0.95f));
    QVERIFY(fuzzyEqual(colors.value.Start.Y, originalColors.value.Start.Y));
    QVERIFY(fuzzyEqual(colors.value.Start.Z, originalColors.value.Start.Z));
    QVERIFY(fuzzyEqual(colors.value.Rand, originalColors.value.Rand));
}

void EmitterEditDialogTests::randomizerEditsRoundTrip_data()
{
    QTest::addColumn<int>("classId");
    QTest::addColumn<float>("first");
    QTest::addColumn<float>("second");
    QTest::addColumn<float>("third");

    QTest::newRow("solid-box") << static_cast<int>(Vector3Randomizer::CLASSID_SOLIDBOX)
                                << 6.25f << 7.5f << 8.75f;
    QTest::newRow("solid-sphere") << static_cast<int>(Vector3Randomizer::CLASSID_SOLIDSPHERE)
                                   << 9.25f << 0.0f << 0.0f;
    QTest::newRow("hollow-sphere") << static_cast<int>(Vector3Randomizer::CLASSID_HOLLOWSPHERE)
                                    << 10.5f << 0.0f << 0.0f;
    QTest::newRow("solid-cylinder") << static_cast<int>(Vector3Randomizer::CLASSID_SOLIDCYLINDER)
                                     << 11.75f << 12.5f << 0.0f;
}

void EmitterEditDialogTests::randomizerEditsRoundTrip()
{
    QFETCH(int, classId);
    QFETCH(float, first);
    QFETCH(float, second);
    QFETCH(float, third);

    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto setRandomizerControls = [&dialog, classId, first, second, third](const char *comboName,
                                                                          const char *firstName,
                                                                          const char *secondName,
                                                                          const char *thirdName) {
        auto *combo = dialog.findChild<QComboBox *>(comboName);
        auto *firstSpin = dialog.findChild<QDoubleSpinBox *>(firstName);
        auto *secondSpin = dialog.findChild<QDoubleSpinBox *>(secondName);
        auto *thirdSpin = dialog.findChild<QDoubleSpinBox *>(thirdName);
        if (!combo || !firstSpin || !secondSpin || !thirdSpin) {
            return false;
        }
        const int index = combo->findData(classId);
        if (index < 0) {
            return false;
        }
        combo->setCurrentIndex(index);
        firstSpin->setValue(first);
        secondSpin->setValue(second);
        thirdSpin->setValue(third);
        return true;
    };

    QVERIFY(setRandomizerControls("creationTypeCombo",
                                  "creationValue1Spin",
                                  "creationValue2Spin",
                                  "creationValue3Spin"));
    QVERIFY(setRandomizerControls("velocityRandomTypeCombo",
                                  "velocityRandomValue1Spin",
                                  "velocityRandomValue2Spin",
                                  "velocityRandomValue3Spin"));

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    verifyRandomizer(result->Get_Creation_Volume(), classId, first, second, third);
    verifyRandomizer(result->Get_Velocity_Random(), classId, first, second, third);
}

void EmitterEditDialogTests::lifetimeChangeRescalesEveryKeyframeChannel()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *useLifetimeCheck = dialog.findChild<QCheckBox *>("useLifetimeCheck");
    auto *lifetimeSpin = dialog.findChild<QDoubleSpinBox *>("lifetimeSpin");
    QVERIFY(useLifetimeCheck);
    QVERIFY(lifetimeSpin);
    QVERIFY(useLifetimeCheck->isChecked());
    lifetimeSpin->setValue(25.0);

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    QVERIFY(fuzzyEqual(result->Get_Lifetime(), 25.0f));
    constexpr float scale = 2.5f;
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Color_Keyframes, scale);
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Opacity_Keyframes, scale);
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Size_Keyframes, scale);
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Rotation_Keyframes, scale);
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Frame_Keyframes, scale);
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Blur_Time_Keyframes, scale);
}

void EmitterEditDialogTests::lineFlagEditPreservesUnknownBitsAndReservedData()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *mergeCheck = dialog.findChild<QCheckBox *>("lineMergeCheck");
    auto *mappingCombo = dialog.findChild<QComboBox *>("lineMappingCombo");
    QVERIFY(mergeCheck);
    QVERIFY(mappingCombo);
    QVERIFY(mergeCheck->isChecked());
    mergeCheck->setChecked(false);
    const int mappingIndex = mappingCombo->findData(W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP);
    QVERIFY(mappingIndex >= 0);
    mappingCombo->setCurrentIndex(mappingIndex);

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    const W3dEmitterLinePropertiesStruct *line = result->Get_Line_Properties();
    const W3dEmitterLinePropertiesStruct *originalLine = original.Get_Line_Properties();
    QVERIFY((line->Flags & kUnknownLineFlag) != 0);
    QVERIFY((line->Flags & W3D_ELINE_MERGE_INTERSECTIONS) == 0);
    QVERIFY((line->Flags & W3D_ELINE_FREEZE_RANDOM) != 0);
    QVERIFY((line->Flags & W3D_ELINE_DISABLE_SORTING) != 0);
    QVERIFY((line->Flags & W3D_ELINE_END_CAPS) != 0);
    QCOMPARE(result->Get_Line_Texture_Mapping_Mode(), W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP);
    for (int index = 0; index < 9; ++index) {
        QCOMPARE(line->Reserved[index], originalLine->Reserved[index]);
    }
}

void EmitterEditDialogTests::userStringEditPreservesWhitespace()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *userStringEdit = dialog.findChild<QPlainTextEdit *>("userStringEdit");
    QVERIFY(userStringEdit);
    const QString exactText = QStringLiteral("  leading spaces\n\tmiddle\t \ntrailing spaces  ");
    userStringEdit->setPlainText(exactText);

    const std::unique_ptr<ParticleEmitterDefClass> result = acceptDialog(dialog);
    QVERIFY(result);
    QCOMPARE(QString::fromLatin1(result->Get_User_String()), exactText);
}

void EmitterEditDialogTests::applyWithoutCloseAdvancesRegisteredName()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    auto *nameEdit = dialog.findChild<QLineEdit *>("nameEdit");
    QVERIFY(buttonBox);
    QVERIFY(nameEdit);
    QPushButton *applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QVERIFY(applyButton);

    QStringList registeredNames;
    QStringList appliedNames;
    dialog.setApplyHandler(
        [&registeredNames, &appliedNames](const ParticleEmitterDefClass &definition,
                                         const QString &registeredName) {
            registeredNames.push_back(registeredName);
            appliedNames.push_back(QString::fromLatin1(definition.Get_Name()));
            return true;
        },
        dialog.originalName());

    dialog.show();
    nameEdit->selectAll();
    QTest::keyClicks(nameEdit, "FirstAppliedName");
    applyButton->click();
    QApplication::processEvents();
    QVERIFY(dialog.isVisible());
    QCOMPARE(registeredNames, QStringList({"EmitterFixture"}));
    QCOMPARE(appliedNames, QStringList({"FirstAppliedName"}));

    nameEdit->selectAll();
    QTest::keyClicks(nameEdit, "SecondAppliedName");
    applyButton->click();
    QApplication::processEvents();
    QVERIFY(dialog.isVisible());
    QCOMPARE(registeredNames, QStringList({"EmitterFixture", "FirstAppliedName"}));
    QCOMPARE(appliedNames, QStringList({"FirstAppliedName", "SecondAppliedName"}));
}

void EmitterEditDialogTests::cancelAfterApplyPreservesLastAppliedDefinition()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    auto *nameEdit = dialog.findChild<QLineEdit *>("nameEdit");
    auto *gravitySpin = dialog.findChild<QDoubleSpinBox *>("gravitySpin");
    QVERIFY(buttonBox);
    QVERIFY(nameEdit);
    QVERIFY(gravitySpin);
    QPushButton *applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    QVERIFY(applyButton);
    QVERIFY(cancelButton);

    QString appliedName;
    float appliedGravity = 0.0f;
    int applyCount = 0;
    dialog.setApplyHandler(
        [&appliedName, &appliedGravity, &applyCount](const ParticleEmitterDefClass &definition,
                                                    const QString &) {
            ++applyCount;
            appliedName = QString::fromLatin1(definition.Get_Name());
            appliedGravity = definition.Get_Gravity();
            return true;
        },
        dialog.originalName());

    dialog.show();
    QApplication::processEvents();
    nameEdit->selectAll();
    QTest::keyClicks(nameEdit, "KeptAppliedName");
    gravitySpin->setValue(-4.5);
    applyButton->click();
    QApplication::processEvents();
    QCOMPARE(applyCount, 1);

    nameEdit->selectAll();
    QTest::keyClicks(nameEdit, "CancelledName");
    gravitySpin->setValue(-9.0);
    cancelButton->click();
    QApplication::processEvents();
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
    QCOMPARE(applyCount, 1);
    QCOMPARE(appliedName, QString("KeptAppliedName"));
    QVERIFY(fuzzyEqual(appliedGravity, -4.5f));

    const std::unique_ptr<ParticleEmitterDefClass> lastApplied(dialog.definition());
    QVERIFY(lastApplied);
    QCOMPARE(QString::fromLatin1(lastApplied->Get_Name()), QString("KeptAppliedName"));
    QVERIFY(fuzzyEqual(lastApplied->Get_Gravity(), -4.5f));
}

void EmitterEditDialogTests::okDoesNotRepeatCleanApply()
{
    const ParticleEmitterDefClass original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);

    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    auto *gravitySpin = dialog.findChild<QDoubleSpinBox *>("gravitySpin");
    QVERIFY(buttonBox);
    QVERIFY(gravitySpin);
    QPushButton *applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QVERIFY(applyButton);
    QVERIFY(okButton);

    int applyCount = 0;
    dialog.setApplyHandler(
        [&applyCount](const ParticleEmitterDefClass &, const QString &) {
            ++applyCount;
            return true;
        },
        dialog.originalName());

    gravitySpin->setValue(-7.25);
    applyButton->click();
    QApplication::processEvents();
    QCOMPARE(applyCount, 1);
    QVERIFY(!applyButton->isEnabled());

    okButton->click();
    QApplication::processEvents();
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Accepted));
    QCOMPARE(applyCount, 1);
}

void EmitterEditDialogTests::serializerRejectsTruncatedBlurChunk()
{
    const QString assetDirectory = qEnvironmentVariable("W3DVIEW_EXTERNAL_ASSET_DIR");
    if (assetDirectory.isEmpty()) {
        QSKIP("Set W3DVIEW_EXTERNAL_ASSET_DIR to run the real-emitter serializer regression");
    }

    const QString sourcePath = QDir(assetDirectory).filePath("e_flare02.w3d");
    QVERIFY2(QFileInfo::exists(sourcePath),
             qPrintable(QString("Missing integration asset: %1").arg(sourcePath)));

    ParticleEmitterDefClass definition;
    {
        const QByteArray nativePath = QDir::toNativeSeparators(sourcePath).toLocal8Bit();
        RawFileClass source(nativePath.constData());
        QVERIFY(source.Open(FileClass::READ));

        ChunkLoadClass load(&source);
        QVERIFY(load.Open_Chunk());
        QCOMPARE(load.Cur_Chunk_ID(), static_cast<uint32>(W3D_CHUNK_EMITTER));
        QCOMPARE(definition.Load_W3D(load), WW3D_ERROR_OK);
        QVERIFY(load.Close_Chunk());
        QCOMPARE(source.Tell(), source.Size());
        source.Close();
    }

    // e_flare02 normally serializes to 708 bytes. At 688 bytes the final blur
    // chunk header fits, but its mandatory header and start keyframe do not.
    std::array<char, 688> storage = {};
    RAMFileClass destination(storage.data(), static_cast<int>(storage.size()));
    QVERIFY(destination.Open(FileClass::WRITE));

    ChunkSaveClass save(&destination);
    QCOMPARE(definition.Save_W3D(save), WW3D_ERROR_SAVE_FAILED);
    QVERIFY(save.Has_Write_Error());
    QCOMPARE(save.Cur_Chunk_Depth(), 0);
    QCOMPARE(destination.Size(), static_cast<int>(storage.size()));
    destination.Close();

    // The failed write still leaves a balanced zero-length blur chunk. This
    // prevents structural validity from masking the serializer failure again.
    QVERIFY(destination.Open(FileClass::READ));
    ChunkLoadClass truncatedLoad(&destination);
    QVERIFY(truncatedLoad.Open_Chunk());
    QCOMPARE(truncatedLoad.Cur_Chunk_ID(), static_cast<uint32>(W3D_CHUNK_EMITTER));
    QCOMPARE(truncatedLoad.Cur_Chunk_Length() + sizeof(ChunkHeader), storage.size());

    uint32 lastChildId = 0;
    uint32 lastChildLength = 0;
    while (truncatedLoad.Open_Chunk()) {
        lastChildId = truncatedLoad.Cur_Chunk_ID();
        lastChildLength = truncatedLoad.Cur_Chunk_Length();
        QVERIFY(truncatedLoad.Close_Chunk());
    }
    QCOMPARE(lastChildId, static_cast<uint32>(W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES));
    QCOMPARE(lastChildLength, static_cast<uint32>(0));
    QVERIFY(truncatedLoad.Close_Chunk());
    QCOMPARE(destination.Tell(), destination.Size());
    destination.Close();
}

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }

    QApplication application(argc, argv);
    EmitterEditDialogTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "EmitterEditDialogTests.moc"
