#include "ResolutionDialog.h"

#include "ui_ResolutionDialog.h"

#include "rddesc.h"
#include "ww3d.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QtGlobal>

#include <algorithm>

namespace {
constexpr int kRoleWidth = Qt::UserRole + 1;
constexpr int kRoleHeight = Qt::UserRole + 2;
constexpr int kRoleBpp = Qt::UserRole + 3;

QVector<ResolutionDialog::Mode> enumerateModes()
{
    QVector<ResolutionDialog::Mode> modes;
    const RenderDeviceDescClass &device_info = WW3D::Get_Render_Device_Desc();
    const DynamicVectorClass<ResolutionDescClass> &res_list = device_info.Enumerate_Resolutions();
    modes.reserve(res_list.Count());
    for (int index = 0; index < res_list.Count(); ++index) {
        modes.push_back(ResolutionDialog::Mode(
            res_list[index].Width, res_list[index].Height, res_list[index].BitDepth));
    }
    return modes;
}

ResolutionDialog::Mode currentMode()
{
    ResolutionDialog::Mode mode;
    bool windowed = true;
    WW3D::Get_Device_Resolution(
        mode.width, mode.height, mode.bitsPerPixel, windowed);
    return mode;
}
} // namespace

ResolutionDialog::ResolutionDialog(const Mode &preferredMode,
                                   bool borderlessFullscreen,
                                   QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::ResolutionDialog)
    , _currentMode(currentMode())
    , _preferredMode(preferredMode)
{
    initialize(enumerateModes(), borderlessFullscreen);
}

ResolutionDialog::ResolutionDialog(const QVector<Mode> &availableModes,
                                   const Mode &currentMode,
                                   const Mode &preferredMode,
                                   bool borderlessFullscreen,
                                   QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::ResolutionDialog)
    , _currentMode(currentMode)
    , _preferredMode(preferredMode)
{
    initialize(availableModes, borderlessFullscreen);
}

ResolutionDialog::~ResolutionDialog()
{
    delete _ui;
}

void ResolutionDialog::initialize(const QVector<Mode> &availableModes,
                                  bool borderlessFullscreen)
{
    _ui->setupUi(this);
    _ui->fullscreenCheck->setChecked(borderlessFullscreen);

    connect(_ui->resolutionTable,
            &QTableWidget::cellDoubleClicked,
            this,
            &ResolutionDialog::onDoubleClicked);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVector<Mode> modes;
    modes.reserve(availableModes.size());
    for (const Mode &mode : availableModes) {
        if (mode.isValid() &&
            (_currentMode.bitsPerPixel <= 0 ||
             mode.bitsPerPixel == _currentMode.bitsPerPixel)) {
            modes.push_back(mode);
        }
    }

    std::sort(modes.begin(), modes.end(), [](const Mode &left, const Mode &right) {
        if (left.width != right.width) {
            return left.width < right.width;
        }
        if (left.height != right.height) {
            return left.height < right.height;
        }
        return left.bitsPerPixel < right.bitsPerPixel;
    });
    modes.erase(std::unique(modes.begin(), modes.end()), modes.end());

    if (modes.isEmpty() && _currentMode.isValid()) {
        modes.push_back(_currentMode);
    }

    _ui->resolutionTable->setRowCount(0);
    const auto append_resolution = [this](int width, int height, int bpp) {
        const int row = _ui->resolutionTable->rowCount();
        _ui->resolutionTable->insertRow(row);

        auto *res_item = new QTableWidgetItem(QString("%1 x %2").arg(width).arg(height));
        res_item->setData(kRoleWidth, width);
        res_item->setData(kRoleHeight, height);
        res_item->setData(kRoleBpp, bpp);
        _ui->resolutionTable->setItem(row, 0, res_item);

        const quint64 colors = (bpp >= 0 && bpp < 63) ? (quint64(1) << bpp) : 0;
        auto *bpp_item =
            new QTableWidgetItem(QString("%1 bpp (%2 colors)").arg(bpp).arg(colors));
        _ui->resolutionTable->setItem(row, 1, bpp_item);
    };

    for (const Mode &mode : modes) {
        append_resolution(mode.width, mode.height, mode.bitsPerPixel);
    }

    selectDefaultRow();
}

int ResolutionDialog::selectedWidth() const
{
    const int row = _ui->resolutionTable->currentRow();
    if (row < 0) {
        return 0;
    }

    const auto *item = _ui->resolutionTable->item(row, 0);
    return item ? item->data(kRoleWidth).toInt() : 0;
}

int ResolutionDialog::selectedHeight() const
{
    const int row = _ui->resolutionTable->currentRow();
    if (row < 0) {
        return 0;
    }

    const auto *item = _ui->resolutionTable->item(row, 0);
    return item ? item->data(kRoleHeight).toInt() : 0;
}

int ResolutionDialog::selectedBitsPerPixel() const
{
    const int row = _ui->resolutionTable->currentRow();
    if (row < 0) {
        return 0;
    }

    const auto *item = _ui->resolutionTable->item(row, 0);
    return item ? item->data(kRoleBpp).toInt() : 0;
}

bool ResolutionDialog::fullscreen() const
{
    return _ui->fullscreenCheck->isChecked();
}

void ResolutionDialog::selectDefaultRow()
{
    int current_row = -1;
    int preferred_row = -1;
    for (int row = 0; row < _ui->resolutionTable->rowCount(); ++row) {
        const auto *item = _ui->resolutionTable->item(row, 0);
        if (!item) {
            continue;
        }

        const Mode mode(item->data(kRoleWidth).toInt(),
                        item->data(kRoleHeight).toInt(),
                        item->data(kRoleBpp).toInt());
        if (preferred_row < 0 && mode == _preferredMode) {
            preferred_row = row;
        }
        if (current_row < 0 && mode == _currentMode) {
            current_row = row;
        }
    }

    const int selected_row = preferred_row >= 0
        ? preferred_row
        : (current_row >= 0 ? current_row : 0);
    if (selected_row >= 0 && selected_row < _ui->resolutionTable->rowCount()) {
        _ui->resolutionTable->setCurrentCell(selected_row, 0);
        _ui->resolutionTable->selectRow(selected_row);
    }
}

void ResolutionDialog::onDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0 && row < _ui->resolutionTable->rowCount()) {
        _ui->resolutionTable->setCurrentCell(row, 0);
        _ui->resolutionTable->selectRow(row);
        accept();
    }
}
