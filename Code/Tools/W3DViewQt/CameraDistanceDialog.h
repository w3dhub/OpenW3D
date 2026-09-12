#pragma once

#include <QDialog>

namespace Ui {
class CameraDistanceDialog;
}

class CameraDistanceDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit CameraDistanceDialog(float distance, QWidget *parent = nullptr);
    ~CameraDistanceDialog() override;

    float distance() const;

private:
    Ui::CameraDistanceDialog *_ui = nullptr;
};
