#pragma once

#include "vector3.h"

#include <QDialog>
#include <QString>

#include <functional>

class QSlider;
namespace Ui {
class ColorLightDialog;
}

class ColorLightDialog final : public QDialog
{
    Q_OBJECT

public:
    using ApplyCallback = std::function<void(const Vector3 &)>;

    explicit ColorLightDialog(const QString &title,
                              const Vector3 &initialColor,
                              ApplyCallback applyCallback,
                              QWidget *parent = nullptr);
    ~ColorLightDialog() override;

    Vector3 selectedColor() const;

public slots:
    void reject() override;

private:
    void colorSliderChanged(QSlider *source, int value);
    void applySelectedColor();
    void updateValueLabels();

    Ui::ColorLightDialog *_ui = nullptr;
    Vector3 _initialColor;
    ApplyCallback _applyCallback;
};
