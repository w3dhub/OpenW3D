#pragma once

#include <QDialog>

class W3DViewport;

namespace Ui {
class AnimationSettingsDialog;
}

class AnimationSettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AnimationSettingsDialog(W3DViewport &viewport, QWidget *parent = nullptr);
    ~AnimationSettingsDialog() override;

private:
    void updateSpeed(int percent);

    Ui::AnimationSettingsDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
};
