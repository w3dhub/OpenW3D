#pragma once

#include <QDialog>

class W3DViewport;

namespace Ui {
class AddToLineupDialog;
}

class AddToLineupDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AddToLineupDialog(W3DViewport *viewport, QWidget *parent = nullptr);
    ~AddToLineupDialog() override;
    QString selectedName() const;

protected:
    void accept() override;

private:
    void populateObjects();

    W3DViewport *_viewport = nullptr;
    Ui::AddToLineupDialog *_ui = nullptr;
};
