#include "KeyframeTableUtils.h"

#include <QDoubleSpinBox>
#include <QHeaderView>
#include <algorithm>

namespace {
QDoubleSpinBox *MakeSpin(double min, double max, int decimals, double value, QWidget *parent)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setRange(min, max);
    spin->setDecimals(decimals);
    spin->setValue(value);
    return spin;
}

void ClearTable(QTableWidget *table)
{
    if (!table) {
        return;
    }
    table->setRowCount(0);
}
}

QTableWidget *CreateKeyframeTable(const QStringList &headers,
                                  const QVector<KeyframeColumnSpec> &specs,
                                  QWidget *parent)
{
    auto *table = new QTableWidget(parent);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(false);
    table->setShowGrid(true);
    table->setRowCount(0);

    return table;
}

void SetKeyframeRows(QTableWidget *table,
                     const QVector<QVector<double>> &rows,
                     const QVector<KeyframeColumnSpec> &specs)
{
    if (!table) {
        return;
    }

    ClearTable(table);
    for (const QVector<double> &values : rows) {
        AddKeyframeRow(table, values, specs);
    }
}

QVector<QVector<double>> GetKeyframeRows(const QTableWidget *table)
{
    QVector<QVector<double>> rows;
    if (!table) {
        return rows;
    }

    const int row_count = table->rowCount();
    const int col_count = table->columnCount();
    rows.reserve(row_count);
    for (int row = 0; row < row_count; ++row) {
        QVector<double> values;
        values.reserve(col_count);
        for (int col = 0; col < col_count; ++col) {
            const QWidget *widget = table->cellWidget(row, col);
            const auto *spin = qobject_cast<const QDoubleSpinBox *>(widget);
            values.push_back(spin ? spin->value() : 0.0);
        }
        rows.push_back(values);
    }

    return rows;
}

void AddKeyframeRow(QTableWidget *table,
                    const QVector<double> &values,
                    const QVector<KeyframeColumnSpec> &specs)
{
    if (!table) {
        return;
    }

    const int row = table->rowCount();
    table->insertRow(row);

    const int col_count = table->columnCount();
    for (int col = 0; col < col_count; ++col) {
        const KeyframeColumnSpec spec = col < specs.size() ? specs[col] : KeyframeColumnSpec{};
        const double value = col < values.size() ? values[col] : 0.0;
        auto *spin = MakeSpin(spec.min, spec.max, spec.decimals, value, table);
        table->setCellWidget(row, col, spin);
    }
}

void RemoveSelectedKeyframeRows(QTableWidget *table)
{
    if (!table) {
        return;
    }

    const QModelIndexList selected = table->selectionModel()->selectedRows();
    QVector<int> rows;
    rows.reserve(selected.size());
    for (const QModelIndex &index : selected) {
        rows.push_back(index.row());
    }
    if (rows.isEmpty()) {
        const int current = table->currentRow();
        if (current >= 0) {
            rows.push_back(current);
        }
    }

    std::sort(rows.begin(), rows.end(), [](int a, int b) { return a > b; });
    for (int row : rows) {
        table->removeRow(row);
    }
}

void SortKeyframeRows(QTableWidget *table,
                      const QVector<KeyframeColumnSpec> &specs)
{
    if (!table || table->columnCount() == 0) {
        return;
    }

    QVector<QVector<double>> rows = GetKeyframeRows(table);
    std::sort(rows.begin(), rows.end(), [](const QVector<double> &a, const QVector<double> &b) {
        const double time_a = a.isEmpty() ? 0.0 : a[0];
        const double time_b = b.isEmpty() ? 0.0 : b[0];
        return time_a < time_b;
    });

    SetKeyframeRows(table, rows, specs);
}
