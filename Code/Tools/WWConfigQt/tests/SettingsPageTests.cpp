#include "AudioPage.h"
#include "MainWindow.h"
#include "PerformancePage.h"
#include "VideoPage.h"
#include "WWConfigBackend.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QTest>
#include <QTimer>

namespace
{
struct BackendState
{
    RenderSettings render;
    VideoSettings video;
    AudioSettings audio;
    std::vector<VideoAdapterInfo> adapters;
    int saves = 0;
    bool saveSucceeds = true;
} state;

template<class T>
T *Control(QObject &owner, const char *name)
{
    auto *control = owner.findChild<T *>(QString::fromLatin1(name));
    if (!control) {
        qFatal("Missing control: %s", name);
    }
    return control;
}
}

// Link a deterministic backend into this UI test instead of writing real INI or registry settings.
WWConfigBackend::WWConfigBackend() = default;
WWConfigBackend::~WWConfigBackend() = default;
QString WWConfigBackend::localizedString(int) const { return {}; }
QString WWConfigBackend::configPath() const { return QStringLiteral("test/openw3d.conf"); }
RenderSettings WWConfigBackend::loadRenderSettings() const { return state.render; }
VideoSettings WWConfigBackend::loadVideoSettings() const { return state.video; }
AudioSettings WWConfigBackend::loadAudioSettings() const { return state.audio; }
std::vector<VideoAdapterInfo> WWConfigBackend::enumerateVideoAdapters() const { return state.adapters; }
bool WWConfigBackend::saveRenderSettings(const RenderSettings &settings)
{
    ++state.saves;
    if (state.saveSucceeds) { state.render = settings; }
    return state.saveSucceeds;
}
bool WWConfigBackend::saveVideoSettings(const VideoSettings &settings)
{
    ++state.saves;
    if (state.saveSucceeds) { state.video = settings; }
    return state.saveSucceeds;
}
bool WWConfigBackend::saveAudioSettings(const AudioSettings &settings)
{
    ++state.saves;
    if (state.saveSucceeds) { state.audio = settings; }
    return state.saveSucceeds;
}
void WWConfigBackend::autoConfigRenderSettings()
{
    state.render.dynamicLOD = 10000;
    state.render.staticLOD = 10000;
    state.render.shadowMode = 3;
    state.video.width = 1024;
    state.video.height = 768;
}

class SettingsPageTests : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        state = BackendState{};
        state.video.deviceName = "Adapter A";
        state.adapters = {
            {"Adapter A", "Display Adapter A", {{640, 480, 16}, {800, 600, 32}, {1024, 768, 32}}},
            {"Adapter B", "Display Adapter B", {{1920, 1080, 32}}}
        };
    }

    void audioControlsLoadAndSave()
    {
        state.audio.soundVolume = 0.37f;
        state.audio.musicEnabled = false;
        WWConfigBackend backend;
        AudioPage page(backend);
        auto *sound = Control<QSlider>(page, "soundSlider");
        auto *music = Control<QSlider>(page, "musicSlider");
        auto *musicEnabled = Control<QCheckBox>(page, "musicEnableCheck");
        QCOMPARE(sound->value(), 37);
        QVERIFY(!music->isEnabled());
        musicEnabled->setChecked(true);
        QVERIFY(music->isEnabled());
        sound->setValue(62);
        music->setValue(24);
        Control<QComboBox>(page, "qualityCombo")->setCurrentIndex(0);
        Control<QComboBox>(page, "rateCombo")->setCurrentIndex(1);
        Control<QComboBox>(page, "speakerCombo")->setCurrentIndex(2);
        Control<QCheckBox>(page, "stereoCheck")->setChecked(false);
        QCOMPARE(state.saves, 0);
        QVERIFY(page.save());
        QCOMPARE(state.audio.soundVolume, 0.62f);
        QCOMPARE(state.audio.musicVolume, 0.24f);
        QVERIFY(state.audio.musicEnabled);
        QCOMPARE(state.audio.bitDepth, 8);
        QCOMPARE(state.audio.sampleRate, 22050);
        QCOMPARE(state.audio.speakerType, 2);
        QVERIFY(!state.audio.stereo);
        page.refresh();
        QCOMPARE(sound->value(), 62);
    }

    void videoSelectionAndEmptyAdapters()
    {
        WWConfigBackend backend;
        VideoPage page(backend);
        auto *drivers = Control<QListWidget>(page, "driverList");
        auto *resolution = Control<QSlider>(page, "resolutionSlider");
        QCOMPARE(drivers->count(), 2);
        QCOMPARE(Control<QLabel>(page, "resolutionValue")->text(), QStringLiteral("800 x 600"));
        resolution->setValue(1);
        Control<QCheckBox>(page, "windowedCheck")->setChecked(true);
        QVERIFY(page.save());
        QCOMPARE(state.video.width, 1024);
        QCOMPARE(state.video.height, 768);
        QVERIFY(state.video.windowed);
        drivers->setCurrentRow(1);
        QCOMPARE(Control<QLabel>(page, "resolutionValue")->text(), QStringLiteral("1920 x 1080"));
        QVERIFY(page.save());
        QCOMPARE(state.video.deviceName, std::string("Adapter B"));
        QCOMPARE(state.video.width, 1920);
        state.adapters.clear();
        page.refresh();
        QVERIFY(!drivers->isEnabled());
        QVERIFY(!resolution->isEnabled());
        QVERIFY(!Control<QComboBox>(page, "bitDepthCombo")->isEnabled());
        QCOMPARE(Control<QLabel>(page, "resolutionValue")->text(), QStringLiteral("N/A"));
    }

    void performancePresetsAndExpertSettings()
    {
        WWConfigBackend backend;
        PerformancePage page(backend);
        auto *expert = Control<QCheckBox>(page, "expertCheck");
        auto *geometry = Control<QSlider>(page, "geometrySlider");
        QVERIFY(!geometry->isEnabled());
        Control<QSlider>(page, "overallSlider")->setValue(3);
        QVERIFY(page.save());
        QCOMPARE(state.render.dynamicLOD, 10000);
        QCOMPARE(state.render.shadowMode, 3);
        QCOMPARE(state.render.textureResolution, 0);
        expert->setChecked(true);
        QVERIFY(geometry->isEnabled());
        geometry->setValue(0);
        Control<QSlider>(page, "textureSlider")->setValue(0);
        Control<QCheckBox>(page, "terrainCheck")->setChecked(false);
        QVERIFY(page.save());
        QCOMPARE(state.render.dynamicLOD, 0);
        QCOMPARE(state.render.textureResolution, 2);
        QCOMPARE(state.render.staticShadows, 0);
        expert->setChecked(false);
        QVERIFY(!geometry->isEnabled());
    }

    void autoConfigRemainsPendingUntilSave()
    {
        state.render.dynamicLOD = 0;
        state.render.staticLOD = 0;
        WWConfigBackend backend;
        PerformancePage page(backend);
        Control<QPushButton>(page, "autoButton")->click();
        QCOMPARE(Control<QSlider>(page, "geometrySlider")->value(), 2);
        QCOMPARE(state.render.dynamicLOD, 0);
        QCOMPARE(state.video.width, 800);
        QVERIFY(page.save());
        QCOMPARE(state.render.dynamicLOD, 10000);
    }

    void cancelDoesNotSaveAndOkSavesAllPages()
    {
        WWConfigBackend backend;
        {
            MainWindow window(backend);
            window.show();
            Control<QSlider>(window, "soundSlider")->setValue(20);
            Control<QDialogButtonBox>(window, "buttonBox")->button(QDialogButtonBox::Cancel)->click();
            QVERIFY(!window.isVisible());
            QCOMPARE(state.saves, 0);
            QCOMPARE(state.audio.soundVolume, 1.0f);
        }
        MainWindow window(backend);
        window.show();
        Control<QSlider>(window, "soundSlider")->setValue(20);
        Control<QDialogButtonBox>(window, "buttonBox")->button(QDialogButtonBox::Ok)->click();
        QCOMPARE(state.saves, 3);
        QCOMPARE(state.audio.soundVolume, 0.20f);
        QVERIFY(!window.isVisible());
    }

    void failedSaveKeepsWindowOpen()
    {
        state.saveSucceeds = false;
        WWConfigBackend backend;
        MainWindow window(backend);
        window.show();
        bool sawError = false;
        QTimer::singleShot(0, [&sawError]() {
            auto *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (message) {
                sawError = message->text().contains(QStringLiteral("test/openw3d.conf"));
                message->accept();
            }
        });
        Control<QDialogButtonBox>(window, "buttonBox")->button(QDialogButtonBox::Ok)->click();
        QVERIFY(sawError);
        QVERIFY(window.isVisible());
        QCOMPARE(state.saves, 3);
    }

    void formsFitAndResourcesRender()
    {
        WWConfigBackend backend;
        MainWindow window(backend);
        auto *tabs = Control<QTabWidget>(window, "tabWidget");
        QCOMPARE(tabs->count(), 3);
        QVERIFY(!window.windowIcon().isNull());
        QVERIFY(!Control<QLabel>(window, "banner")->pixmap().isNull());
        const QString screenshotDir = qEnvironmentVariable("OPENW3D_QT_SCREENSHOT_DIR");
        if (!screenshotDir.isEmpty()) {
            QVERIFY(QDir().mkpath(screenshotDir));
        }
        for (const QSize size : {window.minimumSize(), QSize(720, 640)}) {
            window.resize(size);
            window.show();
            for (int tab = 0; tab < tabs->count(); ++tab) {
                tabs->setCurrentIndex(tab);
                QApplication::processEvents();
                for (QWidget *control : window.centralWidget()->findChildren<QWidget *>()) {
                    if (control->isWindow() || !control->isVisible() || control->width() == 0) {
                        continue;
                    }
                    const QRect bounds(control->mapTo(window.centralWidget(), QPoint()), control->size());
                    QVERIFY2(window.centralWidget()->rect().contains(bounds), qPrintable(control->objectName()));
                }
                if (!screenshotDir.isEmpty()) {
                    QVERIFY(window.grab().save(screenshotDir + QStringLiteral("/wwconfig-%1-%2.png").arg(tab).arg(size.width())));
                }
            }
        }
    }
};

QTEST_MAIN(SettingsPageTests)
#include "SettingsPageTests.moc"
