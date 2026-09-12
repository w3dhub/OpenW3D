#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include "WWConfigBackend.h"
#include "PerformancePage.h"
#include "VideoPage.h"
#include "AudioPage.h"
#include "../WWConfig/wwconfig_ids.h"

MainWindow::MainWindow(WWConfigBackend &backend, QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::WWConfigMainWindow),
      m_backend(backend)
{
    setupUi();
    updateStatusText();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::setupUi()
{
    m_ui->setupUi(this);
    m_tabWidget = m_ui->tabWidget;

    m_videoPage = new VideoPage(m_backend, m_tabWidget);
    m_ui->videoPageLayout->addWidget(m_videoPage);
    m_audioPage = new AudioPage(m_backend, m_tabWidget);
    m_ui->audioPageLayout->addWidget(m_audioPage);
    m_performancePage = new PerformancePage(m_backend, m_tabWidget);
    m_ui->performancePageLayout->addWidget(m_performancePage);

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (saveChanges()) {
            close();
        }
    });
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close);
}

bool MainWindow::saveChanges()
{
    const bool videoSaved = m_videoPage->save();
    const bool audioSaved = m_audioPage->save();
    const bool performanceSaved = m_performancePage->save();
    if (!videoSaved || !audioSaved || !performanceSaved) {
        QMessageBox::warning(this, tr("Save Settings"),
                             tr("Some settings could not be saved. Check that the configuration location is writable:\n%1")
                                 .arg(m_backend.configPath()));
        return false;
    }
    return true;
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
    m_performancePage->refresh();
    m_videoPage->refresh();
    m_audioPage->refresh();
}
