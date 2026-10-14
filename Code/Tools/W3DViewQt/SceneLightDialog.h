#pragma once

#include "W3DViewport.h"

#include <QDialog>

class QSlider;
namespace Ui {
class SceneLightDialog;
}

class SceneLightDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SceneLightDialog(W3DViewport &viewport, QWidget *parent = nullptr);
    ~SceneLightDialog() override;

public slots:
    void reject() override;

private:
    enum Channel {
        Diffuse = 1 << 0,
        Specular = 1 << 1,
        Both = Diffuse | Specular,
    };

    void setColorControls(const Vector3 &color);
    void applyColorFromControls();
    void colorSliderChanged(QSlider *source, int value);
    void applyAttenuation();
    void updateAttenuationControls(bool enabled);

    Ui::SceneLightDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
    Channel _currentChannel = Diffuse;
    W3DViewport::SceneLightState _initialState;
};
