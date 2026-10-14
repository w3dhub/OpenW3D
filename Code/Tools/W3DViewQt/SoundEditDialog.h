#pragma once

#include <QDialog>
#include <QString>

class SoundRenderObjClass;

namespace Ui {
class SoundEditDialog;
}

class SoundEditDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SoundEditDialog(SoundRenderObjClass *sound, QWidget *parent = nullptr);
    ~SoundEditDialog() override;

    SoundRenderObjClass *sound() const;
    QString oldName() const;

protected:
    void accept() override;

private slots:
    void browseSoundFile();
    void toggleSoundType();
    void playSound();

private:
    void loadFromSound();
    void updateEnableState();

    Ui::SoundEditDialog *_ui = nullptr;
    SoundRenderObjClass *_sound = nullptr;
    QString _oldName;
};
