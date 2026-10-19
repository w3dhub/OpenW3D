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

#include <QTableWidget>
#include <QVector>
#include <QStringList>

struct KeyframeColumnSpec {
    double min = 0.0;
    double max = 1.0;
    int decimals = 2;
};

QTableWidget *CreateKeyframeTable(const QStringList &headers,
                                  const QVector<KeyframeColumnSpec> &specs,
                                  QWidget *parent);
void SetKeyframeRows(QTableWidget *table,
                     const QVector<QVector<double>> &rows,
                     const QVector<KeyframeColumnSpec> &specs);
QVector<QVector<double>> GetKeyframeRows(const QTableWidget *table);
void AddKeyframeRow(QTableWidget *table,
                    const QVector<double> &values,
                    const QVector<KeyframeColumnSpec> &specs);
void RemoveSelectedKeyframeRows(QTableWidget *table);
void SortKeyframeRows(QTableWidget *table,
                      const QVector<KeyframeColumnSpec> &specs);
