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
