#pragma once

#include <QDialog>

#include "sphereobj.h"

namespace Ui {
class OpacityVectorEditDialog;
}

class OpacityVectorEditDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit OpacityVectorEditDialog(const AlphaVectorStruct &value, QWidget *parent = nullptr);
    ~OpacityVectorEditDialog() override;

    AlphaVectorStruct value() const;

private slots:
    void handleIntensitySlider(int value);
    void handleIntensitySpin(double value);
    void handleAngleChanged();

private:
    void syncFromValue();
    float sliderPositionFromIntensity(float intensity) const;
    float intensityFromSliderPosition(float position) const;
    void updateValueFromControls();

    Ui::OpacityVectorEditDialog *_ui = nullptr;
    AlphaVectorStruct _value;
};
