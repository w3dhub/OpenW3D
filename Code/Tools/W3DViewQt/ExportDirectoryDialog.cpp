#include "ExportDirectoryDialog.h"

#include "ui_ExportDirectoryDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

ExportDirectoryDialog::ExportDirectoryDialog(const QString &fixedFilename, QWidget *parent)
    : ExportDirectoryDialog(fixedFilename, QDir::currentPath(), parent)
{
}

ExportDirectoryDialog::ExportDirectoryDialog(const QString &fixedFilename,
                                               const QString &initialDirectory,
                                               QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::ExportDirectoryDialog)
    , _fixedFilename(fixedFilename)
{
    _ui->setupUi(this);
    _ui->filenameEdit->setText(_fixedFilename);
    _ui->directoryEdit->setText(QDir::toNativeSeparators(initialDirectory));

    connect(_ui->directoryEdit,
            &QLineEdit::textChanged,
            this,
            &ExportDirectoryDialog::updateOkButton);
    connect(_ui->browseButton,
            &QPushButton::clicked,
            this,
            &ExportDirectoryDialog::browse);
    connect(_ui->buttonBox,
            &QDialogButtonBox::accepted,
            this,
            &ExportDirectoryDialog::accept);
    connect(_ui->buttonBox,
            &QDialogButtonBox::rejected,
            this,
            &QDialog::reject);

    updateOkButton();
}

ExportDirectoryDialog::~ExportDirectoryDialog()
{
    delete _ui;
}

QString ExportDirectoryDialog::selectedPath() const
{
    const QString selectedDirectory = directory();
    if (selectedDirectory.isEmpty()) {
        return {};
    }
    return QDir(selectedDirectory).filePath(_fixedFilename);
}

void ExportDirectoryDialog::accept()
{
    if (!directory().isEmpty() && QDir(directory()).exists()) {
        QDialog::accept();
    }
}

void ExportDirectoryDialog::browse()
{
    QString initialDirectory = directory();
    if (!QDir(initialDirectory).exists()) {
        initialDirectory = QDir::currentPath();
    }

    const QString selectedDirectory = QFileDialog::getExistingDirectory(
        this,
        tr("Select Export Directory"),
        initialDirectory,
        QFileDialog::ShowDirsOnly);
    if (!selectedDirectory.isEmpty()) {
        _ui->directoryEdit->setText(QDir::toNativeSeparators(selectedDirectory));
    }
}

void ExportDirectoryDialog::updateOkButton()
{
    QPushButton *okButton = _ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setEnabled(!directory().isEmpty() && QDir(directory()).exists());
    }
}

QString ExportDirectoryDialog::directory() const
{
    return _ui->directoryEdit->text();
}
