#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTableView>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>

namespace
{
template<class T>
T *Control(QObject &owner, const char *name)
{
    auto *control = owner.findChild<T *>(QString::fromLatin1(name));
    if (!control) {
        qFatal("Missing control: %s", name);
    }
    return control;
}
}

class MainWindowTests : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_files;
    QString m_validPath;
    QString m_invalidPath;

private slots:
    void initTestCase()
    {
        QVERIFY(m_files.isValid());
        QCoreApplication::setOrganizationName(QStringLiteral("OpenW3DTests"));
        QCoreApplication::setApplicationName(QStringLiteral("WDumpDesigner"));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_files.path());
        m_validPath = m_files.filePath(QStringLiteral("textures.w3d"));
        QFile file(m_validPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        for (const QByteArray name : {QByteArray("shared-first.tga"), QByteArray("shared-second.tga")}) {
            const QByteArray payload = name + '\0';
            stream << quint32(0x32) << quint32(payload.size());
            QCOMPARE(stream.writeRawData(payload.constData(), static_cast<int>(payload.size())), static_cast<int>(payload.size()));
        }
        QCOMPARE(stream.status(), QDataStream::Ok);
        file.close();

        m_invalidPath = m_files.filePath(QStringLiteral("invalid.w3d"));
        QFile invalid(m_invalidPath);
        QVERIFY(invalid.open(QIODevice::WriteOnly));
        QDataStream broken(&invalid);
        broken.setByteOrder(QDataStream::LittleEndian);
        broken << quint32(0x32) << quint32(4096);
        QCOMPARE(broken.status(), QDataStream::Ok);
    }

    void init()
    {
        QSettings settings;
        settings.clear();
        settings.sync();
    }

    void loadSelectAndReopenRecent()
    {
        MainWindow window;
        QVERIFY(!window.windowIcon().isNull());
        QVERIFY(window.loadFile(m_validPath));
        auto *tree = Control<QTreeView>(window, "treeView");
        auto *table = Control<QTableView>(window, "tableView");
        auto *hex = Control<QPlainTextEdit>(window, "hexView");
        QCOMPARE(tree->model()->rowCount(), 2);
        tree->setCurrentIndex(tree->model()->index(0, 0));
        QVERIFY(table->model()->rowCount() > 0);
        QVERIFY(hex->toPlainText().contains(QStringLiteral("shared-first.tga")));
        QVERIFY(hex->isReadOnly());
        QVERIFY(!(tree->model()->flags(tree->currentIndex()) & Qt::ItemIsEditable));
        for (int row = 0; row < table->model()->rowCount(); ++row) {
            for (int column = 0; column < table->model()->columnCount(); ++column) {
                QVERIFY(!(table->model()->flags(table->model()->index(row, column)) & Qt::ItemIsEditable));
            }
        }
        auto *recent = Control<QMenu>(window, "recentMenu");
        QCOMPARE(recent->actions().size(), 1);
        QCOMPARE(recent->actions().first()->data().toString(), m_validPath);
        recent->actions().first()->trigger();
        QCOMPARE(tree->model()->rowCount(), 2);
        QVERIFY(window.windowTitle().contains(QStringLiteral("textures.w3d")));
    }

    void failedLoadPreservesDocument()
    {
        MainWindow window;
        QVERIFY(window.loadFile(m_validPath));
        auto *tree = Control<QTreeView>(window, "treeView");
        auto *hex = Control<QPlainTextEdit>(window, "hexView");
        tree->setCurrentIndex(tree->model()->index(0, 0));
        const QString title = window.windowTitle();
        bool sawError = false;
        QTimer::singleShot(0, [&sawError]() {
            auto *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (message) {
                sawError = true;
                message->accept();
            }
        });
        QVERIFY(!window.loadFile(m_invalidPath));
        QVERIFY(sawError);
        QCOMPARE(window.windowTitle(), title);
        QCOMPARE(tree->model()->rowCount(), 2);
        tree->setCurrentIndex(tree->model()->index(1, 0));
        auto *fields = Control<QTableView>(window, "tableView")->model();
        QVERIFY(!fields->match(fields->index(0, 2), Qt::DisplayRole,
                               QStringLiteral("shared-second.tga"), 1, Qt::MatchContains).isEmpty());
        QVERIFY(hex->toPlainText().contains(QStringLiteral("shared-second")));
    }

    void findActionsAdvanceAndWrap()
    {
        MainWindow window;
        QVERIFY(window.loadFile(m_validPath));
        auto *tree = Control<QTreeView>(window, "treeView");
        tree->setCurrentIndex(tree->model()->index(0, 0));
        QTimer::singleShot(0, []() {
            auto *dialog = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
            if (dialog) {
                dialog->setTextValue(QStringLiteral("shared"));
                dialog->accept();
            }
        });
        Control<QAction>(window, "actionFind")->trigger();
        QCOMPARE(tree->currentIndex().row(), 1);
        Control<QAction>(window, "actionFindNext")->trigger();
        QCOMPARE(tree->currentIndex().row(), 0);
    }

    void viewActionsAndLayouts()
    {
        MainWindow window;
        QVERIFY(window.loadFile(m_validPath));
        auto *tree = Control<QTreeView>(window, "treeView");
        tree->setCurrentIndex(tree->model()->index(0, 0));
        window.show();
        QApplication::processEvents();
        auto *toolbar = Control<QToolBar>(window, "mainToolBar");
        auto *toolbarAction = Control<QAction>(window, "actionToolbar");
        QVERIFY(!toolbar->isVisible());
        toolbarAction->trigger();
        QVERIFY(toolbar->isVisible());
        toolbar->hide();
        QVERIFY(!toolbarAction->isChecked());
        Control<QAction>(window, "actionStatusBar")->trigger();
        QVERIFY(!window.statusBar()->isVisible());
        Control<QAction>(window, "actionStatusBar")->trigger();
        QVERIFY(window.statusBar()->isVisible());
        QVERIFY(!Control<QAction>(window, "actionOpen")->icon().isNull());
        QVERIFY(Control<QAction>(window, "actionOpen")->shortcuts().contains(QKeySequence(QKeySequence::Open)));

        const QString screenshotDir = qEnvironmentVariable("OPENW3D_QT_SCREENSHOT_DIR");
        if (!screenshotDir.isEmpty()) {
            QVERIFY(QDir().mkpath(screenshotDir));
        }
        for (const QSize size : {window.minimumSize(), QSize(1024, 768)}) {
            window.resize(size);
            Control<QAction>(window, "actionSplit")->trigger();
            QApplication::processEvents();
            for (QSplitter *split : window.findChildren<QSplitter *>()) {
                const QList<int> sizes = split->sizes();
                QCOMPARE(sizes.size(), 2);
                QVERIFY(sizes[0] > 0);
                QVERIFY(sizes[1] > 0);
            }
            if (!screenshotDir.isEmpty()) {
                QVERIFY(window.grab().save(screenshotDir + QStringLiteral("/wdump-%1.png").arg(size.width())));
            }
        }
    }
};

QTEST_MAIN(MainWindowTests)
#include "MainWindowTests.moc"
