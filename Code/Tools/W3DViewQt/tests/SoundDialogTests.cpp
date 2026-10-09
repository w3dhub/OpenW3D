#include "PlaySoundDialog.h"
#include "SoundEditDialog.h"

#include "AudibleSound.h"
#include "WWAudio.h"

#include <QApplication>
#include <QDataStream>
#include <QDialogButtonBox>
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest/QTest>

#include <memory>

namespace {
QByteArray makeMonoPcmWav()
{
    constexpr quint32 sampleRate = 8000;
    constexpr quint16 channelCount = 1;
    constexpr quint16 bitsPerSample = 8;
    constexpr int sampleCount = 8000;

    QByteArray samples(sampleCount, '\0');
    for (int index = 0; index < samples.size(); ++index) {
        // Unsigned 8-bit square wave with a modest amplitude.
        samples[index] = static_cast<char>(((index / 20) % 2) ? 160 : 96);
    }

    QByteArray wav;
    QDataStream stream(&wav, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32(36 + samples.size());
    stream.writeRawData("WAVE", 4);
    stream.writeRawData("fmt ", 4);
    stream << quint32(16);
    stream << quint16(1); // PCM
    stream << channelCount;
    stream << sampleRate;
    stream << quint32(sampleRate * channelCount * bitsPerSample / 8);
    stream << quint16(channelCount * bitsPerSample / 8);
    stream << bitsPerSample;
    stream.writeRawData("data", 4);
    stream << quint32(samples.size());
    stream.writeRawData(samples.constData(), samples.size());
    return wav;
}

bool isOpenALBackend(const WWAudioClass &audio)
{
    return QString::fromLatin1(audio.Get_3D_Driver_Name().Peek_Buffer()) ==
           QStringLiteral("OpenAL 3D Audio");
}

void closeActiveMessageBox()
{
    if (auto *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
        message->accept();
    }
}
} // namespace

class SoundDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void nameLimitMatchesW3dFormat();
    void runtimeNameLimitRejectsOversizedName();
    void failedPreviewDoesNotOpenPlaybackDialog();
    void successfulPreviewPlaysAndStopsWithOpenAL();
};

void SoundDialogTests::nameLimitMatchesW3dFormat()
{
    SoundEditDialog dialog(nullptr);
    auto *nameEdit = dialog.findChild<QLineEdit *>("nameEdit");
    QVERIFY(nameEdit);
    QCOMPARE(nameEdit->maxLength(), 15);
}

void SoundDialogTests::runtimeNameLimitRejectsOversizedName()
{
    SoundEditDialog dialog(nullptr);
    auto *nameEdit = dialog.findChild<QLineEdit *>("nameEdit");
    auto *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    QVERIFY(nameEdit);
    QVERIFY(buttonBox);

    // Exercise the runtime guard independently of the Designer constraint.
    nameEdit->setMaxLength(64);
    nameEdit->setText("sixteen_characters");

    bool warningClosed = false;
    QTimer::singleShot(0, &dialog, [&warningClosed]() {
        warningClosed = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()) != nullptr;
        closeActiveMessageBox();
    });
    buttonBox->button(QDialogButtonBox::Ok)->click();

    QVERIFY(warningClosed);
    QVERIFY(dialog.result() != QDialog::Accepted);
}

void SoundDialogTests::failedPreviewDoesNotOpenPlaybackDialog()
{
    SoundEditDialog dialog(nullptr);
    auto *fileEdit = dialog.findChild<QLineEdit *>("fileEdit");
    auto *playButton = dialog.findChild<QPushButton *>("playButton");
    QVERIFY(fileEdit);
    QVERIFY(playButton);
    fileEdit->setText("missing-preview.wav");

    bool warningClosed = false;
    bool playbackDialogShown = false;
    QTimer modalMonitor;
    connect(&modalMonitor, &QTimer::timeout, &dialog, [&]() {
        if (auto *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            warningClosed = true;
            message->accept();
            return;
        }

        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (widget->objectName() == "PlaySoundDialog" && widget->isVisible()) {
                playbackDialogShown = true;
                qobject_cast<QDialog *>(widget)->reject();
            }
        }
    });
    modalMonitor.start(0);
    playButton->click();
    modalMonitor.stop();

    QVERIFY(warningClosed);
    QVERIFY(!playbackDialogShown);
}

void SoundDialogTests::successfulPreviewPlaysAndStopsWithOpenAL()
{
    QTemporaryDir fixtureDirectory;
    QVERIFY(fixtureDirectory.isValid());

    const QString soundPath = fixtureDirectory.filePath("preview.wav");
    QFile soundFile(soundPath);
    QVERIFY(soundFile.open(QIODevice::WriteOnly));
    const QByteArray wav = makeMonoPcmWav();
    QCOMPARE(soundFile.write(wav), static_cast<qint64>(wav.size()));
    soundFile.close();

    std::unique_ptr<WWAudioClass> audio(WWAudioClass::Create_Instance());
    QVERIFY(audio != nullptr);
    audio->Initialize();
    if (!isOpenALBackend(*audio)) {
        QSKIP("Successful preview playback requires the OpenAL backend");
    }
    QVERIFY2(audio->Get_2D_Sample_Count() > 0,
             "OpenAL did not create any 2D sources; ensure a playback device or "
             "ALSOFT_DRIVERS=null is available");

    {
        PlaySoundDialog dialog(soundPath);
        QVERIFY(dialog.isReady());

        auto *playButton = dialog.findChild<QPushButton *>("playButton");
        auto *stopButton = dialog.findChild<QPushButton *>("stopButton");
        QVERIFY(playButton);
        QVERIFY(stopButton);

        const auto activePreviewSound = [&audio]() -> AudibleSoundClass * {
            for (int index = 0; index < audio->Get_2D_Sample_Count(); ++index) {
                if (AudibleSoundClass *sound = audio->Peek_2D_Sample(index)) {
                    return sound;
                }
            }
            return nullptr;
        };

        AudibleSoundClass *previewSound = activePreviewSound();
        QVERIFY2(previewSound, "The ready preview did not acquire an OpenAL source");
        QVERIFY(previewSound->Is_Playing());

        stopButton->click();
        QVERIFY(!previewSound->Is_Playing());
        playButton->click();
        QCOMPARE(activePreviewSound(), previewSound);
        QVERIFY(previewSound->Is_Playing());
        stopButton->click();
        QVERIFY(!previewSound->Is_Playing());
    }

    audio.reset();
    QVERIFY(WWAudioClass::Get_Instance() == nullptr);
}

QTEST_MAIN(SoundDialogTests)

#include "SoundDialogTests.moc"
