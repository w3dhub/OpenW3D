#pragma once

#include <QDialog>

class W3DViewport;

namespace Ui {
class CameraSettingsDialog;
}

class CameraSettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit CameraSettingsDialog(W3DViewport *viewport, QWidget *parent = nullptr);
    ~CameraSettingsDialog() override;

    bool isManualFovEnabled() const;
    bool isManualClipPlanesEnabled() const;
    double hfovDegrees() const;
    double vfovDegrees() const;
    double lensMm() const;
    float nearClip() const;
    float farClip() const;

private slots:
    void onFovCheckChanged(bool checked);
    void onClipCheckChanged(bool checked);
    void onReset();
    void onHfovChanged(double value);
    void onLensChanged(double value);

private:
    void refreshFromViewport();
    void updateLensFromHfov();
    void updateFovFromLens();
    void setFovControlsEnabled(bool enabled);
    void setClipControlsEnabled(bool enabled);

    Ui::CameraSettingsDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
    bool _updating = false;
};
