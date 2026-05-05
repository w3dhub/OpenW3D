#pragma once

#include <QMainWindow>

class QTabWidget;
class WWConfigBackend;
class PerformancePage;
class VideoPage;
class AudioPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(WWConfigBackend &backend, QWidget *parent = nullptr);

private:
    void setupUi();
    void saveChanges();
    void updateStatusText();
    void refreshTabs();

    WWConfigBackend &m_backend;
    QTabWidget *m_tabWidget = nullptr;
    PerformancePage *m_performancePage = nullptr;
    VideoPage *m_videoPage = nullptr;
    AudioPage *m_audioPage = nullptr;
};
