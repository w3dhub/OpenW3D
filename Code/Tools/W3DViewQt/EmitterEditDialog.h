#pragma once

#include "part_ldr.h"

#include <QDialog>
#include <QSet>
#include <QString>
#include <QVector>
#include <functional>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QTableWidget;
class QPushButton;
class EmitterKeyframeBar;

namespace Ui {
class EmitterEditDialog;
}

class EmitterEditDialog final : public QDialog
{
    Q_OBJECT

public:
    using ApplyHandler = std::function<bool(const ParticleEmitterDefClass &, const QString &)>;

    explicit EmitterEditDialog(const ParticleEmitterDefClass &definition, QWidget *parent = nullptr);
    ~EmitterEditDialog() override;

    ParticleEmitterDefClass *definition() const;
    QString originalName() const;
    void setApplyHandler(ApplyHandler handler,
                         const QString &registeredName,
                         bool initialApplyRequired = false);

protected:
    void accept() override;

private slots:
    void apply();
    void browseTexture();
    void toggleLifetime(bool enabled);
    void toggleMaxParticles(bool enabled);

private:
    struct ScalarTimeline {
        QString prefix;
        EmitterKeyframeBar *bar;
        QTableWidget *table;
        QDoubleSpinBox *start;
        QDoubleSpinBox *time;
        QDoubleSpinBox *value;
        QLabel *selection;
        QPushButton *remove;
    };
    void connectScalarTimelines();
    void refreshScalarTimeline(const ScalarTimeline &channel);
    void refreshSelectedScalarKey(const ScalarTimeline &channel);
    void insertScalarKey(const ScalarTimeline &channel, double time);
    void removeScalarKey(const ScalarTimeline &channel, int index);
    void configureControls();
    void connectDirtyTracking();
    void connectColorTimelines();
    void refreshColorTimelines();
    void refreshSelectedColorKey(bool opacity);
    void editTimelineColor();
    void insertTimelineKey(bool opacity, double time);
    void removeTimelineKey(bool opacity, int index);
    void connectTableEditors(QTableWidget *table, const QString &dirtyKey);
    void loadFromDefinition();
    void loadRandomizer(Vector3Randomizer *randomizer,
                        QComboBox *typeCombo,
                        QDoubleSpinBox *value1,
                        QDoubleSpinBox *value2,
                        QDoubleSpinBox *value3);
    void updateRandomizerControls(QComboBox *typeCombo,
                                  QLabel *value1Label,
                                  QLabel *value2Label,
                                  QLabel *value3Label,
                                  QDoubleSpinBox *value1,
                                  QDoubleSpinBox *value2,
                                  QDoubleSpinBox *value3) const;
    Vector3Randomizer *randomizerFromUi(QComboBox *typeCombo,
                                        QDoubleSpinBox *value1,
                                        QDoubleSpinBox *value2,
                                        QDoubleSpinBox *value3) const;
    void updateRenderModeTabs();
    void applyColorKeyframes();
    void applyOpacityKeyframes();
    void applySizeKeyframes();
    void applyRotationKeyframes();
    void applyFrameKeyframes();
    void applyBlurTimeKeyframes();
    void rescaleKeyframeTimes(float oldLifetime, float newLifetime);
    bool updateDefinitionFromUi();
    bool commitPendingChanges();
    void updateApplyButton();
    void markDirty(const QString &key);
    bool isDirty(const QString &key) const;
    int findShaderIndex() const;

    ParticleEmitterDefClass _definition;
    QString _originalName;
    QString _registeredName;
    QSet<QString> _dirtyFields;
    QVector<ScalarTimeline> _scalarTimelines;
    ApplyHandler _applyHandler;
    bool _initialApplyRequired = false;
    Ui::EmitterEditDialog *_ui = nullptr;
};
