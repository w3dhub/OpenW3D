#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class HAnimClass;
class W3DViewport;

namespace Ui {
class AdvancedAnimationDialog;
}

class AdvancedAnimationDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedAnimationDialog(W3DViewport *viewport,
                                     const QString &renderObjectName,
                                     QWidget *parent = nullptr);
    ~AdvancedAnimationDialog() override;

protected:
    void accept() override;

private slots:
    void updateReport();
    void onTabChanged(int index);

private:
    void loadAnimations();
    void populateMixingList();
    QString makeChannelString(int boneIndex, HAnimClass *anim) const;

    Ui::AdvancedAnimationDialog *_ui = nullptr;
    W3DViewport *_viewport = nullptr;
    QString _renderObjectName;
    QVector<HAnimClass *> _animations;
    bool _hasHierarchy = false;
    QString _hierarchyName;
};
