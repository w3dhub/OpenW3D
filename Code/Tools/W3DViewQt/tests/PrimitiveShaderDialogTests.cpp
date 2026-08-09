#include "RingEditDialog.h"
#include "RenderObjUtils.h"
#include "SphereEditDialog.h"

#include "assetmgr.h"
#include "chunkio.h"
#include "ramfile.h"
#include "ringobj.h"
#include "shader.h"
#include "sphereobj.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QtTest/QTest>

#include <array>
#include <memory>

namespace {
template<typename T>
struct ReleaseRef {
    void operator()(T *object) const
    {
        if (object) {
            object->Release_Ref();
        }
    }
};

template<typename T>
using RefPtr = std::unique_ptr<T, ReleaseRef<T>>;

class FailOnceRAMFile final : public RAMFileClass
{
public:
    FailOnceRAMFile(void *buffer, int length, int failedWrite)
        : RAMFileClass(buffer, length), failedWrite_(failedWrite)
    {
    }

    int Write(const void *buffer, int size) override
    {
        ++writeCount_;
        if (writeCount_ == failedWrite_) {
            return 0;
        }
        return RAMFileClass::Write(buffer, size);
    }

    int writeCount() const { return writeCount_; }

private:
    int failedWrite_ = 0;
    int writeCount_ = 0;
};

ShaderClass customShader()
{
    ShaderClass shader = ShaderClass::_PresetAdditiveShader;
    shader.Set_Fog_Func(ShaderClass::FOG_ENABLE);
    return shader;
}

QComboBox *shaderCombo(QDialog &dialog)
{
    auto *combo = dialog.findChild<QComboBox *>("shaderCombo");
    if (!combo) {
        QTest::qFail("shaderCombo was not created from the Designer form", __FILE__, __LINE__);
    }
    return combo;
}

void acceptDialog(QDialog &dialog)
{
    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    if (!buttonBox) {
        QTest::qFail("buttonBox was not created from the Designer form", __FILE__, __LINE__);
        return;
    }

    auto *okButton = buttonBox->button(QDialogButtonBox::Ok);
    if (!okButton) {
        QTest::qFail("buttonBox has no OK button", __FILE__, __LINE__);
        return;
    }

    okButton->click();
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Accepted));
}

QPushButton *dialogButton(QDialog &dialog, QDialogButtonBox::StandardButton button)
{
    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    if (!buttonBox) {
        QTest::qFail("buttonBox was not created from the Designer form", __FILE__, __LINE__);
        return nullptr;
    }

    auto *result = buttonBox->button(button);
    if (!result) {
        QTest::qFail("requested standard button is missing", __FILE__, __LINE__);
    }
    return result;
}

template<typename Dialog>
void selectPreset(Dialog &dialog, const QString &label)
{
    QComboBox *combo = shaderCombo(dialog);
    QVERIFY(combo);
    const int index = combo->findText(label);
    QVERIFY2(index >= 0, qPrintable(QString("Shader preset '%1' was not found").arg(label)));
    combo->setCurrentIndex(index);
}
}

class PrimitiveShaderDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void spherePreservesCustomShaderOnAccept();
    void sphereAppliesExplicitKnownPreset();
    void ringPreservesCustomShaderOnAccept();
    void ringAppliesExplicitKnownPreset();
    void sphereCancelRestoresLastAppliedPreview();
    void ringCleanOkDoesNotApplyTwice();
    void prototypeNameCollisionIsNonDestructive();
    void primitiveSerializersReportWriteFailure();
};

void PrimitiveShaderDialogTests::spherePreservesCustomShaderOnAccept()
{
    RefPtr<SphereRenderObjClass> sphere(new SphereRenderObjClass);
    sphere->Set_Name("CustomSphere");
    ShaderClass original = customShader();
    sphere->Set_Shader(original);

    SphereEditDialog dialog(sphere.get());
    QComboBox *combo = shaderCombo(dialog);
    QVERIFY(combo);
    QCOMPARE(combo->currentText(), QString("Custom (preserved)"));
    QCOMPARE(combo->currentData().toInt(), -1);

    acceptDialog(dialog);
    QCOMPARE(sphere->Get_Shader().Get_Bits(), original.Get_Bits());
}

void PrimitiveShaderDialogTests::sphereAppliesExplicitKnownPreset()
{
    RefPtr<SphereRenderObjClass> sphere(new SphereRenderObjClass);
    sphere->Set_Name("PresetSphere");
    ShaderClass original = customShader();
    sphere->Set_Shader(original);

    SphereEditDialog dialog(sphere.get());
    selectPreset(dialog, "Opaque");
    acceptDialog(dialog);

    QCOMPARE(sphere->Get_Shader().Get_Bits(), ShaderClass::_PresetOpaqueShader.Get_Bits());
}

void PrimitiveShaderDialogTests::ringPreservesCustomShaderOnAccept()
{
    RefPtr<RingRenderObjClass> ring(new RingRenderObjClass);
    ring->Set_Name("CustomRing");
    ShaderClass original = customShader();
    ring->Set_Shader(original);

    RingEditDialog dialog(ring.get());
    QComboBox *combo = shaderCombo(dialog);
    QVERIFY(combo);
    QCOMPARE(combo->currentText(), QString("Custom (preserved)"));
    QCOMPARE(combo->currentData().toInt(), -1);

    acceptDialog(dialog);
    QCOMPARE(ring->Get_Shader().Get_Bits(), original.Get_Bits());
}

void PrimitiveShaderDialogTests::ringAppliesExplicitKnownPreset()
{
    RefPtr<RingRenderObjClass> ring(new RingRenderObjClass);
    ring->Set_Name("PresetRing");
    ShaderClass original = customShader();
    ring->Set_Shader(original);

    RingEditDialog dialog(ring.get());
    selectPreset(dialog, "Opaque");
    acceptDialog(dialog);

    QCOMPARE(ring->Get_Shader().Get_Bits(), ShaderClass::_PresetOpaqueShader.Get_Bits());
}

void PrimitiveShaderDialogTests::sphereCancelRestoresLastAppliedPreview()
{
    RefPtr<SphereRenderObjClass> sphere(new SphereRenderObjClass);
    sphere->Set_Name("OriginalSphere");
    sphere->Set_Animation_Duration(1.0f);

    SphereEditDialog dialog(sphere.get());
    int applyCount = 0;
    QString appliedFrom;
    dialog.setApplyHandler(
        [&](SphereRenderObjClass &, const QString &registeredName) {
            ++applyCount;
            appliedFrom = registeredName;
            return true;
        },
        "OriginalSphere");

    auto *nameEdit = dialog.findChild<QLineEdit *>("nameEdit");
    auto *lifetimeSpin = dialog.findChild<QDoubleSpinBox *>("lifetimeSpin");
    QVERIFY(nameEdit);
    QVERIFY(lifetimeSpin);
    nameEdit->setText("AppliedSphere");
    lifetimeSpin->setValue(2.0);
    QCOMPARE(QString::fromLatin1(sphere->Get_Name()), QString("AppliedSphere"));
    QCOMPARE(sphere->Get_Animation_Duration(), 2.0f);

    QPushButton *applyButton = dialogButton(dialog, QDialogButtonBox::Apply);
    QVERIFY(applyButton);
    QVERIFY(applyButton->isEnabled());
    applyButton->click();
    QCOMPARE(applyCount, 1);
    QCOMPARE(appliedFrom, QString("OriginalSphere"));
    QCOMPARE(dialog.registeredName(), QString("AppliedSphere"));
    QVERIFY(!applyButton->isEnabled());

    lifetimeSpin->setValue(3.0);
    QCOMPARE(sphere->Get_Animation_Duration(), 3.0f);
    QPushButton *cancelButton = dialogButton(dialog, QDialogButtonBox::Cancel);
    QVERIFY(cancelButton);
    cancelButton->click();
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
    QCOMPARE(QString::fromLatin1(sphere->Get_Name()), QString("AppliedSphere"));
    QCOMPARE(sphere->Get_Animation_Duration(), 2.0f);
    QCOMPARE(applyCount, 1);
}

void PrimitiveShaderDialogTests::ringCleanOkDoesNotApplyTwice()
{
    RefPtr<RingRenderObjClass> ring(new RingRenderObjClass);
    ring->Set_Name("OriginalRing");
    ring->Set_Texture_Tiling(1);

    RingEditDialog dialog(ring.get());
    int applyCount = 0;
    QStringList registeredNames;
    dialog.setApplyHandler(
        [&](RingRenderObjClass &, const QString &registeredName) {
            ++applyCount;
            registeredNames.push_back(registeredName);
            return true;
        },
        "OriginalRing");

    auto *nameEdit = dialog.findChild<QLineEdit *>("nameEdit");
    auto *tilingSpin = dialog.findChild<QSpinBox *>("tilingSpin");
    QVERIFY(nameEdit);
    QVERIFY(tilingSpin);
    nameEdit->setText("AppliedRing");
    tilingSpin->setValue(2);
    QCOMPARE(QString::fromLatin1(ring->Get_Name()), QString("AppliedRing"));
    QCOMPARE(ring->Get_Texture_Tiling(), 2);

    QPushButton *applyButton = dialogButton(dialog, QDialogButtonBox::Apply);
    QVERIFY(applyButton);
    applyButton->click();
    QCOMPARE(applyCount, 1);
    QCOMPARE(registeredNames, QStringList{"OriginalRing"});
    QVERIFY(!applyButton->isEnabled());

    acceptDialog(dialog);
    QCOMPARE(applyCount, 1);
    QCOMPARE(dialog.registeredName(), QString("AppliedRing"));
}

void PrimitiveShaderDialogTests::prototypeNameCollisionIsNonDestructive()
{
    WW3DAssetManager assetManager;
    RefPtr<SphereRenderObjClass> source(new SphereRenderObjClass);
    source->Set_Name("SourceSphere");
    RefPtr<RingRenderObjClass> destination(new RingRenderObjClass);
    destination->Set_Name("TakenName");

    QString errorMessage;
    QVERIFY(UpdateSpherePrototype(*source, QString(), &errorMessage));
    QVERIFY(UpdateRingPrototype(*destination, QString(), &errorMessage));
    auto *sourcePrototype = assetManager.Find_Prototype("SourceSphere");
    auto *destinationPrototype = assetManager.Find_Prototype("TakenName");
    QVERIFY(sourcePrototype);
    QVERIFY(destinationPrototype);

    source->Set_Name("TakenName");
    QVERIFY(!UpdateSpherePrototype(*source, "SourceSphere", &errorMessage));
    QVERIFY(errorMessage.contains("already exists"));
    QCOMPARE(assetManager.Find_Prototype("SourceSphere"), sourcePrototype);
    QCOMPARE(assetManager.Find_Prototype("TakenName"), destinationPrototype);
}

void PrimitiveShaderDialogTests::primitiveSerializersReportWriteFailure()
{
    // One chunk header fits, but the first nested definition header does not.
    // This reproduces a short write without relying on a full disk.
    std::array<char, 12> sphereStorage = {};
    RAMFileClass sphereFile(sphereStorage.data(), static_cast<int>(sphereStorage.size()));
    QVERIFY(sphereFile.Open(FileClass::WRITE));
    ChunkSaveClass sphereSave(&sphereFile);
    SpherePrototypeClass spherePrototype;
    QVERIFY(!spherePrototype.Save(sphereSave));
    QCOMPARE(sphereSave.Cur_Chunk_Depth(), 0);
    sphereFile.Close();

    std::array<char, 12> ringStorage = {};
    RAMFileClass ringFile(ringStorage.data(), static_cast<int>(ringStorage.size()));
    QVERIFY(ringFile.Open(FileClass::WRITE));
    ChunkSaveClass ringSave(&ringFile);
    RingPrototypeClass ringPrototype;
    QVERIFY(!ringPrototype.Save(ringSave));
    QCOMPARE(ringSave.Cur_Chunk_Depth(), 0);
    ringFile.Close();

    SphereRenderObjClass animatedSphere;
    animatedSphere.Set_Name("AnimatedSphere");
    animatedSphere.Get_Color_Channel().Add_Key(Vector3(0.25f, 0.5f, 0.75f), 0.0f);
    SpherePrototypeClass animatedSpherePrototype(&animatedSphere);

    std::array<char, 4096> countingSphereStorage = {};
    FailOnceRAMFile countingSphereFile(
        countingSphereStorage.data(), static_cast<int>(countingSphereStorage.size()), 0);
    QVERIFY(countingSphereFile.Open(FileClass::WRITE));
    ChunkSaveClass countingSphereSave(&countingSphereFile);
    QVERIFY(animatedSpherePrototype.Save(countingSphereSave));
    const int sphereWriteCount = countingSphereFile.writeCount();
    QVERIFY(sphereWriteCount > 0);
    countingSphereFile.Close();

    for (int failedWrite = 1; failedWrite <= sphereWriteCount; ++failedWrite) {
        std::array<char, 4096> storage = {};
        FailOnceRAMFile file(storage.data(), static_cast<int>(storage.size()), failedWrite);
        QVERIFY(file.Open(FileClass::WRITE));
        ChunkSaveClass save(&file);
        QVERIFY2(!animatedSpherePrototype.Save(save),
                 qPrintable(QString("Sphere serializer ignored failed write %1 of %2")
                                .arg(failedWrite)
                                .arg(sphereWriteCount)));
        QCOMPARE(save.Cur_Chunk_Depth(), 0);
        QVERIFY(save.Has_Write_Error());
        file.Close();
    }

    RingRenderObjClass animatedRing;
    animatedRing.Set_Name("AnimatedRing");
    animatedRing.Get_Color_Channel().Add_Key(Vector3(0.75f, 0.5f, 0.25f), 0.0f);
    RingPrototypeClass animatedRingPrototype(&animatedRing);

    std::array<char, 4096> countingRingStorage = {};
    FailOnceRAMFile countingRingFile(
        countingRingStorage.data(), static_cast<int>(countingRingStorage.size()), 0);
    QVERIFY(countingRingFile.Open(FileClass::WRITE));
    ChunkSaveClass countingRingSave(&countingRingFile);
    QVERIFY(animatedRingPrototype.Save(countingRingSave));
    const int ringWriteCount = countingRingFile.writeCount();
    QVERIFY(ringWriteCount > 0);
    countingRingFile.Close();

    for (int failedWrite = 1; failedWrite <= ringWriteCount; ++failedWrite) {
        std::array<char, 4096> storage = {};
        FailOnceRAMFile file(storage.data(), static_cast<int>(storage.size()), failedWrite);
        QVERIFY(file.Open(FileClass::WRITE));
        ChunkSaveClass save(&file);
        QVERIFY2(!animatedRingPrototype.Save(save),
                 qPrintable(QString("Ring serializer ignored failed write %1 of %2")
                                .arg(failedWrite)
                                .arg(ringWriteCount)));
        QCOMPARE(save.Cur_Chunk_Depth(), 0);
        QVERIFY(save.Has_Write_Error());
        file.Close();
    }
}

QTEST_MAIN(PrimitiveShaderDialogTests)

#include "PrimitiveShaderDialogTests.moc"
