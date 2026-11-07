/*
** Copyright 2026 OpenW3D contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed WITHOUT ANY WARRANTY; without even the
** implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "SaveSettingsDialog.h"

#include "ui_SaveSettingsDialog.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>

SaveSettingsDialog::SaveSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::SaveSettingsDialog)
{
    _ui->setupUi(this);

    connect(_ui->browseButton, &QPushButton::clicked, this, &SaveSettingsDialog::browse);
    connect(_ui->pathLineEdit,
            &QLineEdit::textChanged,
            this,
            &SaveSettingsDialog::updateOkEnabled);
    connect(_ui->lightingCheckBox,
            &QCheckBox::toggled,
            this,
            &SaveSettingsDialog::updateOkEnabled);
    connect(_ui->backgroundCheckBox,
            &QCheckBox::toggled,
            this,
            &SaveSettingsDialog::updateOkEnabled);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateOkEnabled();
}

SaveSettingsDialog::~SaveSettingsDialog()
{
    delete _ui;
}

QString SaveSettingsDialog::selectedPath() const
{
    QString path = QDir::fromNativeSeparators(_ui->pathLineEdit->text().trimmed());
    if (path.isEmpty()) {
        return {};
    }

    if (QFileInfo(path).isRelative()) {
        path = QDir(QCoreApplication::applicationDirPath()).filePath(path);
    }
    return QDir::toNativeSeparators(QDir::cleanPath(path));
}

bool SaveSettingsDialog::saveLighting() const
{
    return _ui->lightingCheckBox->isChecked();
}

bool SaveSettingsDialog::saveBackground() const
{
    return _ui->backgroundCheckBox->isChecked();
}

void SaveSettingsDialog::browse()
{
    QString initial_path = selectedPath();
    if (initial_path.isEmpty()) {
        initial_path = QStringLiteral("Default.dat");
    }

    QFileInfo initial_info(initial_path);
    if (initial_info.isRelative()) {
        initial_info.setFile(QDir(QCoreApplication::applicationDirPath()), initial_path);
    }

    QFileDialog dialog(this, tr("Save Settings"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix(QStringLiteral("dat"));
    dialog.setNameFilter(tr("Setting data files (*.dat);;All Files (*.*)"));
    dialog.setDirectory(initial_info.absolutePath());
    dialog.selectFile(initial_info.fileName());

    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return;
    }

    _ui->pathLineEdit->setText(QDir::toNativeSeparators(dialog.selectedFiles().at(0)));
}

void SaveSettingsDialog::updateOkEnabled()
{
    const bool has_path = !selectedPath().isEmpty();
    const bool has_supported_category = saveLighting() || saveBackground();
    if (QPushButton *ok_button = _ui->buttonBox->button(QDialogButtonBox::Ok)) {
        ok_button->setEnabled(has_path && has_supported_category);
    }
}
