#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class ExportDirectoryDialog;
}

class ExportDirectoryDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDirectoryDialog(const QString &fixedFilename,
                                   QWidget *parent = nullptr);
    ExportDirectoryDialog(const QString &fixedFilename,
                          const QString &initialDirectory,
                          QWidget *parent = nullptr);
    ~ExportDirectoryDialog() override;

    QString selectedPath() const;

public slots:
    void accept() override;

private slots:
    void browse();
    void updateOkButton();

private:
    QString directory() const;

    Ui::ExportDirectoryDialog *_ui = nullptr;
    QString _fixedFilename;
};
