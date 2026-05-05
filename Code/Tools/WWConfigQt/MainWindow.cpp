#include "MainWindow.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "WWConfigBackend.h"
#include "PerformancePage.h"
#include "VideoPage.h"
#include "AudioPage.h"
#include "../WWConfig/wwconfig_ids.h"

MainWindow::MainWindow(WWConfigBackend &backend, QWidget *parent)
    : QMainWindow(parent),
      m_backend(backend)
{
    setupUi();
    updateStatusText();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *banner = new QLabel(central);
    banner->setFrameShape(QFrame::Panel);
    banner->setFrameShadow(QFrame::Sunken);
    banner->setAlignment(Qt::AlignCenter);
    banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPalette bannerPalette = banner->palette();
    bannerPalette.setColor(QPalette::Window, Qt::black);
    bannerPalette.setColor(QPalette::WindowText, Qt::white);
    banner->setAutoFillBackground(true);
    banner->setPalette(bannerPalette);
    QPixmap logo(QStringLiteral(":/wwconfig/logo.bmp"));
    if (!logo.isNull()) {
        banner->setPixmap(logo);
        banner->setScaledContents(false);
        banner->setMinimumHeight(logo.height());
    } else {
        banner->setText(tr("Renegade Config"));
        banner->setMinimumHeight(40);
    }
    layout->addWidget(banner);

    m_tabWidget = new QTabWidget(central);
    m_tabWidget->setDocumentMode(true);
    layout->addWidget(m_tabWidget);

    m_videoPage = new VideoPage(m_backend, m_tabWidget);
    m_tabWidget->addTab(m_videoPage, tr("Video"));

    m_audioPage = new AudioPage(m_backend, m_tabWidget);
    m_tabWidget->addTab(m_audioPage, tr("Audio"));

    m_performancePage = new PerformancePage(m_backend, m_tabWidget);
    m_tabWidget->addTab(m_performancePage, tr("Performance"));

    auto *buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    auto *okButton = new QPushButton(tr("OK"), central);
    auto *cancelButton = new QPushButton(tr("Cancel"), central);
    buttonRow->addWidget(okButton);
    buttonRow->addWidget(cancelButton);
    layout->addLayout(buttonRow);

    connect(okButton, &QPushButton::clicked, this, [this]() {
        saveChanges();
        close();
    });
    connect(cancelButton, &QPushButton::clicked, this, &QWidget::close);

    setCentralWidget(central);
    const QSize windowSize(420, 480);
    resize(windowSize);
    setMinimumSize(windowSize);
    setWindowTitle(tr("Renegade Config"));
    setWindowIcon(QIcon(QStringLiteral(":/wwconfig/wwconfig.ico")));
}

void MainWindow::saveChanges()
{
    if (m_videoPage) {
        m_videoPage->save();
    }
    if (m_audioPage) {
        m_audioPage->save();
    }
    if (m_performancePage) {
        m_performancePage->save();
    }
}

void MainWindow::updateStatusText()
{
    const QString localizedTitle = m_backend.localizedString(IDS_WWCONFIG_TITLE);
    if (!localizedTitle.isEmpty()) {
        setWindowTitle(localizedTitle);
    }

    refreshTabs();
}

void MainWindow::refreshTabs()
{
    if (m_performancePage) {
        m_performancePage->refresh();
    }
    if (m_videoPage) {
        m_videoPage->refresh();
    }
    if (m_audioPage) {
        m_audioPage->refresh();
    }
}
