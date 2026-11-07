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

#pragma once

#include <QDialog>
#include <QString>

class QTreeWidgetItem;

namespace Ui {
class HierarchyPropertiesDialog;
}

class HierarchyPropertiesDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit HierarchyPropertiesDialog(const QString &hierarchyName, QWidget *parent = nullptr);
    ~HierarchyPropertiesDialog() override;

private slots:
    void showSubObjectProperties(QTreeWidgetItem *item, int column);

private:
    void setErrorState(const QString &message);

    Ui::HierarchyPropertiesDialog *_ui = nullptr;
    QString _hierarchyName;
};
