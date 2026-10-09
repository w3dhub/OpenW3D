#include "EmitterEditDialog.h"
#include "EmitterKeyframeBar.h"

#include "part_ldr.h"
#include "shader.h"
#include "v3_rnd.h"
#include "vector2.h"
#include "vector3.h"
#include "w3d_file.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QTimer>
#include <QtTest/QTest>

#include <cmath>
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
void getScalarProperty(const ParticleEmitterDefClass &definition, const QString &channel, ParticlePropertyStruct<float> &property)
{
    if (channel == "size") definition.Get_Size_Keyframes(property);
    else if (channel == "rotation") definition.Get_Rotation_Keyframes(property);
    else if (channel == "frame") definition.Get_Frame_Keyframes(property);
    else definition.Get_Blur_Time_Keyframes(property);
}

void setScalarProperty(ParticleEmitterDefClass &definition, const QString &channel, ParticlePropertyStruct<float> &property)
{
    if (channel == "size") definition.Set_Size_Keyframes(property);
    else if (channel == "rotation") definition.Set_Rotation_Keyframes(property, definition.Get_Initial_Orientation_Random());
    else if (channel == "frame") definition.Set_Frame_Keyframes(property);
    else definition.Set_Blur_Time_Keyframes(property);
}
} // namespace

class EmitterEditDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void tabsMatchMfcAndRemainAccessible();
    void layoutFitsAtMinimumSize();
    void visualSelectionAndNumericToggleAreReadOnly();
    void timelineDragInsertDeletePreservesOtherChannels();
    void timelinePickerCancelAndAccept();
    void opacityTimelineEditingAndLifetimeRescale();
    void visualEditsCancelAfterApply();
    void numericColorLayoutFits();
    void scalarViewsAreReadOnlyAndFit_data();
    void scalarViewsAreReadOnlyAndFit();
    void scalarTimelineEdits_data();
    void scalarTimelineEdits();
    void scalarApplyRescaleAndCancel_data();
    void scalarApplyRescaleAndCancel();
    void scalarGraphRanges();
    void timelineHandlesDegenerateLifetime_data();
    void timelineHandlesDegenerateLifetime();
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
};

void EmitterEditDialogTests::tabsMatchMfcAndRemainAccessible()
{
    EmitterEditDialog dialog(makeFixtureDefinition());
    auto *tabs = dialog.findChild<QTabWidget *>("tabWidget");
    auto *renderMode = dialog.findChild<QComboBox *>("renderModeCombo");
    auto *lineOptions = dialog.findChild<QWidget *>("lineOptionsGroup");
    auto *lineParameters = dialog.findChild<QWidget *>("lineParametersGroup");
    auto *blurTime = dialog.findChild<QDoubleSpinBox *>("blurStartSpin");
    QVERIFY(tabs && renderMode && lineOptions && lineParameters && blurTime);

    const QStringList titles = {"General", "Emission", "Physics", "Color", "Size", "User",
                                "Line Properties", "Rotation", "Frame / UCoordinate", "Line Group"};
    QCOMPARE(tabs->count(), titles.size());
    for (int mode : {W3D_EMITTER_RENDER_MODE_TRI_PARTICLES, W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES,
                     W3D_EMITTER_RENDER_MODE_LINE, W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA,
                     W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM}) {
        renderMode->setCurrentIndex(renderMode->findData(mode));
        for (int index = 0; index < tabs->count(); ++index) {
            QCOMPARE(tabs->tabText(index), titles[index]);
            QVERIFY(tabs->isTabEnabled(index));
            tabs->setCurrentIndex(index);
            QCOMPARE(tabs->currentIndex(), index);
        }
        QCOMPARE(lineOptions->isEnabled(), mode == W3D_EMITTER_RENDER_MODE_LINE);
        QCOMPARE(lineParameters->isEnabled(), mode == W3D_EMITTER_RENDER_MODE_LINE);
        QVERIFY(blurTime->isEnabled());
    }
}

void EmitterEditDialogTests::layoutFitsAtMinimumSize()
{
    EmitterEditDialog dialog(makeFixtureDefinition());
    auto *tabs = dialog.findChild<QTabWidget *>("tabWidget");
    QVERIFY(tabs);
    dialog.resize(dialog.minimumSize());
    dialog.show();
    QApplication::processEvents();

    // All ten full tab labels and the Apply/OK/Cancel row must fit without
    // horizontal scrolling or the dialog growing beyond its requested size.
    QCOMPARE(dialog.size(), dialog.minimumSize());
    QVERIFY(!tabs->usesScrollButtons());
    QCOMPARE(tabs->elideMode(), Qt::ElideNone);
    for (int index = 0; index < tabs->count(); ++index) {
        QVERIFY2(tabs->tabBar()->rect().contains(tabs->tabBar()->tabRect(index)),
                 qPrintable(tabs->tabText(index)));
        tabs->setCurrentIndex(index);
        QApplication::processEvents();
        QWidget *page = tabs->widget(index);
        for (QWidget *control : page->findChildren<QWidget *>()) {
            if (control->isVisible() && (qobject_cast<QAbstractSpinBox *>(control)
                                        || qobject_cast<QComboBox *>(control)
                                        || qobject_cast<QPushButton *>(control))) {
                const QRect bounds(control->mapTo(page, QPoint(0, 0)), control->size());
                QVERIFY2(page->rect().contains(bounds), qPrintable(control->objectName()));
            }
        }
    }
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(buttons);
    QVERIFY(dialog.rect().contains(QRect(buttons->mapTo(&dialog, QPoint(0, 0)), buttons->size())));
}

void EmitterEditDialogTests::visualSelectionAndNumericToggleAreReadOnly()
{
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    auto *bar = dialog.findChild<EmitterKeyframeBar *>("colorGradientBar");
    auto *opacity = dialog.findChild<EmitterKeyframeBar *>("opacityGradientBar");
    auto *toggle = dialog.findChild<QCheckBox *>("colorNumericToggle");
    auto *stack = dialog.findChild<QStackedWidget *>("colorEditorStack");
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(bar && opacity && toggle && stack && buttons);
    QCOMPARE(bar->keys().size(), 3);
    QCOMPARE(opacity->keys().size(), 3);
    QVERIFY(opacity->opacityMode());
    QCOMPARE(stack->currentIndex(), 0);
    QCOMPARE(bar->selectedKey(), 0);
    QCOMPARE(bar->keys()[1].time, 1.25);
    bar->setSelectedKey(2);
    opacity->setSelectedKey(1);
    toggle->setChecked(true);
    QCOMPARE(stack->currentIndex(), 1);
    toggle->setChecked(false);
    QCOMPARE(stack->currentIndex(), 0);
    QVERIFY(!buttons->button(QDialogButtonBox::Apply)->isEnabled());
    auto result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, original);
}

void EmitterEditDialogTests::timelineDragInsertDeletePreservesOtherChannels()
{
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    dialog.findChild<QTabWidget *>("tabWidget")->setCurrentIndex(3);
    dialog.show();
    QApplication::processEvents();
    auto *bar = dialog.findChild<EmitterKeyframeBar *>("colorGradientBar");
    QVERIFY(bar && bar->isVisible());
    const auto positionAt = [bar](double time) {
        return QPoint(12 + qRound(time / bar->duration() * (bar->width() - 24)), bar->keyPosition(0).y());
    };
    const QPoint start = bar->keyPosition(1);
    const QPoint end = positionAt(3.0);
    QTest::mousePress(bar, Qt::LeftButton, Qt::NoModifier, start);
    QMouseEvent move(QEvent::MouseMove, end, bar->mapToGlobal(end), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(bar, &move);
    QTest::mouseRelease(bar, Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(bar->selectedKey(), 1);
    QVERIFY(std::abs(bar->keys()[1].time - 3.0) < 0.05);
    const double movedTime = bar->keys()[1].time;
    QCOMPARE(bar->keys()[1].red, 0.2);

    // Ctrl-click empty space inserts the interpolated value, not a default white key.
    QPoint insert = positionAt(6.0);
    insert.ry() -= 20;
    QTest::mouseClick(bar, Qt::LeftButton, Qt::ControlModifier, insert);
    QCOMPARE(bar->keys().size(), 4);
    QCOMPARE(bar->selectedKey(), 3);
    QCOMPARE(bar->keys()[3].red, 0.8);
    QTest::keyClick(bar, Qt::Key_Delete);
    QCOMPARE(bar->keys().size(), 3);

    // The fixed starting key cannot be deleted or dragged.
    bar->setSelectedKey(0);
    QTest::keyClick(bar, Qt::Key_Delete);
    QTest::mousePress(bar, Qt::LeftButton, Qt::NoModifier, bar->keyPosition(0));
    QTest::mouseRelease(bar, Qt::LeftButton, Qt::NoModifier, positionAt(2.0));
    QCOMPARE(bar->keys().size(), 3);
    QCOMPARE(bar->keys()[0].time, 0.0);

    auto expected = original;
    OwnedProperty<Vector3> colors;
    expected.Get_Color_Keyframes(colors.value);
    colors.value.KeyTimes[0] = static_cast<float>(movedTime);
    expected.Set_Color_Keyframes(colors.value);
    auto result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, expected);
}

void EmitterEditDialogTests::timelinePickerCancelAndAccept()
{
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    auto *bar = dialog.findChild<EmitterKeyframeBar *>("colorGradientBar");
    auto *edit = dialog.findChild<QPushButton *>("colorKeyEditButton");
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(bar && edit && buttons);
    bar->setSelectedKey(1);
    bool pickerOpened = false;
    QTimer::singleShot(0, &dialog, [&]() {
        if (auto *picker = dialog.findChild<QColorDialog *>()) {
            pickerOpened = true;
            picker->setCurrentColor(Qt::red);
            picker->reject();
        }
    });
    edit->click();
    QVERIFY(pickerOpened);
    QVERIFY(!buttons->button(QDialogButtonBox::Apply)->isEnabled());
    QCOMPARE(bar->keys()[1].red, 0.2);
    QTimer::singleShot(0, &dialog, [&]() {
        if (auto *picker = dialog.findChild<QColorDialog *>()) {
            picker->setCurrentColor(Qt::red);
            picker->accept();
        }
    });
    edit->click();
    QCOMPARE(bar->keys()[1].red, 1.0);
    QCOMPARE(bar->keys()[1].green, 0.0);
    QCOMPARE(bar->keys()[0].red, 0.1);
    auto expected = original;
    OwnedProperty<Vector3> colors;
    expected.Get_Color_Keyframes(colors.value);
    colors.value.Values[0] = Vector3(1.0f, 0.0f, 0.0f);
    expected.Set_Color_Keyframes(colors.value);
    auto result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, expected);
}

void EmitterEditDialogTests::opacityTimelineEditingAndLifetimeRescale()
{
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    auto *bar = dialog.findChild<EmitterKeyframeBar *>("opacityGradientBar");
    auto *value = dialog.findChild<QDoubleSpinBox *>("opacityKeyValueSpin");
    auto *time = dialog.findChild<QDoubleSpinBox *>("opacityKeyTimeSpin");
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(bar && value && time && buttons);
    bar->setSelectedKey(1);
    QCOMPARE(value->value(), 75.0);
    value->setValue(42.5);
    time->setValue(2.0);
    QCOMPARE(bar->keys()[1].red, 0.425);
    QCOMPARE(bar->keys()[1].time, 2.0);
    QCOMPARE(dialog.findChild<QDoubleSpinBox *>("opacityStartSpin")->value(), 0.9);
    // Numeric and visual views are two views of the same data.
    auto *table = dialog.findChild<QTableWidget *>("opacityKeysTable");
    qobject_cast<QDoubleSpinBox *>(table->cellWidget(0, 1))->setValue(0.625);
    QCOMPARE(value->value(), 62.5);
    dialog.findChild<QDoubleSpinBox *>("lifetimeSpin")->setValue(20.0);
    buttons->button(QDialogButtonBox::Apply)->click();
    QCOMPARE(bar->keys()[1].time, 4.0);
    QCOMPARE(bar->duration(), 20.0);
    QCOMPARE(time->value(), 4.0);
    QVERIFY(!buttons->button(QDialogButtonBox::Apply)->isEnabled());
    auto result = dialog.definition();
    std::unique_ptr<ParticleEmitterDefClass> owned(result);
    OwnedProperty<float> opacity;
    result->Get_Opacity_Keyframes(opacity.value);
    compareValue(opacity.value.Values[0], 0.625f);
    compareValue(opacity.value.Start, 0.9f);
    compareValue(opacity.value.Rand, 0.08f);
    comparePropertyWithScaledTimes(*result, original, &ParticleEmitterDefClass::Get_Color_Keyframes, 2.0f);
}

void EmitterEditDialogTests::visualEditsCancelAfterApply()
{
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    auto *bar = dialog.findChild<EmitterKeyframeBar *>("opacityGradientBar");
    auto *value = dialog.findChild<QDoubleSpinBox *>("opacityKeyValueSpin");
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(bar && value && buttons);
    int applies = 0;
    dialog.setApplyHandler([&](const ParticleEmitterDefClass &, const QString &) { ++applies; return true; }, dialog.originalName());
    bar->setSelectedKey(0);
    value->setValue(50.0);
    QCOMPARE(applies, 0);
    buttons->button(QDialogButtonBox::Apply)->click();
    QCOMPARE(applies, 1);
    value->setValue(10.0);
    buttons->button(QDialogButtonBox::Cancel)->click();
    QCOMPARE(applies, 1);
    auto expected = original;
    OwnedProperty<float> opacity;
    expected.Get_Opacity_Keyframes(opacity.value);
    opacity.value.Start = 0.5f;
    expected.Set_Opacity_Keyframes(opacity.value);
    const std::unique_ptr<ParticleEmitterDefClass> result(dialog.definition());
    compareDefinitions(*result, expected);
}

void EmitterEditDialogTests::numericColorLayoutFits()
{
    EmitterEditDialog dialog(makeFixtureDefinition());
    dialog.findChild<QTabWidget *>("tabWidget")->setCurrentIndex(3);
    dialog.findChild<QCheckBox *>("colorNumericToggle")->setChecked(true);
    dialog.resize(dialog.minimumSize());
    dialog.show();
    QApplication::processEvents();
    QCOMPARE(dialog.size(), dialog.minimumSize());
    auto *page = dialog.findChild<QWidget *>("colorNumericPage");
    QVERIFY(page && page->isVisible());
    for (QWidget *control : page->findChildren<QWidget *>()) {
        if (control->isVisible() && (qobject_cast<QPushButton *>(control) || qobject_cast<QTableWidget *>(control))) {
            QVERIFY2(page->rect().contains(QRect(control->mapTo(page, QPoint(0, 0)), control->size())),
                     qPrintable(control->objectName()));
        }
    }
}

void EmitterEditDialogTests::scalarViewsAreReadOnlyAndFit_data()
{
    QTest::addColumn<QString>("channel");
    QTest::addColumn<int>("tab");
    QTest::newRow("size") << QString("size") << 4;
    QTest::newRow("rotation") << QString("rotation") << 7;
    QTest::newRow("frame-u") << QString("frame") << 8;
    QTest::newRow("blur-time") << QString("blur") << 9;
}

void EmitterEditDialogTests::scalarViewsAreReadOnlyAndFit()
{
    QFETCH(QString, channel);
    QFETCH(int, tab);
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    auto *tabs = dialog.findChild<QTabWidget *>("tabWidget");
    auto *bar = dialog.findChild<EmitterKeyframeBar *>(channel + "GraphBar");
    auto *toggle = dialog.findChild<QCheckBox *>(channel + "NumericToggle");
    auto *stack = dialog.findChild<QStackedWidget *>(channel + "EditorStack");
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(tabs && bar && toggle && stack && buttons);
    tabs->setCurrentIndex(tab);
    dialog.resize(dialog.minimumSize());
    dialog.show();
    QApplication::processEvents();
    QCOMPARE(dialog.size(), dialog.minimumSize());
    QVERIFY(bar->scalarMode());
    QCOMPARE(bar->keys().size(), 3);
    QCOMPARE(stack->currentIndex(), 0);
    bar->setSelectedKey(2);
    for (bool numeric : {true, false}) {
        toggle->setChecked(numeric);
        QCOMPARE(stack->currentIndex(), numeric ? 1 : 0);
        QApplication::processEvents();
        QWidget *page = stack->currentWidget();
        for (QWidget *control : page->findChildren<QWidget *>()) {
            if (control->isVisible() && (qobject_cast<QAbstractSpinBox *>(control)
                                        || qobject_cast<QPushButton *>(control)
                                        || qobject_cast<QTableWidget *>(control)
                                        || qobject_cast<EmitterKeyframeBar *>(control))) {
                QVERIFY2(page->rect().contains(QRect(control->mapTo(page, QPoint(0, 0)), control->size())),
                         qPrintable(control->objectName()));
            }
        }
        QVERIFY(dialog.findChild<QDoubleSpinBox *>(channel + "RandomSpin")->isVisible());
        if (channel == "rotation") QVERIFY(dialog.findChild<QDoubleSpinBox *>("orientationRandomSpin")->isVisible());
        if (channel == "frame") QVERIFY(dialog.findChild<QComboBox *>("frameModeCombo")->isVisible());
    }
    QVERIFY(!buttons->button(QDialogButtonBox::Apply)->isEnabled());
    auto result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, original);
}

void EmitterEditDialogTests::scalarTimelineEdits_data() { scalarViewsAreReadOnlyAndFit_data(); }

void EmitterEditDialogTests::scalarTimelineEdits()
{
    QFETCH(QString, channel);
    QFETCH(int, tab);
    const auto original = makeFixtureDefinition();
    EmitterEditDialog dialog(original);
    dialog.findChild<QTabWidget *>("tabWidget")->setCurrentIndex(tab);
    dialog.show();
    QApplication::processEvents();
    auto *bar = dialog.findChild<EmitterKeyframeBar *>(channel + "GraphBar");
    auto *value = dialog.findChild<QDoubleSpinBox *>(channel + "KeyValueSpin");
    auto *time = dialog.findChild<QDoubleSpinBox *>(channel + "KeyTimeSpin");
    auto *table = dialog.findChild<QTableWidget *>(channel + "KeysTable");
    QVERIFY(bar && value && time && table);
    // Starting value stays editable; its fixed time cannot move or be deleted.
    QVERIFY(!time->isEnabled());
    value->setValue(0.125);
    QTest::keyClick(bar, Qt::Key_Delete);
    QCOMPARE(bar->keys().size(), 3);
    QCOMPARE(bar->keys()[0].time, 0.0);
    QTest::mouseDClick(bar, Qt::LeftButton, Qt::NoModifier, bar->keyPosition(1));
    QCOMPARE(bar->selectedKey(), 1);
    QVERIFY(value->hasFocus());
    value->setValue(-0.375);
    QCOMPARE(qobject_cast<QDoubleSpinBox *>(table->cellWidget(0, 1))->value(), -0.375);
    // Numeric edits feed back into the graph without rounding signed/fractional values.
    qobject_cast<QDoubleSpinBox *>(table->cellWidget(0, 1))->setValue(-0.625);
    QCOMPARE(value->value(), -0.625);
    time->setValue(2.0);
    QCOMPARE(bar->keys()[1].time, 2.0);
    const auto positionAt = [bar](double position) {
        const double pixelsPerSecond = (bar->keyPosition(2).x() - bar->keyPosition(0).x()) / bar->keys()[2].time;
        return QPoint(bar->keyPosition(0).x() + qRound(position * pixelsPerSecond), bar->keyPosition(0).y());
    };
    const QPoint end = positionAt(3.0);
    QTest::mousePress(bar, Qt::LeftButton, Qt::NoModifier, bar->keyPosition(1));
    QMouseEvent move(QEvent::MouseMove, end, bar->mapToGlobal(end), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(bar, &move);
    QTest::mouseRelease(bar, Qt::LeftButton, Qt::NoModifier, end);
    const double movedTime = bar->keys()[1].time;
    QVERIFY(std::abs(movedTime - 3.0) < 0.05);
    const auto beforeInsert = bar->keys();
    QPoint insert = positionAt(3.75);
    insert.ry() -= 20;
    QTest::mouseClick(bar, Qt::LeftButton, Qt::ControlModifier, insert);
    QCOMPARE(bar->keys().size(), 4);
    QCOMPARE(bar->selectedKey(), 2);
    const auto inserted = bar->keys()[2];
    const double blend = (inserted.time - movedTime) / (beforeInsert[2].time - movedTime);
    QVERIFY(std::abs(inserted.red - (beforeInsert[1].red + (beforeInsert[2].red - beforeInsert[1].red) * blend)) < 0.00001);
    QTest::keyClick(bar, Qt::Key_Delete);
    QCOMPARE(bar->keys().size(), 3);
    auto expected = original;
    OwnedProperty<float> property;
    getScalarProperty(expected, channel, property.value);
    property.value.Start = 0.125f;
    property.value.Values[0] = -0.625f;
    property.value.KeyTimes[0] = static_cast<float>(movedTime);
    setScalarProperty(expected, channel, property.value);
    auto result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, expected);
}

void EmitterEditDialogTests::scalarApplyRescaleAndCancel_data() { scalarViewsAreReadOnlyAndFit_data(); }

void EmitterEditDialogTests::scalarApplyRescaleAndCancel()
{
    QFETCH(QString, channel);
    EmitterEditDialog dialog(makeFixtureDefinition());
    auto *bar = dialog.findChild<EmitterKeyframeBar *>(channel + "GraphBar");
    auto *value = dialog.findChild<QDoubleSpinBox *>(channel + "KeyValueSpin");
    auto *time = dialog.findChild<QDoubleSpinBox *>(channel + "KeyTimeSpin");
    auto *buttons = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(bar && value && time && buttons);
    int applies = 0;
    dialog.setApplyHandler([&](const ParticleEmitterDefClass &, const QString &) { ++applies; return true; }, dialog.originalName());
    bar->setSelectedKey(1);
    value->setValue(0.375);
    time->setValue(2.0);
    dialog.findChild<QDoubleSpinBox *>("lifetimeSpin")->setValue(20.0);
    QCOMPARE(applies, 0);
    buttons->button(QDialogButtonBox::Apply)->click();
    QCOMPARE(applies, 1);
    QCOMPARE(bar->duration(), 20.0);
    QCOMPARE(bar->keys()[1].time, 4.0);
    QCOMPARE(time->value(), 4.0);
    QCOMPARE(value->value(), 0.375);
    QVERIFY(!buttons->button(QDialogButtonBox::Apply)->isEnabled());
    const std::unique_ptr<ParticleEmitterDefClass> applied(dialog.definition());
    value->setValue(-9.75);
    buttons->button(QDialogButtonBox::Cancel)->click();
    QCOMPARE(applies, 1);
    const std::unique_ptr<ParticleEmitterDefClass> result(dialog.definition());
    compareDefinitions(*result, *applied);
}

void EmitterEditDialogTests::scalarGraphRanges()
{
    EmitterKeyframeBar bar;
    bar.setScalarMode(true);
    bar.setKeys({{0.0, 0.0}, {0.0, 0.0}}, 0.0);
    QCOMPARE(bar.valueRange(), qMakePair(0.0, 1.0));
    bar.setKeys({{0.0, -0.25}, {1.0, -0.25}}, 1.0);
    QCOMPARE(bar.valueRange(), qMakePair(-0.25, 0.0));
    bar.setKeys({{0.0, -0.5}, {1.0, 0.25}}, 1.0);
    QCOMPARE(bar.valueRange(), qMakePair(-0.5, 0.25));
    QCOMPARE(bar.interpolatedKey(0.5).red, -0.125);
    // Coincident times must not divide by zero; rendering is read-only.
    bar.setKeys({{0.0, 0.0}, {0.0, 0.125}, {1.0, -0.375}}, -1.0);
    QVERIFY(std::isfinite(bar.interpolatedKey(0.0).red));
    QVERIFY(!bar.grab().isNull());
    QCOMPARE(bar.keys()[2].red, -0.375);
}

void EmitterEditDialogTests::timelineHandlesDegenerateLifetime_data()
{
    QTest::addColumn<float>("lifetime");
    QTest::newRow("zero") << 0.0f;
    QTest::newRow("negative") << -1.0f;
    QTest::newRow("unlimited") << 5000000.0f;
    QTest::newRow("keys-after-lifetime") << 1.0f;
}

void EmitterEditDialogTests::timelineHandlesDegenerateLifetime()
{
    QFETCH(float, lifetime);
    auto original = makeFixtureDefinition();
    original.Set_Lifetime(lifetime);
    EmitterEditDialog dialog(original);
    auto *bar = dialog.findChild<EmitterKeyframeBar *>("colorGradientBar");
    QVERIFY(bar);
    QVERIFY(std::isfinite(bar->duration()));
    QVERIFY(bar->duration() >= 4.5);
    const auto sample = bar->interpolatedKey(2.5);
    QVERIFY(std::isfinite(sample.red));
    auto result = acceptDialog(dialog);
    QVERIFY(result);
    compareDefinitions(*result, original);
}

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
