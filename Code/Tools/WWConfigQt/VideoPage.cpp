#include "VideoPage.h"

#include <algorithm>
#include <cstdlib>
#include <limits>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QFontMetrics>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSizePolicy>
#include <QStyle>
#include <QSlider>
#include <QVBoxLayout>

namespace
{
QString AdapterDisplayName(const VideoAdapterInfo &adapter)
{
    if (!adapter.description.empty()) {
        return QString::fromStdString(adapter.description);
    }
    if (!adapter.deviceName.empty()) {
        return QString::fromStdString(adapter.deviceName);
    }
    return QObject::tr("Display Adapter");
}
} // namespace

VideoPage::VideoPage(WWConfigBackend &backend, QWidget *parent)
    : QWidget(parent),
      m_backend(backend)
{
    buildUi();
    refresh();
}

void VideoPage::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(8);

    auto *deviceGroup = new QGroupBox(tr("Device"), this);
    auto *deviceLayout = new QVBoxLayout(deviceGroup);

    auto *driverRow = new QHBoxLayout();
    auto *driverIcon = new QLabel(deviceGroup);
    driverIcon->setPixmap(style()->standardIcon(QStyle::SP_ComputerIcon).pixmap(20, 20));
    driverIcon->setFixedSize(20, 20);
    driverRow->addWidget(driverIcon);
    driverRow->addWidget(new QLabel(tr("Driver:"), deviceGroup));
    driverRow->addStretch();
    deviceLayout->addLayout(driverRow);

    m_driverList = new QListWidget(deviceGroup);
    m_driverList->setUniformItemSizes(true);
    m_driverList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_driverList->setMinimumHeight(64);
    deviceLayout->addWidget(m_driverList);

    layout->addWidget(deviceGroup);

    auto *displayGroup = new QGroupBox(tr("Display"), this);
    auto *displayLayout = new QGridLayout(displayGroup);
    displayLayout->setHorizontalSpacing(6);
    displayLayout->setVerticalSpacing(4);

    displayLayout->addWidget(new QLabel(tr("Resolution:"), displayGroup), 0, 0);
    m_resolutionSlider = new QSlider(Qt::Horizontal, displayGroup);
    m_resolutionSlider->setRange(0, 0);
    m_resolutionSlider->setTickPosition(QSlider::TicksBelow);
    m_resolutionSlider->setTickInterval(1);
    displayLayout->addWidget(m_resolutionSlider, 0, 1);
    m_resolutionValue = new QLabel(tr("N/A"), displayGroup);
    m_resolutionValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_resolutionValue->setFixedWidth(m_resolutionValue->fontMetrics().horizontalAdvance(QStringLiteral("9999 x 9999")) + 8);
    m_resolutionValue->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    displayLayout->addWidget(m_resolutionValue, 0, 2);

    displayLayout->addWidget(new QLabel(tr("Color Depth:"), displayGroup), 1, 0);
    m_bitDepthCombo = new QComboBox(displayGroup);
    displayLayout->addWidget(m_bitDepthCombo, 1, 1, 1, 2);

    m_windowedCheck = new QCheckBox(tr("Windowed Mode"), displayGroup);
    displayLayout->addWidget(m_windowedCheck, 2, 0, 1, 3);

    m_textureDepthLabel = new QLabel(tr("Texture depth: --"), displayGroup);
    displayLayout->addWidget(m_textureDepthLabel, 3, 0, 1, 3);

    layout->addWidget(displayGroup);

    auto *note = new QLabel(tr("Press OK to save video changes to Renegade.ini."), this);
    note->setWordWrap(true);
    layout->addWidget(note);
    layout->addStretch();

    connect(m_driverList, &QListWidget::currentRowChanged, this, [this](int) {
        if (m_blockSignals) {
            return;
        }
        updateBitDepthOptions();
        updateResolutionSlider();
        applySelection();
        updateControlStates();
    });

    connect(m_bitDepthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (m_blockSignals) {
            return;
        }
        m_currentBitDepth = m_bitDepthCombo->currentData().toInt();
        updateResolutionSlider();
        applySelection();
    });

    connect(m_resolutionSlider, &QSlider::valueChanged, this, [this](int) {
        if (m_blockSignals) {
            return;
        }
        updateResolutionLabel();
        applySelection();
    });

    connect(m_windowedCheck, &QCheckBox::toggled, this, [this](bool) {
        if (m_blockSignals) {
            return;
        }
        applySelection();
    });
}

void VideoPage::refresh()
{
    m_blockSignals = true;
    m_settings = m_backend.loadVideoSettings();
    m_adapters = m_backend.enumerateVideoAdapters();
    populateDrivers();
    updateBitDepthOptions();
    updateResolutionSlider();
    m_windowedCheck->setChecked(m_settings.windowed);
    m_textureDepthLabel->setText(tr("Texture depth: %1-bit")
                                     .arg(m_settings.textureDepth > 0 ? m_settings.textureDepth : 0));
    updateControlStates();
    m_blockSignals = false;
    updateResolutionLabel();
    applySelection();
}

bool VideoPage::save()
{
    applySelection();
    return m_backend.saveVideoSettings(m_settings);
}

void VideoPage::populateDrivers()
{
    m_driverList->clear();

    if (m_adapters.empty()) {
        auto *item = new QListWidgetItem(tr("No adapters detected"));
        item->setFlags(Qt::NoItemFlags);
        m_driverList->addItem(item);
        return;
    }

    const QString desired = QString::fromStdString(m_settings.deviceName);
    int selectedIndex = 0;
    for (int i = 0; i < static_cast<int>(m_adapters.size()); ++i) {
        const QString label = AdapterDisplayName(m_adapters[i]);
        auto *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, i);
        m_driverList->addItem(item);
        if (!desired.isEmpty()) {
            const QString adapterName = QString::fromStdString(m_adapters[i].deviceName);
            if (adapterName.compare(desired, Qt::CaseInsensitive) == 0) {
                selectedIndex = i;
            }
        }
    }

    m_driverList->setCurrentRow(selectedIndex);
}

void VideoPage::updateBitDepthOptions()
{
    m_bitDepthCombo->clear();

    const VideoAdapterInfo *adapter = currentAdapter();
    if (!adapter) {
        m_currentBitDepth = (m_settings.bitDepth > 0) ? m_settings.bitDepth : 32;
        return;
    }

    std::vector<int> depths;
    depths.reserve(adapter->resolutions.size());
    for (const VideoResolution &mode : adapter->resolutions) {
        if (std::find(depths.begin(), depths.end(), mode.bitDepth) == depths.end()) {
            depths.push_back(mode.bitDepth);
        }
    }
    std::sort(depths.begin(), depths.end());

    if (depths.empty()) {
        m_currentBitDepth = (m_settings.bitDepth > 0) ? m_settings.bitDepth : 32;
        return;
    }

    int targetDepth = (m_settings.bitDepth > 0) ? m_settings.bitDepth : depths.front();
    if (std::find(depths.begin(), depths.end(), targetDepth) == depths.end()) {
        targetDepth = depths.front();
    }

    for (int depth : depths) {
        m_bitDepthCombo->addItem(tr("%1-bit color").arg(depth), depth);
        if (depth == targetDepth) {
            m_bitDepthCombo->setCurrentIndex(m_bitDepthCombo->count() - 1);
        }
    }
    m_currentBitDepth = targetDepth;
}

void VideoPage::updateResolutionSlider()
{
    m_activeResolutions.clear();
    const VideoAdapterInfo *adapter = currentAdapter();

    if (!adapter) {
        m_resolutionSlider->setRange(0, 0);
        m_resolutionSlider->setEnabled(false);
        m_resolutionValue->setText(tr("N/A"));
        return;
    }

    for (const VideoResolution &mode : adapter->resolutions) {
        if (mode.bitDepth == m_currentBitDepth) {
            m_activeResolutions.push_back(mode);
        }
    }

    if (m_activeResolutions.empty()) {
        m_resolutionSlider->setRange(0, 0);
        m_resolutionSlider->setEnabled(false);
        m_resolutionValue->setText(tr("N/A"));
        return;
    }

    const int maxIndex = static_cast<int>(m_activeResolutions.size()) - 1;

    int selectedIndex = 0;
    const auto matchIt = std::find_if(
        m_activeResolutions.begin(), m_activeResolutions.end(), [&](const VideoResolution &mode) {
            return mode.width == m_settings.width && mode.height == m_settings.height;
        });
    if (matchIt != m_activeResolutions.end()) {
        selectedIndex = static_cast<int>(std::distance(m_activeResolutions.begin(), matchIt));
    } else if (m_settings.width > 0 && m_settings.height > 0) {
        int bestError = std::numeric_limits<int>::max();
        for (int i = 0; i <= maxIndex; ++i) {
            const VideoResolution &mode = m_activeResolutions[i];
            const int error = std::abs(mode.width - m_settings.width) + std::abs(mode.height - m_settings.height);
            if (error < bestError) {
                bestError = error;
                selectedIndex = i;
            }
        }
    }

    m_resolutionSlider->setEnabled(true);
    m_blockSignals = true;
    m_resolutionSlider->setRange(0, maxIndex);
    m_resolutionSlider->setValue(selectedIndex);
    m_blockSignals = false;
    updateResolutionLabel();
}

void VideoPage::updateResolutionLabel()
{
    if (m_activeResolutions.empty()) {
        m_resolutionValue->setText(tr("N/A"));
        return;
    }

    const int index = std::clamp(
        m_resolutionSlider->value(), 0, static_cast<int>(m_activeResolutions.size()) - 1);
    const VideoResolution &mode = m_activeResolutions[index];
    m_resolutionValue->setText(tr("%1 x %2").arg(mode.width).arg(mode.height));
}

void VideoPage::applySelection()
{
    if (m_activeResolutions.empty()) {
        return;
    }

    const VideoAdapterInfo *adapter = currentAdapter();
    if (!adapter) {
        return;
    }

    const int index = std::clamp(
        m_resolutionSlider->value(), 0, static_cast<int>(m_activeResolutions.size()) - 1);
    const VideoResolution &mode = m_activeResolutions[index];

    m_settings.deviceName = adapter->deviceName;
    m_settings.width = mode.width;
    m_settings.height = mode.height;
    m_settings.bitDepth = mode.bitDepth;
    m_settings.windowed = m_windowedCheck->isChecked();

    if (m_settings.textureDepth <= 0) {
        m_settings.textureDepth = mode.bitDepth;
    }
    m_textureDepthLabel->setText(tr("Texture depth: %1-bit").arg(m_settings.textureDepth));

}

void VideoPage::updateControlStates()
{
    const bool hasAdapters = !m_adapters.empty();
    m_driverList->setEnabled(hasAdapters);
    m_bitDepthCombo->setEnabled(hasAdapters && m_bitDepthCombo->count() > 0);
    m_windowedCheck->setEnabled(hasAdapters);
    m_resolutionSlider->setEnabled(hasAdapters && !m_activeResolutions.empty());
}

const VideoAdapterInfo *VideoPage::currentAdapter() const
{
    if (m_adapters.empty()) {
        return nullptr;
    }

    const int row = m_driverList->currentRow();
    if (row < 0 || row >= static_cast<int>(m_adapters.size())) {
        return nullptr;
    }
    return &m_adapters[row];
}
