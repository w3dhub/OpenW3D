#include <QApplication>
#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QCheckBox>
#include <QIcon>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>

#include "MainWindow.h"
#include "WWConfigBackend.h"
#include "../WWConfig/wwconfig_ids.h"
#include "../../wwlib/openw3d.h"

namespace
{
constexpr int kInvalidLanguage = -999;

int LanguageFromString(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == "english" || normalized == "en") {
        return IDL_ENGLISH;
    }
    if (normalized == "french" || normalized == "fr") {
        return IDL_FRENCH;
    }
    if (normalized == "german" || normalized == "de") {
        return IDL_GERMAN;
    }
    if (normalized == "japanese" || normalized == "jp" || normalized == "ja") {
        return IDL_JAPANESE;
    }
    if (normalized == "korean" || normalized == "ko") {
        return IDL_KOREAN;
    }
    if (normalized == "chinese" || normalized == "zh") {
        return IDL_CHINESE;
    }
    if (normalized.isEmpty()) {
        return -1;
    }
    return kInvalidLanguage;
}
} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("WWConfigQt"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Renegade configuration (Qt prototype)"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniOption(
        QStringLiteral("ini"),
        QObject::tr("Use the specified OpenW3D configuration file."),
        QObject::tr("path"));
    parser.addOption(iniOption);

    QCommandLineOption languageOption(
        {QStringLiteral("l"), QStringLiteral("language")},
        QObject::tr("Override the UI language (english, french, german, japanese, korean, chinese)."),
        QObject::tr("language"));
    parser.addOption(languageOption);

    parser.process(app);

    if (parser.isSet(iniOption)) {
        const QString iniPath = QFileInfo(parser.value(iniOption)).absoluteFilePath();
        const QByteArray iniPathBytes = iniPath.toLocal8Bit();
        OpenW3D::Set_Config_File_Path_Override(iniPathBytes.constData());
    }

    app.setWindowIcon(QIcon(QStringLiteral(":/wwconfig/wwconfig.ico")));

    int languageOverride = -1;
    if (parser.isSet(languageOption)) {
        languageOverride = LanguageFromString(parser.value(languageOption));
        if (languageOverride == kInvalidLanguage) {
            qWarning() << "Unknown language override:" << parser.value(languageOption);
            languageOverride = -1;
        }
    }

    WWConfigBackend backend;
    if (!backend.initializeLocale(languageOverride)) {
        qWarning() << "Failed to initialize locale bank, continuing with built-in strings.";
    }

    // Offer to auto-config on first run (no ini file yet).
    const QString configPath = backend.configPath();
    if (!QFileInfo::exists(configPath)) {
        const auto answer = QMessageBox::question(
            nullptr,
            QObject::tr("Auto Config"),
            QObject::tr("No Renegade.ini was found. Run Auto Config now?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        if (answer == QMessageBox::Yes) {
            backend.autoConfigRenderSettings();
        }
    }

    // Driver version warning flow.
    DriverWarningInfo warningInfo;
    if (backend.checkDriverWarning(warningInfo) && warningInfo.show) {
        QMessageBox box(QMessageBox::Warning,
                        QObject::tr("Driver Warning"),
                        warningInfo.message,
                        QMessageBox::Ok);
        QCheckBox *dontShow = new QCheckBox(QObject::tr("Don't show this warning again"));
        box.setCheckBox(dontShow);
        box.exec();
        if (dontShow->isChecked()) {
            backend.disableDriverWarning();
        }
    }

    MainWindow window(backend);
    window.show();
    return app.exec();
}
