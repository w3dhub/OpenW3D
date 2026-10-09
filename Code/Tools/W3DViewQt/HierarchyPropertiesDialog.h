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
