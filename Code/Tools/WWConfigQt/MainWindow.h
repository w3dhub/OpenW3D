#pragma once

#include <QMainWindow>

class QTabWidget;
class WWConfigBackend;
class PerformancePage;
class VideoPage;
class AudioPage;
namespace Ui {
class WWConfigMainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(WWConfigBackend &backend, QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    bool saveChanges();
    void updateStatusText();
    void refreshTabs();

    Ui::WWConfigMainWindow *m_ui = nullptr;
    WWConfigBackend &m_backend;
    QTabWidget *m_tabWidget = nullptr;
    PerformancePage *m_performancePage = nullptr;
    VideoPage *m_videoPage = nullptr;
    AudioPage *m_audioPage = nullptr;
};
