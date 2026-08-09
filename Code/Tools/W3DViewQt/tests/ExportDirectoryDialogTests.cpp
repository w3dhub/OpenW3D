#include "ExportDirectoryDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QTest>

namespace
{
QLineEdit *lineEdit(ExportDirectoryDialog &dialog, const char *name)
{
    QLineEdit *edit = dialog.findChild<QLineEdit *>(name);
    if (!edit) {
        QTest::qFail("Expected line edit was not created from the Designer form",
                     __FILE__,
                     __LINE__);
    }
    return edit;
}

QPushButton *dialogButton(ExportDirectoryDialog &dialog,
                          QDialogButtonBox::StandardButton standardButton)
{
    QDialogButtonBox *buttonBox = dialog.findChild<QDialogButtonBox *>("buttonBox");
    if (!buttonBox) {
        QTest::qFail("buttonBox was not created from the Designer form", __FILE__, __LINE__);
        return nullptr;
    }

    QPushButton *button = buttonBox->button(standardButton);
    if (!button) {
        QTest::qFail("Expected standard dialog button was not created", __FILE__, __LINE__);
    }
    return button;
}
}

class ExportDirectoryDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void filenameIsExactAndReadOnly();
    void selectedPathJoinsDirectoryAndExactFilename();
    void invalidDirectoryDisablesOkAndBlocksAcceptance();
    void cancelButtonRejectsDialog();
};

void ExportDirectoryDialogTests::filenameIsExactAndReadOnly()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString exactFilename = QStringLiteral("XG_IonC_Shock0.w3d");
    ExportDirectoryDialog dialog(exactFilename, temporaryDirectory.path());

    QLineEdit *filenameEdit = lineEdit(dialog, "filenameEdit");
    QVERIFY(filenameEdit);
    QCOMPARE(filenameEdit->text(), exactFilename);
    QVERIFY(filenameEdit->isReadOnly());

    QLineEdit *directoryEdit = lineEdit(dialog, "directoryEdit");
    QVERIFY(directoryEdit);
    QVERIFY(!directoryEdit->isReadOnly());
}

void ExportDirectoryDialogTests::selectedPathJoinsDirectoryAndExactFilename()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString exactFilename = QStringLiteral("C_NOD_SK_.w3d");
    ExportDirectoryDialog dialog(exactFilename, temporaryDirectory.path());
    QVERIFY(QDir(temporaryDirectory.path()).mkdir("chosen-directory"));
    const QString chosenDirectory =
        QDir(temporaryDirectory.path()).filePath("chosen-directory");
    QLineEdit *directoryEdit = lineEdit(dialog, "directoryEdit");
    QVERIFY(directoryEdit);
    directoryEdit->setText(chosenDirectory);

    QCOMPARE(dialog.selectedPath(),
             QDir(chosenDirectory).filePath(exactFilename));
}

void ExportDirectoryDialogTests::invalidDirectoryDisablesOkAndBlocksAcceptance()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    ExportDirectoryDialog dialog(QStringLiteral("asset.w3d"), temporaryDirectory.path());
    QPushButton *okButton = dialogButton(dialog, QDialogButtonBox::Ok);
    QVERIFY(okButton);
    QVERIFY(okButton->isEnabled());

    QLineEdit *directoryEdit = lineEdit(dialog, "directoryEdit");
    QVERIFY(directoryEdit);
    directoryEdit->setText(QDir(temporaryDirectory.path()).filePath("missing-directory"));
    QVERIFY(!okButton->isEnabled());

    QSignalSpy acceptedSpy(&dialog, &QDialog::accepted);
    dialog.accept();
    QCOMPARE(acceptedSpy.count(), 0);
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));

    directoryEdit->setText(temporaryDirectory.path());
    QVERIFY(okButton->isEnabled());
}

void ExportDirectoryDialogTests::cancelButtonRejectsDialog()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    ExportDirectoryDialog dialog(QStringLiteral("asset.w3d"), temporaryDirectory.path());
    QSignalSpy rejectedSpy(&dialog, &QDialog::rejected);
    QPushButton *cancelButton = dialogButton(dialog, QDialogButtonBox::Cancel);
    QVERIFY(cancelButton);

    cancelButton->click();

    QCOMPARE(rejectedSpy.count(), 1);
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
}

QTEST_MAIN(ExportDirectoryDialogTests)

#include "ExportDirectoryDialogTests.moc"
