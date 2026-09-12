#include "TexturePathDialog.h"

#include "ui_TexturePathDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>

TexturePathDialog::TexturePathDialog(const QString &path1, const QString &path2, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::TexturePathDialog)
{
    _ui->setupUi(this);
    _ui->path1LineEdit->setText(QDir::toNativeSeparators(path1));
    _ui->path2LineEdit->setText(QDir::toNativeSeparators(path2));

    connect(_ui->path1BrowseButton, &QPushButton::clicked, this, &TexturePathDialog::browsePath1);
    connect(_ui->path2BrowseButton, &QPushButton::clicked, this, &TexturePathDialog::browsePath2);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

TexturePathDialog::~TexturePathDialog()
{
    delete _ui;
}

QString TexturePathDialog::path1() const
{
    return _ui->path1LineEdit->text().trimmed();
}

QString TexturePathDialog::path2() const
{
    return _ui->path2LineEdit->text().trimmed();
}

void TexturePathDialog::browsePath1()
{
    const QString start = _ui->path1LineEdit->text();
    const QString dir = QFileDialog::getExistingDirectory(this, "Texture Path 1", start);
    if (!dir.isEmpty()) {
        _ui->path1LineEdit->setText(QDir::toNativeSeparators(dir));
    }
}

void TexturePathDialog::browsePath2()
{
    const QString start = _ui->path2LineEdit->text();
    const QString dir = QFileDialog::getExistingDirectory(this, "Texture Path 2", start);
    if (!dir.isEmpty()) {
        _ui->path2LineEdit->setText(QDir::toNativeSeparators(dir));
    }
}
