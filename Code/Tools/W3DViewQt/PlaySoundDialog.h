#pragma once

#include <QDialog>
#include <QString>

class AudibleSoundClass;

namespace Ui {
class PlaySoundDialog;
}

class PlaySoundDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit PlaySoundDialog(const QString &filename, QWidget *parent = nullptr);
    ~PlaySoundDialog() override;

    bool isReady() const;

private slots:
    void playSound();
    void stopSound();

private:
    bool createSound();

    Ui::PlaySoundDialog *_ui = nullptr;
    QString _filename;
    AudibleSoundClass *_sound = nullptr;
};
