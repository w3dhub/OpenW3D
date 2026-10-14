#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class AnimationPropertiesDialog;
}

class AnimationPropertiesDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AnimationPropertiesDialog(const QString &animationName, QWidget *parent = nullptr);
    ~AnimationPropertiesDialog() override;

private:
    void setErrorState(const QString &message);

    Ui::AnimationPropertiesDialog *_ui = nullptr;
};
