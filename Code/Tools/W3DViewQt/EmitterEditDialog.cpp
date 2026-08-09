#include "EmitterEditDialog.h"

#include "ui_EmitterEditDialog.h"

#include "KeyframeTableUtils.h"

#include "part_ldr.h"
#include "shader.h"
#include "v3_rnd.h"
#include "vector2.h"
#include "vector3.h"
#include "w3d_file.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <algorithm>
#include <memory>
#include <utility>

namespace {
constexpr double kWideMinimum = -1000000000.0;
constexpr double kWideMaximum = 1000000000.0;
constexpr double kMaximumKeyTime = 5000000.0;

struct ShaderPreset {
    const char *label;
    ShaderClass shader;
};

ShaderPreset BuildPreset(const char *label, const ShaderClass &shader)
{
    return ShaderPreset{label, shader};
}

const ShaderPreset *ShaderPresets(int &count)
{
    static ShaderPreset presets[] = {
        BuildPreset("Additive", ShaderClass::_PresetAdditiveSpriteShader),
        BuildPreset("Alpha", ShaderClass::_PresetAlphaSpriteShader),
        BuildPreset("Alpha-Test", ShaderClass::_PresetATestSpriteShader),
        BuildPreset("Alpha-Test-Blend", ShaderClass::_PresetATestBlendSpriteShader),
        BuildPreset("Screen", ShaderClass::_PresetScreenSpriteShader),
        BuildPreset("Multiplicative", ShaderClass::_PresetMultiplicativeSpriteShader),
        BuildPreset("Opaque", ShaderClass::_PresetOpaqueSpriteShader),
    };

    count = static_cast<int>(sizeof(presets) / sizeof(presets[0]));
    return presets;
}

bool ShaderMatches(const ShaderClass &a, const ShaderClass &b)
{
    return a.Get_Bits() == b.Get_Bits();
}

void ConfigureSpin(QDoubleSpinBox *spin,
                   double minimum = kWideMinimum,
                   double maximum = kWideMaximum,
                   int decimals = 6)
{
    spin->setRange(minimum, maximum);
    spin->setDecimals(decimals);
    spin->setKeyboardTracking(false);
}

void ConfigureTable(QTableWidget *table, const QStringList &headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(false);
    table->setShowGrid(true);
}

const QVector<KeyframeColumnSpec> &ColorKeySpecs()
{
    static const QVector<KeyframeColumnSpec> specs = {
        {0.0, kMaximumKeyTime, 6},
        {0.0, 1.0, 6},
        {0.0, 1.0, 6},
        {0.0, 1.0, 6},
    };
    return specs;
}

const QVector<KeyframeColumnSpec> &OpacityKeySpecs()
{
    static const QVector<KeyframeColumnSpec> specs = {
        {0.0, kMaximumKeyTime, 6},
        {0.0, 1.0, 6},
    };
    return specs;
}

const QVector<KeyframeColumnSpec> &ScalarKeySpecs()
{
    static const QVector<KeyframeColumnSpec> specs = {
        {0.0, kMaximumKeyTime, 6},
        {kWideMinimum, kWideMaximum, 6},
    };
    return specs;
}

template<typename T>
void FreeProperty(ParticlePropertyStruct<T> &property)
{
    delete[] property.KeyTimes;
    delete[] property.Values;
    property.KeyTimes = nullptr;
    property.Values = nullptr;
    property.NumKeyFrames = 0;
}

QVector<QVector<double>> SortedRows(const QTableWidget *table)
{
    QVector<QVector<double>> rows = GetKeyframeRows(table);
    std::sort(rows.begin(), rows.end(), [](const QVector<double> &a, const QVector<double> &b) {
        const double timeA = a.isEmpty() ? 0.0 : a[0];
        const double timeB = b.isEmpty() ? 0.0 : b[0];
        return timeA < timeB;
    });
    return rows;
}

void ReplaceScalarKeys(ParticlePropertyStruct<float> &property, const QTableWidget *table)
{
    const QVector<QVector<double>> rows = SortedRows(table);
    delete[] property.KeyTimes;
    delete[] property.Values;
    property.NumKeyFrames = static_cast<unsigned int>(rows.size());
    property.KeyTimes = property.NumKeyFrames ? new float[property.NumKeyFrames] : nullptr;
    property.Values = property.NumKeyFrames ? new float[property.NumKeyFrames] : nullptr;
    for (unsigned int index = 0; index < property.NumKeyFrames; ++index) {
        const QVector<double> &row = rows[static_cast<int>(index)];
        property.KeyTimes[index] = row.isEmpty() ? 0.0f : static_cast<float>(row[0]);
        property.Values[index] = row.size() < 2 ? 0.0f : static_cast<float>(row[1]);
    }
}

void ReplaceVectorKeys(ParticlePropertyStruct<Vector3> &property, const QTableWidget *table)
{
    const QVector<QVector<double>> rows = SortedRows(table);
    delete[] property.KeyTimes;
    delete[] property.Values;
    property.NumKeyFrames = static_cast<unsigned int>(rows.size());
    property.KeyTimes = property.NumKeyFrames ? new float[property.NumKeyFrames] : nullptr;
    property.Values = property.NumKeyFrames ? new Vector3[property.NumKeyFrames] : nullptr;
    for (unsigned int index = 0; index < property.NumKeyFrames; ++index) {
        const QVector<double> &row = rows[static_cast<int>(index)];
        property.KeyTimes[index] = row.isEmpty() ? 0.0f : static_cast<float>(row[0]);
        property.Values[index] = Vector3(row.size() > 1 ? static_cast<float>(row[1]) : 0.0f,
                                         row.size() > 2 ? static_cast<float>(row[2]) : 0.0f,
                                         row.size() > 3 ? static_cast<float>(row[3]) : 0.0f);
    }
}

void ScaleTableKeyTimes(QTableWidget *table, float conversion)
{
    if (!table) {
        return;
    }

    for (int row = 0; row < table->rowCount(); ++row) {
        if (auto *timeSpin = qobject_cast<QDoubleSpinBox *>(table->cellWidget(row, 0))) {
            timeSpin->setValue(timeSpin->value() * conversion);
        }
    }
}

double PromptKeyTime(QWidget *parent, const QString &title, bool &ok)
{
    return QInputDialog::getDouble(parent,
                                   title,
                                   "Time (seconds):",
                                   0.0,
                                   0.0,
                                   kMaximumKeyTime,
                                   6,
                                   &ok);
}
}

EmitterEditDialog::EmitterEditDialog(const ParticleEmitterDefClass &definition, QWidget *parent)
    : QDialog(parent)
    , _definition(definition)
    , _ui(new Ui::EmitterEditDialog)
{
    _ui->setupUi(this);
    configureControls();
    loadFromDefinition();
    _registeredName = _originalName;
    connectDirtyTracking();

    connect(_ui->browseButton, &QPushButton::clicked, this, &EmitterEditDialog::browseTexture);
    connect(_ui->useLifetimeCheck, &QCheckBox::toggled, this, &EmitterEditDialog::toggleLifetime);
    connect(_ui->limitParticlesCheck, &QCheckBox::toggled, this, &EmitterEditDialog::toggleMaxParticles);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &EmitterEditDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    if (QPushButton *applyButton = _ui->buttonBox->button(QDialogButtonBox::Apply)) {
        connect(applyButton, &QPushButton::clicked, this, &EmitterEditDialog::apply);
    }

    updateRenderModeTabs();
    updateApplyButton();
}

EmitterEditDialog::~EmitterEditDialog()
{
    delete _ui;
}

ParticleEmitterDefClass *EmitterEditDialog::definition() const
{
    return new ParticleEmitterDefClass(_definition);
}

QString EmitterEditDialog::originalName() const
{
    return _originalName;
}

void EmitterEditDialog::setApplyHandler(ApplyHandler handler,
                                        const QString &registeredName,
                                        bool initialApplyRequired)
{
    _applyHandler = std::move(handler);
    _registeredName = registeredName;
    _initialApplyRequired = initialApplyRequired;
    updateApplyButton();
}

void EmitterEditDialog::configureControls()
{
    int presetCount = 0;
    const ShaderPreset *presets = ShaderPresets(presetCount);
    for (int index = 0; index < presetCount; ++index) {
        _ui->shaderCombo->addItem(presets[index].label, index);
    }

    _ui->renderModeCombo->addItem("Triangles", W3D_EMITTER_RENDER_MODE_TRI_PARTICLES);
    _ui->renderModeCombo->addItem("Quads", W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES);
    _ui->renderModeCombo->addItem("Line", W3D_EMITTER_RENDER_MODE_LINE);
    _ui->renderModeCombo->addItem("Line Group (Tetra)", W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA);
    _ui->renderModeCombo->addItem("Line Group (Prism)", W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM);

    _ui->frameModeCombo->addItem("1x1", W3D_EMITTER_FRAME_MODE_1x1);
    _ui->frameModeCombo->addItem("2x2", W3D_EMITTER_FRAME_MODE_2x2);
    _ui->frameModeCombo->addItem("4x4", W3D_EMITTER_FRAME_MODE_4x4);
    _ui->frameModeCombo->addItem("8x8", W3D_EMITTER_FRAME_MODE_8x8);
    _ui->frameModeCombo->addItem("16x16", W3D_EMITTER_FRAME_MODE_16x16);

    _ui->lineMappingCombo->addItem("Uniform Width", W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP);
    _ui->lineMappingCombo->addItem("Uniform Length", W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP);
    _ui->lineMappingCombo->addItem("Tiled", W3D_ELINE_TILED_TEXTURE_MAP);

    for (int index = 0; index < EMITTER_TYPEID_COUNT; ++index) {
        _ui->userTypeCombo->addItem(QString::fromLatin1(EMITTER_TYPE_NAMES[index]), index);
    }

    const auto populateRandomizers = [](QComboBox *combo) {
        combo->addItem("Solid Box", Vector3Randomizer::CLASSID_SOLIDBOX);
        combo->addItem("Solid Sphere", Vector3Randomizer::CLASSID_SOLIDSPHERE);
        combo->addItem("Hollow Sphere", Vector3Randomizer::CLASSID_HOLLOWSPHERE);
        combo->addItem("Solid Cylinder", Vector3Randomizer::CLASSID_SOLIDCYLINDER);
    };
    populateRandomizers(_ui->creationTypeCombo);
    populateRandomizers(_ui->velocityRandomTypeCombo);

    const QList<QDoubleSpinBox *> wideSpins = {
        _ui->lifetimeSpin,
        _ui->emissionRateSpin,
        _ui->burstSizeSpin,
        _ui->maxParticlesSpin,
        _ui->fadeTimeSpin,
        _ui->creationValue1Spin,
        _ui->creationValue2Spin,
        _ui->creationValue3Spin,
        _ui->velocityXSpin,
        _ui->velocityYSpin,
        _ui->velocityZSpin,
        _ui->velocityRandomValue1Spin,
        _ui->velocityRandomValue2Spin,
        _ui->velocityRandomValue3Spin,
        _ui->accelXSpin,
        _ui->accelYSpin,
        _ui->accelZSpin,
        _ui->outwardVelSpin,
        _ui->inheritVelSpin,
        _ui->gravitySpin,
        _ui->elasticitySpin,
        _ui->sizeStartSpin,
        _ui->sizeRandomSpin,
        _ui->lineNoiseSpin,
        _ui->lineMergeAbortSpin,
        _ui->lineTileSpin,
        _ui->lineUSpin,
        _ui->lineVSpin,
        _ui->rotationStartSpin,
        _ui->rotationRandomSpin,
        _ui->orientationRandomSpin,
        _ui->frameStartSpin,
        _ui->frameRandomSpin,
        _ui->blurStartSpin,
        _ui->blurRandomSpin,
    };
    for (QDoubleSpinBox *spin : wideSpins) {
        ConfigureSpin(spin);
    }

    for (QDoubleSpinBox *spin : {_ui->colorStartRSpin,
                                 _ui->colorStartGSpin,
                                 _ui->colorStartBSpin,
                                 _ui->colorRandomRSpin,
                                 _ui->colorRandomGSpin,
                                 _ui->colorRandomBSpin,
                                 _ui->opacityStartSpin,
                                 _ui->opacityRandomSpin}) {
        ConfigureSpin(spin, 0.0, 1.0, 6);
    }
    ConfigureSpin(_ui->burstSizeSpin, 0.0, 4294967295.0, 0);
    ConfigureSpin(_ui->maxParticlesSpin, 0.0, kWideMaximum, 0);
    _ui->lineSubdivisionSpin->setRange(0, 8);

    ConfigureTable(_ui->colorKeysTable, {"Time (s)", "Red", "Green", "Blue"});
    ConfigureTable(_ui->opacityKeysTable, {"Time (s)", "Opacity"});
    ConfigureTable(_ui->sizeKeysTable, {"Time (s)", "Size"});
    ConfigureTable(_ui->rotationKeysTable, {"Time (s)", "Rotations / sec"});
    ConfigureTable(_ui->frameKeysTable, {"Time (s)", "Frame / U"});
    ConfigureTable(_ui->blurKeysTable, {"Time (s)", "Blur time"});
}

void EmitterEditDialog::connectDirtyTracking()
{
    const auto dirtyLineEdit = [this](QLineEdit *edit, const char *key) {
        connect(edit, &QLineEdit::textEdited, this, [this, key]() { markDirty(QString::fromLatin1(key)); });
    };
    const auto dirtyDouble = [this](QDoubleSpinBox *spin, const char *key) {
        connect(spin,
                qOverload<double>(&QDoubleSpinBox::valueChanged),
                this,
                [this, key]() { markDirty(QString::fromLatin1(key)); });
    };
    const auto dirtySpin = [this](QSpinBox *spin, const char *key) {
        connect(spin,
                qOverload<int>(&QSpinBox::valueChanged),
                this,
                [this, key]() { markDirty(QString::fromLatin1(key)); });
    };
    const auto dirtyCheck = [this](QCheckBox *check, const char *key) {
        connect(check, &QCheckBox::toggled, this, [this, key]() { markDirty(QString::fromLatin1(key)); });
    };
    const auto dirtyCombo = [this](QComboBox *combo, const char *key) {
        connect(combo,
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                [this, key]() { markDirty(QString::fromLatin1(key)); });
    };

    dirtyLineEdit(_ui->nameEdit, "general.name");
    dirtyLineEdit(_ui->textureEdit, "general.texture");
    dirtyCheck(_ui->useLifetimeCheck, "general.lifetime");
    dirtyDouble(_ui->lifetimeSpin, "general.lifetime");
    dirtyCombo(_ui->shaderCombo, "general.shader");
    dirtyCombo(_ui->renderModeCombo, "general.renderMode");
    connect(_ui->renderModeCombo,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &EmitterEditDialog::updateRenderModeTabs);

    dirtyDouble(_ui->emissionRateSpin, "particle.rate");
    dirtyDouble(_ui->burstSizeSpin, "particle.burst");
    dirtyCheck(_ui->limitParticlesCheck, "particle.max");
    dirtyDouble(_ui->maxParticlesSpin, "particle.max");
    dirtyDouble(_ui->fadeTimeSpin, "particle.fade");
    dirtyCombo(_ui->creationTypeCombo, "particle.creation");
    dirtyDouble(_ui->creationValue1Spin, "particle.creation");
    dirtyDouble(_ui->creationValue2Spin, "particle.creation");
    dirtyDouble(_ui->creationValue3Spin, "particle.creation");
    connect(_ui->creationTypeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        updateRandomizerControls(_ui->creationTypeCombo,
                                 _ui->creationValue1Label,
                                 _ui->creationValue2Label,
                                 _ui->creationValue3Label,
                                 _ui->creationValue1Spin,
                                 _ui->creationValue2Spin,
                                 _ui->creationValue3Spin);
    });

    dirtyDouble(_ui->velocityXSpin, "physics.velocity.x");
    dirtyDouble(_ui->velocityYSpin, "physics.velocity.y");
    dirtyDouble(_ui->velocityZSpin, "physics.velocity.z");
    dirtyCombo(_ui->velocityRandomTypeCombo, "physics.randomizer");
    dirtyDouble(_ui->velocityRandomValue1Spin, "physics.randomizer");
    dirtyDouble(_ui->velocityRandomValue2Spin, "physics.randomizer");
    dirtyDouble(_ui->velocityRandomValue3Spin, "physics.randomizer");
    connect(_ui->velocityRandomTypeCombo,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            [this]() {
                updateRandomizerControls(_ui->velocityRandomTypeCombo,
                                         _ui->velocityRandomValue1Label,
                                         _ui->velocityRandomValue2Label,
                                         _ui->velocityRandomValue3Label,
                                         _ui->velocityRandomValue1Spin,
                                         _ui->velocityRandomValue2Spin,
                                         _ui->velocityRandomValue3Spin);
            });
    dirtyDouble(_ui->accelXSpin, "physics.acceleration.x");
    dirtyDouble(_ui->accelYSpin, "physics.acceleration.y");
    dirtyDouble(_ui->accelZSpin, "physics.acceleration.z");
    dirtyDouble(_ui->outwardVelSpin, "physics.outward");
    dirtyDouble(_ui->inheritVelSpin, "physics.inherit");
    dirtyDouble(_ui->gravitySpin, "physics.gravity");
    dirtyDouble(_ui->elasticitySpin, "physics.elasticity");

    dirtyDouble(_ui->colorStartRSpin, "color.start.r");
    dirtyDouble(_ui->colorStartGSpin, "color.start.g");
    dirtyDouble(_ui->colorStartBSpin, "color.start.b");
    dirtyDouble(_ui->colorRandomRSpin, "color.random.r");
    dirtyDouble(_ui->colorRandomGSpin, "color.random.g");
    dirtyDouble(_ui->colorRandomBSpin, "color.random.b");
    dirtyDouble(_ui->opacityStartSpin, "opacity.start");
    dirtyDouble(_ui->opacityRandomSpin, "opacity.random");

    dirtyDouble(_ui->sizeStartSpin, "size.start");
    dirtyDouble(_ui->sizeRandomSpin, "size.random");

    dirtyCombo(_ui->userTypeCombo, "user.type");
    connect(_ui->userStringEdit, &QPlainTextEdit::textChanged, this, [this]() { markDirty("user.string"); });

    dirtyCombo(_ui->lineMappingCombo, "line.mapping");
    dirtyCheck(_ui->lineMergeCheck, "line.merge");
    dirtyCheck(_ui->lineFreezeCheck, "line.freeze");
    dirtyCheck(_ui->lineDisableSortingCheck, "line.sorting");
    dirtyCheck(_ui->lineEndCapsCheck, "line.endCaps");
    dirtySpin(_ui->lineSubdivisionSpin, "line.subdivision");
    dirtyDouble(_ui->lineNoiseSpin, "line.noise");
    dirtyDouble(_ui->lineMergeAbortSpin, "line.mergeAbort");
    dirtyDouble(_ui->lineTileSpin, "line.tile");
    dirtyDouble(_ui->lineUSpin, "line.u");
    dirtyDouble(_ui->lineVSpin, "line.v");

    dirtyDouble(_ui->rotationStartSpin, "rotation.start");
    dirtyDouble(_ui->rotationRandomSpin, "rotation.random");
    dirtyDouble(_ui->orientationRandomSpin, "rotation.orientationRandom");

    dirtyDouble(_ui->frameStartSpin, "frame.start");
    dirtyDouble(_ui->frameRandomSpin, "frame.random");
    dirtyCombo(_ui->frameModeCombo, "frame.mode");

    dirtyDouble(_ui->blurStartSpin, "blur.start");
    dirtyDouble(_ui->blurRandomSpin, "blur.random");

    connectTableEditors(_ui->colorKeysTable, "color.keys");
    connectTableEditors(_ui->opacityKeysTable, "opacity.keys");
    connectTableEditors(_ui->sizeKeysTable, "size.keys");
    connectTableEditors(_ui->rotationKeysTable, "rotation.keys");
    connectTableEditors(_ui->frameKeysTable, "frame.keys");
    connectTableEditors(_ui->blurKeysTable, "blur.keys");

    const auto connectTableButtons = [this](QPushButton *addButton,
                                             QPushButton *removeButton,
                                             QPushButton *sortButton,
                                             QTableWidget *table,
                                             const QVector<KeyframeColumnSpec> &specs,
                                             const QString &dirtyKey,
                                             const QString &title,
                                             const QVector<double> &defaultValues) {
        connect(addButton, &QPushButton::clicked, this, [this, table, specs, dirtyKey, title, defaultValues]() {
            bool ok = false;
            const double time = PromptKeyTime(this, title, ok);
            if (!ok) {
                return;
            }
            QVector<double> values{time};
            values += defaultValues;
            AddKeyframeRow(table, values, specs);
            SortKeyframeRows(table, specs);
            connectTableEditors(table, dirtyKey);
            markDirty(dirtyKey);
        });
        connect(removeButton, &QPushButton::clicked, this, [this, table, dirtyKey]() {
            RemoveSelectedKeyframeRows(table);
            markDirty(dirtyKey);
        });
        connect(sortButton, &QPushButton::clicked, this, [this, table, specs, dirtyKey]() {
            SortKeyframeRows(table, specs);
            connectTableEditors(table, dirtyKey);
            markDirty(dirtyKey);
        });
    };

    connectTableButtons(_ui->colorAddButton,
                        _ui->colorRemoveButton,
                        _ui->colorSortButton,
                        _ui->colorKeysTable,
                        ColorKeySpecs(),
                        "color.keys",
                        "Add Color Key",
                        {_ui->colorStartRSpin->value(), _ui->colorStartGSpin->value(), _ui->colorStartBSpin->value()});
    connectTableButtons(_ui->opacityAddButton,
                        _ui->opacityRemoveButton,
                        _ui->opacitySortButton,
                        _ui->opacityKeysTable,
                        OpacityKeySpecs(),
                        "opacity.keys",
                        "Add Opacity Key",
                        {_ui->opacityStartSpin->value()});
    connectTableButtons(_ui->sizeAddButton,
                        _ui->sizeRemoveButton,
                        _ui->sizeSortButton,
                        _ui->sizeKeysTable,
                        ScalarKeySpecs(),
                        "size.keys",
                        "Add Size Key",
                        {_ui->sizeStartSpin->value()});
    connectTableButtons(_ui->rotationAddButton,
                        _ui->rotationRemoveButton,
                        _ui->rotationSortButton,
                        _ui->rotationKeysTable,
                        ScalarKeySpecs(),
                        "rotation.keys",
                        "Add Rotation Key",
                        {_ui->rotationStartSpin->value()});
    connectTableButtons(_ui->frameAddButton,
                        _ui->frameRemoveButton,
                        _ui->frameSortButton,
                        _ui->frameKeysTable,
                        ScalarKeySpecs(),
                        "frame.keys",
                        "Add Frame / U Key",
                        {_ui->frameStartSpin->value()});
    connectTableButtons(_ui->blurAddButton,
                        _ui->blurRemoveButton,
                        _ui->blurSortButton,
                        _ui->blurKeysTable,
                        ScalarKeySpecs(),
                        "blur.keys",
                        "Add Blur-Time Key",
                        {_ui->blurStartSpin->value()});
}

void EmitterEditDialog::connectTableEditors(QTableWidget *table, const QString &dirtyKey)
{
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int column = 0; column < table->columnCount(); ++column) {
            auto *spin = qobject_cast<QDoubleSpinBox *>(table->cellWidget(row, column));
            if (!spin || spin->property("emitterDirtyConnected").toBool()) {
                continue;
            }
            spin->setProperty("emitterDirtyConnected", true);
            connect(spin,
                    qOverload<double>(&QDoubleSpinBox::valueChanged),
                    this,
                    [this, dirtyKey]() { markDirty(dirtyKey); });
        }
    }
}

bool EmitterEditDialog::updateDefinitionFromUi()
{
    const QString name = _ui->nameEdit->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Emitter", "Invalid emitter name. Please enter a name.");
        return false;
    }

    const float oldLifetime = _definition.Get_Lifetime();
    const float newLifetime = _ui->useLifetimeCheck->isChecked()
                                  ? static_cast<float>(_ui->lifetimeSpin->value())
                                  : 5000000.0f;
    const bool lifetimeChanged = isDirty("general.lifetime") && newLifetime != oldLifetime;

    if (isDirty("general.name")) {
        const QByteArray bytes = name.toLatin1();
        _definition.Set_Name(bytes.constData());
    }
    if (isDirty("general.texture")) {
        const QByteArray bytes = _ui->textureEdit->text().toLatin1();
        _definition.Set_Texture_Filename(bytes.constData());
    }
    if (isDirty("general.lifetime")) {
        _definition.Set_Lifetime(newLifetime);
    }
    if (isDirty("general.shader")) {
        const int presetIndex = _ui->shaderCombo->currentData().toInt();
        int presetCount = 0;
        const ShaderPreset *presets = ShaderPresets(presetCount);
        if (presetIndex >= 0 && presetIndex < presetCount) {
            _definition.Set_Shader(presets[presetIndex].shader);
        }
    }
    if (isDirty("general.renderMode")) {
        _definition.Set_Render_Mode(_ui->renderModeCombo->currentData().toInt());
    }

    if (isDirty("particle.rate")) {
        _definition.Set_Emission_Rate(static_cast<float>(_ui->emissionRateSpin->value()));
    }
    if (isDirty("particle.burst")) {
        _definition.Set_Burst_Size(static_cast<unsigned int>(_ui->burstSizeSpin->value()));
    }
    if (isDirty("particle.max")) {
        _definition.Set_Max_Emissions(_ui->limitParticlesCheck->isChecked()
                                          ? static_cast<float>(_ui->maxParticlesSpin->value())
                                          : 0.0f);
    }
    if (isDirty("particle.fade")) {
        _definition.Set_Fade_Time(static_cast<float>(_ui->fadeTimeSpin->value()));
    }
    if (isDirty("particle.creation")) {
        if (Vector3Randomizer *randomizer = randomizerFromUi(_ui->creationTypeCombo,
                                                             _ui->creationValue1Spin,
                                                             _ui->creationValue2Spin,
                                                             _ui->creationValue3Spin)) {
            _definition.Set_Creation_Volume(randomizer);
        }
    }

    Vector3 velocity = _definition.Get_Velocity();
    bool velocityChanged = false;
    if (isDirty("physics.velocity.x")) {
        velocity.X = static_cast<float>(_ui->velocityXSpin->value());
        velocityChanged = true;
    }
    if (isDirty("physics.velocity.y")) {
        velocity.Y = static_cast<float>(_ui->velocityYSpin->value());
        velocityChanged = true;
    }
    if (isDirty("physics.velocity.z")) {
        velocity.Z = static_cast<float>(_ui->velocityZSpin->value());
        velocityChanged = true;
    }
    if (velocityChanged) {
        _definition.Set_Velocity(velocity);
    }

    if (isDirty("physics.randomizer")) {
        if (Vector3Randomizer *randomizer = randomizerFromUi(_ui->velocityRandomTypeCombo,
                                                             _ui->velocityRandomValue1Spin,
                                                             _ui->velocityRandomValue2Spin,
                                                             _ui->velocityRandomValue3Spin)) {
            _definition.Set_Velocity_Random(randomizer);
        }
    }

    Vector3 acceleration = _definition.Get_Acceleration();
    bool accelerationChanged = false;
    if (isDirty("physics.acceleration.x")) {
        acceleration.X = static_cast<float>(_ui->accelXSpin->value());
        accelerationChanged = true;
    }
    if (isDirty("physics.acceleration.y")) {
        acceleration.Y = static_cast<float>(_ui->accelYSpin->value());
        accelerationChanged = true;
    }
    if (isDirty("physics.acceleration.z")) {
        acceleration.Z = static_cast<float>(_ui->accelZSpin->value());
        accelerationChanged = true;
    }
    if (accelerationChanged) {
        _definition.Set_Acceleration(acceleration);
    }
    if (isDirty("physics.outward")) {
        _definition.Set_Outward_Vel(static_cast<float>(_ui->outwardVelSpin->value()));
    }
    if (isDirty("physics.inherit")) {
        _definition.Set_Vel_Inherit(static_cast<float>(_ui->inheritVelSpin->value()));
    }
    if (isDirty("physics.gravity")) {
        _definition.Set_Gravity(static_cast<float>(_ui->gravitySpin->value()));
    }
    if (isDirty("physics.elasticity")) {
        _definition.Set_Elasticity(static_cast<float>(_ui->elasticitySpin->value()));
    }

    applyColorKeyframes();
    applyOpacityKeyframes();
    applySizeKeyframes();

    if (isDirty("user.string")) {
        const QByteArray bytes = _ui->userStringEdit->toPlainText().toLatin1();
        _definition.Set_User_String(bytes.constData());
    }
    if (isDirty("user.type")) {
        _definition.Set_User_Type(_ui->userTypeCombo->currentData().toInt());
    }

    if (isDirty("line.mapping")) {
        _definition.Set_Line_Texture_Mapping_Mode(_ui->lineMappingCombo->currentData().toInt());
    }
    if (isDirty("line.merge")) {
        _definition.Set_Merge_Intersections(_ui->lineMergeCheck->isChecked());
    }
    if (isDirty("line.freeze")) {
        _definition.Set_Freeze_Random(_ui->lineFreezeCheck->isChecked());
    }
    if (isDirty("line.sorting")) {
        _definition.Set_Disable_Sorting(_ui->lineDisableSortingCheck->isChecked());
    }
    if (isDirty("line.endCaps")) {
        _definition.Set_End_Caps(_ui->lineEndCapsCheck->isChecked());
    }
    if (isDirty("line.subdivision")) {
        _definition.Set_Subdivision_Level(_ui->lineSubdivisionSpin->value());
    }
    if (isDirty("line.noise")) {
        _definition.Set_Noise_Amplitude(static_cast<float>(_ui->lineNoiseSpin->value()));
    }
    if (isDirty("line.mergeAbort")) {
        _definition.Set_Merge_Abort_Factor(static_cast<float>(_ui->lineMergeAbortSpin->value()));
    }
    if (isDirty("line.tile")) {
        _definition.Set_Texture_Tile_Factor(static_cast<float>(_ui->lineTileSpin->value()));
    }
    Vector2 uvRate = _definition.Get_UV_Offset_Rate();
    bool uvChanged = false;
    if (isDirty("line.u")) {
        uvRate.X = static_cast<float>(_ui->lineUSpin->value());
        uvChanged = true;
    }
    if (isDirty("line.v")) {
        uvRate.Y = static_cast<float>(_ui->lineVSpin->value());
        uvChanged = true;
    }
    if (uvChanged) {
        _definition.Set_UV_Offset_Rate(uvRate);
    }

    applyRotationKeyframes();
    applyFrameKeyframes();
    applyBlurTimeKeyframes();
    if (lifetimeChanged) {
        rescaleKeyframeTimes(oldLifetime, newLifetime);
    }

    return true;
}

bool EmitterEditDialog::commitPendingChanges()
{
    const bool hasPendingChanges = _initialApplyRequired || !_dirtyFields.isEmpty();
    if (!updateDefinitionFromUi()) {
        return false;
    }

    if (!hasPendingChanges) {
        return true;
    }

    if (_applyHandler && !_applyHandler(_definition, _registeredName)) {
        return false;
    }

    if (const char *name = _definition.Get_Name()) {
        _registeredName = QString::fromLatin1(name);
    }
    _dirtyFields.clear();
    _initialApplyRequired = false;
    updateApplyButton();
    return true;
}

void EmitterEditDialog::apply()
{
    commitPendingChanges();
}

void EmitterEditDialog::accept()
{
    if (!commitPendingChanges()) {
        return;
    }

    QDialog::accept();
}

void EmitterEditDialog::browseTexture()
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                       "Select Texture",
                                                       _ui->textureEdit->text(),
                                                       "Texture Files (*.tga *.dds *.png *.jpg *.jpeg);;All Files (*.*)");
    if (!path.isEmpty()) {
        _ui->textureEdit->setText(path);
        markDirty("general.texture");
    }
}

void EmitterEditDialog::toggleLifetime(bool enabled)
{
    _ui->lifetimeSpin->setEnabled(enabled);
}

void EmitterEditDialog::toggleMaxParticles(bool enabled)
{
    _ui->maxParticlesSpin->setEnabled(enabled);
}

void EmitterEditDialog::loadFromDefinition()
{
    if (const char *name = _definition.Get_Name()) {
        _ui->nameEdit->setText(QString::fromLatin1(name));
        _originalName = QString::fromLatin1(name);
    }
    if (const char *texture = _definition.Get_Texture_Filename()) {
        _ui->textureEdit->setText(QString::fromLatin1(texture));
    }

    const float lifetime = _definition.Get_Lifetime();
    const bool useLifetime = lifetime < 100.0f;
    _ui->useLifetimeCheck->setChecked(useLifetime);
    _ui->lifetimeSpin->setEnabled(useLifetime);
    _ui->lifetimeSpin->setValue(useLifetime ? lifetime : 0.0);

    int shaderIndex = findShaderIndex();
    if (shaderIndex < 0) {
        _ui->shaderCombo->addItem("Custom (preserved)", -1);
        shaderIndex = _ui->shaderCombo->count() - 1;
    }
    _ui->shaderCombo->setCurrentIndex(shaderIndex);

    const auto selectData = [](QComboBox *combo, int value, const QString &customLabel) {
        int index = combo->findData(value);
        if (index < 0) {
            combo->addItem(customLabel.arg(value), value);
            index = combo->count() - 1;
        }
        combo->setCurrentIndex(index);
    };
    selectData(_ui->renderModeCombo, _definition.Get_Render_Mode(), "Custom (%1)");
    selectData(_ui->frameModeCombo, _definition.Get_Frame_Mode(), "Custom (%1)");

    _ui->emissionRateSpin->setValue(_definition.Get_Emission_Rate());
    _ui->burstSizeSpin->setValue(_definition.Get_Burst_Size());
    const float maxEmissions = _definition.Get_Max_Emissions();
    const bool limitParticles = maxEmissions != 0.0f;
    _ui->limitParticlesCheck->setChecked(limitParticles);
    _ui->maxParticlesSpin->setEnabled(limitParticles);
    _ui->maxParticlesSpin->setValue(limitParticles ? maxEmissions : 0.0f);
    _ui->fadeTimeSpin->setValue(_definition.Get_Fade_Time());

    loadRandomizer(_definition.Get_Creation_Volume(),
                   _ui->creationTypeCombo,
                   _ui->creationValue1Spin,
                   _ui->creationValue2Spin,
                   _ui->creationValue3Spin);
    updateRandomizerControls(_ui->creationTypeCombo,
                             _ui->creationValue1Label,
                             _ui->creationValue2Label,
                             _ui->creationValue3Label,
                             _ui->creationValue1Spin,
                             _ui->creationValue2Spin,
                             _ui->creationValue3Spin);

    const Vector3 velocity = _definition.Get_Velocity();
    _ui->velocityXSpin->setValue(velocity.X);
    _ui->velocityYSpin->setValue(velocity.Y);
    _ui->velocityZSpin->setValue(velocity.Z);
    loadRandomizer(_definition.Get_Velocity_Random(),
                   _ui->velocityRandomTypeCombo,
                   _ui->velocityRandomValue1Spin,
                   _ui->velocityRandomValue2Spin,
                   _ui->velocityRandomValue3Spin);
    updateRandomizerControls(_ui->velocityRandomTypeCombo,
                             _ui->velocityRandomValue1Label,
                             _ui->velocityRandomValue2Label,
                             _ui->velocityRandomValue3Label,
                             _ui->velocityRandomValue1Spin,
                             _ui->velocityRandomValue2Spin,
                             _ui->velocityRandomValue3Spin);

    const Vector3 acceleration = _definition.Get_Acceleration();
    _ui->accelXSpin->setValue(acceleration.X);
    _ui->accelYSpin->setValue(acceleration.Y);
    _ui->accelZSpin->setValue(acceleration.Z);
    _ui->outwardVelSpin->setValue(_definition.Get_Outward_Vel());
    _ui->inheritVelSpin->setValue(_definition.Get_Vel_Inherit());
    _ui->gravitySpin->setValue(_definition.Get_Gravity());
    _ui->elasticitySpin->setValue(_definition.Get_Elasticity());

    ParticlePropertyStruct<Vector3> colors{};
    _definition.Get_Color_Keyframes(colors);
    _ui->colorStartRSpin->setValue(colors.Start.X);
    _ui->colorStartGSpin->setValue(colors.Start.Y);
    _ui->colorStartBSpin->setValue(colors.Start.Z);
    _ui->colorRandomRSpin->setValue(colors.Rand.X);
    _ui->colorRandomGSpin->setValue(colors.Rand.Y);
    _ui->colorRandomBSpin->setValue(colors.Rand.Z);
    QVector<QVector<double>> colorRows;
    colorRows.reserve(static_cast<int>(colors.NumKeyFrames));
    for (unsigned int index = 0; index < colors.NumKeyFrames; ++index) {
        colorRows.push_back({colors.KeyTimes[index], colors.Values[index].X, colors.Values[index].Y, colors.Values[index].Z});
    }
    SetKeyframeRows(_ui->colorKeysTable, colorRows, ColorKeySpecs());
    FreeProperty(colors);

    ParticlePropertyStruct<float> opacity{};
    _definition.Get_Opacity_Keyframes(opacity);
    _ui->opacityStartSpin->setValue(opacity.Start);
    _ui->opacityRandomSpin->setValue(opacity.Rand);
    QVector<QVector<double>> opacityRows;
    opacityRows.reserve(static_cast<int>(opacity.NumKeyFrames));
    for (unsigned int index = 0; index < opacity.NumKeyFrames; ++index) {
        opacityRows.push_back({opacity.KeyTimes[index], opacity.Values[index]});
    }
    SetKeyframeRows(_ui->opacityKeysTable, opacityRows, OpacityKeySpecs());
    FreeProperty(opacity);

    ParticlePropertyStruct<float> size{};
    _definition.Get_Size_Keyframes(size);
    _ui->sizeStartSpin->setValue(size.Start);
    _ui->sizeRandomSpin->setValue(size.Rand);
    QVector<QVector<double>> sizeRows;
    sizeRows.reserve(static_cast<int>(size.NumKeyFrames));
    for (unsigned int index = 0; index < size.NumKeyFrames; ++index) {
        sizeRows.push_back({size.KeyTimes[index], size.Values[index]});
    }
    SetKeyframeRows(_ui->sizeKeysTable, sizeRows, ScalarKeySpecs());
    FreeProperty(size);

    const char *userString = _definition.Get_User_String();
    _ui->userStringEdit->setPlainText(userString ? QString::fromLatin1(userString) : QString());
    selectData(_ui->userTypeCombo, _definition.Get_User_Type(), "Custom (%1)");

    selectData(_ui->lineMappingCombo, _definition.Get_Line_Texture_Mapping_Mode(), "Custom (%1)");
    _ui->lineMergeCheck->setChecked(_definition.Is_Merge_Intersections() != 0);
    _ui->lineFreezeCheck->setChecked(_definition.Is_Freeze_Random() != 0);
    _ui->lineDisableSortingCheck->setChecked(_definition.Is_Sorting_Disabled() != 0);
    _ui->lineEndCapsCheck->setChecked(_definition.Are_End_Caps_Enabled() != 0);
    _ui->lineSubdivisionSpin->setValue(_definition.Get_Subdivision_Level());
    _ui->lineNoiseSpin->setValue(_definition.Get_Noise_Amplitude());
    _ui->lineMergeAbortSpin->setValue(_definition.Get_Merge_Abort_Factor());
    _ui->lineTileSpin->setValue(_definition.Get_Texture_Tile_Factor());
    const Vector2 uvRate = _definition.Get_UV_Offset_Rate();
    _ui->lineUSpin->setValue(uvRate.X);
    _ui->lineVSpin->setValue(uvRate.Y);

    ParticlePropertyStruct<float> rotation{};
    _definition.Get_Rotation_Keyframes(rotation);
    _ui->rotationStartSpin->setValue(rotation.Start);
    _ui->rotationRandomSpin->setValue(rotation.Rand);
    _ui->orientationRandomSpin->setValue(_definition.Get_Initial_Orientation_Random());
    QVector<QVector<double>> rotationRows;
    rotationRows.reserve(static_cast<int>(rotation.NumKeyFrames));
    for (unsigned int index = 0; index < rotation.NumKeyFrames; ++index) {
        rotationRows.push_back({rotation.KeyTimes[index], rotation.Values[index]});
    }
    SetKeyframeRows(_ui->rotationKeysTable, rotationRows, ScalarKeySpecs());
    FreeProperty(rotation);

    ParticlePropertyStruct<float> frames{};
    _definition.Get_Frame_Keyframes(frames);
    _ui->frameStartSpin->setValue(frames.Start);
    _ui->frameRandomSpin->setValue(frames.Rand);
    QVector<QVector<double>> frameRows;
    frameRows.reserve(static_cast<int>(frames.NumKeyFrames));
    for (unsigned int index = 0; index < frames.NumKeyFrames; ++index) {
        frameRows.push_back({frames.KeyTimes[index], frames.Values[index]});
    }
    SetKeyframeRows(_ui->frameKeysTable, frameRows, ScalarKeySpecs());
    FreeProperty(frames);

    ParticlePropertyStruct<float> blurTimes{};
    _definition.Get_Blur_Time_Keyframes(blurTimes);
    _ui->blurStartSpin->setValue(blurTimes.Start);
    _ui->blurRandomSpin->setValue(blurTimes.Rand);
    QVector<QVector<double>> blurRows;
    blurRows.reserve(static_cast<int>(blurTimes.NumKeyFrames));
    for (unsigned int index = 0; index < blurTimes.NumKeyFrames; ++index) {
        blurRows.push_back({blurTimes.KeyTimes[index], blurTimes.Values[index]});
    }
    SetKeyframeRows(_ui->blurKeysTable, blurRows, ScalarKeySpecs());
    FreeProperty(blurTimes);
}

void EmitterEditDialog::loadRandomizer(Vector3Randomizer *randomizer,
                                       QComboBox *typeCombo,
                                       QDoubleSpinBox *value1,
                                       QDoubleSpinBox *value2,
                                       QDoubleSpinBox *value3)
{
    std::unique_ptr<Vector3Randomizer> owned(randomizer);
    if (!owned) {
        return;
    }

    const int classId = static_cast<int>(owned->Class_ID());
    int index = typeCombo->findData(classId);
    if (index < 0) {
        typeCombo->addItem(QString("Custom (%1, preserved)").arg(classId), classId);
        index = typeCombo->count() - 1;
    }
    typeCombo->setCurrentIndex(index);

    switch (owned->Class_ID()) {
        case Vector3Randomizer::CLASSID_SOLIDBOX: {
            const Vector3 extents = static_cast<Vector3SolidBoxRandomizer *>(owned.get())->Get_Extents();
            value1->setValue(extents.X);
            value2->setValue(extents.Y);
            value3->setValue(extents.Z);
            break;
        }
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
            value1->setValue(static_cast<Vector3SolidSphereRandomizer *>(owned.get())->Get_Radius());
            break;
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            value1->setValue(static_cast<Vector3HollowSphereRandomizer *>(owned.get())->Get_Radius());
            break;
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            value1->setValue(static_cast<Vector3SolidCylinderRandomizer *>(owned.get())->Get_Height());
            value2->setValue(static_cast<Vector3SolidCylinderRandomizer *>(owned.get())->Get_Radius());
            break;
        default:
            break;
    }
}

void EmitterEditDialog::updateRandomizerControls(QComboBox *typeCombo,
                                                  QLabel *value1Label,
                                                  QLabel *value2Label,
                                                  QLabel *value3Label,
                                                  QDoubleSpinBox *value1,
                                                  QDoubleSpinBox *value2,
                                                  QDoubleSpinBox *value3) const
{
    const int classId = typeCombo->currentData().toInt();
    QString label1 = "Value 1:";
    QString label2 = "Value 2:";
    QString label3 = "Value 3:";
    bool enable1 = true;
    bool enable2 = true;
    bool enable3 = true;

    switch (classId) {
        case Vector3Randomizer::CLASSID_SOLIDBOX:
            label1 = "X extent:";
            label2 = "Y extent:";
            label3 = "Z extent:";
            break;
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            label1 = "Radius:";
            label2.clear();
            label3.clear();
            enable2 = false;
            enable3 = false;
            break;
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            label1 = "Height:";
            label2 = "Radius:";
            label3.clear();
            enable3 = false;
            break;
        default:
            label1 = "Custom data is preserved until a known type is selected.";
            label2.clear();
            label3.clear();
            enable1 = false;
            enable2 = false;
            enable3 = false;
            break;
    }

    value1Label->setText(label1);
    value2Label->setText(label2);
    value3Label->setText(label3);
    value1->setEnabled(enable1);
    value2->setEnabled(enable2);
    value3->setEnabled(enable3);
    value2Label->setVisible(enable2);
    value2->setVisible(enable2);
    value3Label->setVisible(enable3);
    value3->setVisible(enable3);
}

Vector3Randomizer *EmitterEditDialog::randomizerFromUi(QComboBox *typeCombo,
                                                        QDoubleSpinBox *value1,
                                                        QDoubleSpinBox *value2,
                                                        QDoubleSpinBox *value3) const
{
    const int classId = typeCombo->currentData().toInt();
    const float first = static_cast<float>(value1->value());
    const float second = static_cast<float>(value2->value());
    const float third = static_cast<float>(value3->value());
    switch (classId) {
        case Vector3Randomizer::CLASSID_SOLIDBOX:
            return new Vector3SolidBoxRandomizer(Vector3(first, second, third));
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
            return new Vector3SolidSphereRandomizer(first);
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            return new Vector3HollowSphereRandomizer(first);
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            return new Vector3SolidCylinderRandomizer(first, second);
        default:
            return nullptr;
    }
}

void EmitterEditDialog::updateRenderModeTabs()
{
    const int mode = _ui->renderModeCombo->currentData().toInt();
    _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->lineTab), mode == W3D_EMITTER_RENDER_MODE_LINE);
    const bool lineGroup = mode == W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA ||
                           mode == W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM;
    _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->lineGroupTab), lineGroup);
}

void EmitterEditDialog::applyColorKeyframes()
{
    const QStringList keys = {"color.start.r", "color.start.g", "color.start.b", "color.random.r",
                              "color.random.g", "color.random.b", "color.keys"};
    bool changed = false;
    for (const QString &key : keys) {
        changed |= isDirty(key);
    }
    if (!changed) {
        return;
    }

    ParticlePropertyStruct<Vector3> property{};
    _definition.Get_Color_Keyframes(property);
    if (isDirty("color.start.r")) property.Start.X = static_cast<float>(_ui->colorStartRSpin->value());
    if (isDirty("color.start.g")) property.Start.Y = static_cast<float>(_ui->colorStartGSpin->value());
    if (isDirty("color.start.b")) property.Start.Z = static_cast<float>(_ui->colorStartBSpin->value());
    if (isDirty("color.random.r")) property.Rand.X = static_cast<float>(_ui->colorRandomRSpin->value());
    if (isDirty("color.random.g")) property.Rand.Y = static_cast<float>(_ui->colorRandomGSpin->value());
    if (isDirty("color.random.b")) property.Rand.Z = static_cast<float>(_ui->colorRandomBSpin->value());
    if (isDirty("color.keys")) ReplaceVectorKeys(property, _ui->colorKeysTable);
    _definition.Set_Color_Keyframes(property);
    FreeProperty(property);
}

void EmitterEditDialog::applyOpacityKeyframes()
{
    if (!isDirty("opacity.start") && !isDirty("opacity.random") && !isDirty("opacity.keys")) return;
    ParticlePropertyStruct<float> property{};
    _definition.Get_Opacity_Keyframes(property);
    if (isDirty("opacity.start")) property.Start = static_cast<float>(_ui->opacityStartSpin->value());
    if (isDirty("opacity.random")) property.Rand = static_cast<float>(_ui->opacityRandomSpin->value());
    if (isDirty("opacity.keys")) ReplaceScalarKeys(property, _ui->opacityKeysTable);
    _definition.Set_Opacity_Keyframes(property);
    FreeProperty(property);
}

void EmitterEditDialog::applySizeKeyframes()
{
    if (!isDirty("size.start") && !isDirty("size.random") && !isDirty("size.keys")) return;
    ParticlePropertyStruct<float> property{};
    _definition.Get_Size_Keyframes(property);
    if (isDirty("size.start")) property.Start = static_cast<float>(_ui->sizeStartSpin->value());
    if (isDirty("size.random")) property.Rand = static_cast<float>(_ui->sizeRandomSpin->value());
    if (isDirty("size.keys")) ReplaceScalarKeys(property, _ui->sizeKeysTable);
    _definition.Set_Size_Keyframes(property);
    FreeProperty(property);
}

void EmitterEditDialog::applyRotationKeyframes()
{
    const bool propertyChanged = isDirty("rotation.start") || isDirty("rotation.random") || isDirty("rotation.keys");
    const bool orientationChanged = isDirty("rotation.orientationRandom");
    if (!propertyChanged && !orientationChanged) return;
    ParticlePropertyStruct<float> property{};
    _definition.Get_Rotation_Keyframes(property);
    if (isDirty("rotation.start")) property.Start = static_cast<float>(_ui->rotationStartSpin->value());
    if (isDirty("rotation.random")) property.Rand = static_cast<float>(_ui->rotationRandomSpin->value());
    if (isDirty("rotation.keys")) ReplaceScalarKeys(property, _ui->rotationKeysTable);
    const float orientation = orientationChanged
                                  ? static_cast<float>(_ui->orientationRandomSpin->value())
                                  : _definition.Get_Initial_Orientation_Random();
    _definition.Set_Rotation_Keyframes(property, orientation);
    FreeProperty(property);
}

void EmitterEditDialog::applyFrameKeyframes()
{
    if (isDirty("frame.mode")) {
        _definition.Set_Frame_Mode(_ui->frameModeCombo->currentData().toInt());
    }
    if (!isDirty("frame.start") && !isDirty("frame.random") && !isDirty("frame.keys")) return;
    ParticlePropertyStruct<float> property{};
    _definition.Get_Frame_Keyframes(property);
    if (isDirty("frame.start")) property.Start = static_cast<float>(_ui->frameStartSpin->value());
    if (isDirty("frame.random")) property.Rand = static_cast<float>(_ui->frameRandomSpin->value());
    if (isDirty("frame.keys")) ReplaceScalarKeys(property, _ui->frameKeysTable);
    _definition.Set_Frame_Keyframes(property);
    FreeProperty(property);
}

void EmitterEditDialog::applyBlurTimeKeyframes()
{
    if (!isDirty("blur.start") && !isDirty("blur.random") && !isDirty("blur.keys")) return;
    ParticlePropertyStruct<float> property{};
    _definition.Get_Blur_Time_Keyframes(property);
    if (isDirty("blur.start")) property.Start = static_cast<float>(_ui->blurStartSpin->value());
    if (isDirty("blur.random")) property.Rand = static_cast<float>(_ui->blurRandomSpin->value());
    if (isDirty("blur.keys")) ReplaceScalarKeys(property, _ui->blurKeysTable);
    _definition.Set_Blur_Time_Keyframes(property);
    FreeProperty(property);
}

void EmitterEditDialog::rescaleKeyframeTimes(float oldLifetime, float newLifetime)
{
    if (oldLifetime == 0.0f) {
        return;
    }
    const float conversion = newLifetime / oldLifetime;

    ScaleTableKeyTimes(_ui->colorKeysTable, conversion);
    ScaleTableKeyTimes(_ui->opacityKeysTable, conversion);
    ScaleTableKeyTimes(_ui->sizeKeysTable, conversion);
    ScaleTableKeyTimes(_ui->rotationKeysTable, conversion);
    ScaleTableKeyTimes(_ui->frameKeysTable, conversion);
    ScaleTableKeyTimes(_ui->blurKeysTable, conversion);

    ParticlePropertyStruct<Vector3> colors{};
    _definition.Get_Color_Keyframes(colors);
    for (unsigned int index = 0; index < colors.NumKeyFrames; ++index) colors.KeyTimes[index] *= conversion;
    _definition.Set_Color_Keyframes(colors);
    FreeProperty(colors);

    ParticlePropertyStruct<float> opacity{};
    _definition.Get_Opacity_Keyframes(opacity);
    for (unsigned int index = 0; index < opacity.NumKeyFrames; ++index) opacity.KeyTimes[index] *= conversion;
    _definition.Set_Opacity_Keyframes(opacity);
    FreeProperty(opacity);

    ParticlePropertyStruct<float> size{};
    _definition.Get_Size_Keyframes(size);
    for (unsigned int index = 0; index < size.NumKeyFrames; ++index) size.KeyTimes[index] *= conversion;
    _definition.Set_Size_Keyframes(size);
    FreeProperty(size);

    ParticlePropertyStruct<float> rotation{};
    _definition.Get_Rotation_Keyframes(rotation);
    for (unsigned int index = 0; index < rotation.NumKeyFrames; ++index) rotation.KeyTimes[index] *= conversion;
    _definition.Set_Rotation_Keyframes(rotation, _definition.Get_Initial_Orientation_Random());
    FreeProperty(rotation);

    ParticlePropertyStruct<float> frames{};
    _definition.Get_Frame_Keyframes(frames);
    for (unsigned int index = 0; index < frames.NumKeyFrames; ++index) frames.KeyTimes[index] *= conversion;
    _definition.Set_Frame_Keyframes(frames);
    FreeProperty(frames);

    ParticlePropertyStruct<float> blurTimes{};
    _definition.Get_Blur_Time_Keyframes(blurTimes);
    for (unsigned int index = 0; index < blurTimes.NumKeyFrames; ++index) blurTimes.KeyTimes[index] *= conversion;
    _definition.Set_Blur_Time_Keyframes(blurTimes);
    FreeProperty(blurTimes);
}

void EmitterEditDialog::markDirty(const QString &key)
{
    _dirtyFields.insert(key);
    updateApplyButton();
}

void EmitterEditDialog::updateApplyButton()
{
    if (!_ui || !_ui->buttonBox) {
        return;
    }

    if (QPushButton *applyButton = _ui->buttonBox->button(QDialogButtonBox::Apply)) {
        applyButton->setEnabled(_initialApplyRequired || !_dirtyFields.isEmpty());
    }
}

bool EmitterEditDialog::isDirty(const QString &key) const
{
    return _dirtyFields.contains(key);
}

int EmitterEditDialog::findShaderIndex() const
{
    ShaderClass current;
    _definition.Get_Shader(current);

    int presetCount = 0;
    const ShaderPreset *presets = ShaderPresets(presetCount);
    for (int index = 0; index < presetCount; ++index) {
        if (ShaderMatches(current, presets[index].shader)) {
            return index;
        }
    }
    return -1;
}
