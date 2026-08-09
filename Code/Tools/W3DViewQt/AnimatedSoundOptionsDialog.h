#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class AnimatedSoundOptionsDialog;
}

class AnimatedSoundOptionsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AnimatedSoundOptionsDialog(const QString &definitionLibraryPath,
                                        const QString &iniPath,
                                        const QString &dataPath,
                                        QWidget *parent = nullptr);
    ~AnimatedSoundOptionsDialog() override;

    QString definitionLibraryPath() const;
    QString iniPath() const;
    QString dataPath() const;

    static void LoadAnimatedSoundSettings();

private slots:
    void browseDefinitionLibrary();
    void browseIniPath();
    void browseDataPath();

private:
    Ui::AnimatedSoundOptionsDialog *_ui = nullptr;
};
