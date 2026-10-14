#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class TexturePathDialog;
}

class TexturePathDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit TexturePathDialog(const QString &path1, const QString &path2, QWidget *parent = nullptr);
    ~TexturePathDialog() override;

    QString path1() const;
    QString path2() const;

private slots:
    void browsePath1();
    void browsePath2();

private:
    Ui::TexturePathDialog *_ui = nullptr;
};
