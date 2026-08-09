#include "ResolutionDialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QMetaObject>
#include <QTableWidget>
#include <QtTest/QTest>

namespace {
QTableWidget *resolutionTable(ResolutionDialog &dialog)
{
    auto *table = dialog.findChild<QTableWidget *>("resolutionTable");
    if (!table) {
        QTest::qFail("resolutionTable was not created from the Designer form", __FILE__, __LINE__);
    }
    return table;
}
} // namespace

class ResolutionDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void ordersDeduplicatesAndSelectsStoredPreference();
    void fallsBackToCurrentModeAndUsesLiveWindowState();
    void doubleClickSelectsModeAndAccepts();
};

void ResolutionDialogTests::ordersDeduplicatesAndSelectsStoredPreference()
{
    const QVector<ResolutionDialog::Mode> modes = {
        {1920, 1080, 32},
        {800, 600, 16},
        {1280, 720, 32},
        {1920, 1080, 32},
        {0, 768, 32},
        {1024, 768, 32},
        {800, 600, 32},
    };
    ResolutionDialog dialog(modes,
                            ResolutionDialog::Mode(1280, 720, 32),
                            ResolutionDialog::Mode(1920, 1080, 32),
                            true);

    QTableWidget *table = resolutionTable(dialog);
    QVERIFY(table);
    QCOMPARE(table->rowCount(), 4);
    QCOMPARE(table->item(0, 0)->text(), QString("800 x 600"));
    QCOMPARE(table->item(1, 0)->text(), QString("1024 x 768"));
    QCOMPARE(table->item(2, 0)->text(), QString("1280 x 720"));
    QCOMPARE(table->item(3, 0)->text(), QString("1920 x 1080"));
    for (int row = 0; row < table->rowCount(); ++row) {
        QVERIFY(table->item(row, 1)->text().startsWith("32 bpp"));
    }

    QCOMPARE(dialog.selectedWidth(), 1920);
    QCOMPARE(dialog.selectedHeight(), 1080);
    QCOMPARE(dialog.selectedBitsPerPixel(), 32);

    auto *fullscreen = dialog.findChild<QCheckBox *>("fullscreenCheck");
    QVERIFY(fullscreen);
    QVERIFY(fullscreen->isChecked());
    QCOMPARE(fullscreen->text(), QString("&Borderless fullscreen"));

    auto *hint = dialog.findChild<QLabel *>("hintLabel");
    QVERIFY(hint);
    QVERIFY(hint->text().contains("viewport follows the window size"));
}

void ResolutionDialogTests::fallsBackToCurrentModeAndUsesLiveWindowState()
{
    const QVector<ResolutionDialog::Mode> modes = {
        {1920, 1080, 32},
        {1280, 720, 32},
    };
    ResolutionDialog dialog(modes,
                            ResolutionDialog::Mode(1280, 720, 32),
                            ResolutionDialog::Mode(1600, 900, 32),
                            false);

    QCOMPARE(dialog.selectedWidth(), 1280);
    QCOMPARE(dialog.selectedHeight(), 720);
    QCOMPARE(dialog.selectedBitsPerPixel(), 32);

    auto *fullscreen = dialog.findChild<QCheckBox *>("fullscreenCheck");
    QVERIFY(fullscreen);
    QVERIFY(!fullscreen->isChecked());
}

void ResolutionDialogTests::doubleClickSelectsModeAndAccepts()
{
    const QVector<ResolutionDialog::Mode> modes = {
        {800, 600, 32},
        {1920, 1080, 32},
    };
    ResolutionDialog dialog(modes,
                            ResolutionDialog::Mode(800, 600, 32),
                            ResolutionDialog::Mode(800, 600, 32),
                            false);

    QVERIFY(QMetaObject::invokeMethod(&dialog,
                                      "onDoubleClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(int, 1),
                                      Q_ARG(int, 0)));
    QCOMPARE(dialog.result(), int(QDialog::Accepted));
    QCOMPARE(dialog.selectedWidth(), 1920);
    QCOMPARE(dialog.selectedHeight(), 1080);
    QCOMPARE(dialog.selectedBitsPerPixel(), 32);
}

QTEST_MAIN(ResolutionDialogTests)

#include "ResolutionDialogTests.moc"
