#pragma once

#include <QDialog>
#include <QVector>

namespace Ui {
class ResolutionDialog;
}

class ResolutionDialog final : public QDialog
{
    Q_OBJECT

public:
    struct Mode {
        Mode() = default;
        Mode(int modeWidth, int modeHeight, int modeBitsPerPixel)
            : width(modeWidth)
            , height(modeHeight)
            , bitsPerPixel(modeBitsPerPixel)
        {
        }

        bool isValid() const
        {
            return width > 0 && height > 0 && bitsPerPixel > 0;
        }

        bool operator==(const Mode &other) const
        {
            return width == other.width && height == other.height &&
                   bitsPerPixel == other.bitsPerPixel;
        }

        int width = 0;
        int height = 0;
        int bitsPerPixel = 0;
    };

    explicit ResolutionDialog(const Mode &preferredMode,
                              bool borderlessFullscreen,
                              QWidget *parent = nullptr);
    ResolutionDialog(const QVector<Mode> &availableModes,
                     const Mode &currentMode,
                     const Mode &preferredMode,
                     bool borderlessFullscreen,
                     QWidget *parent = nullptr);
    ~ResolutionDialog() override;

    int selectedWidth() const;
    int selectedHeight() const;
    int selectedBitsPerPixel() const;
    bool fullscreen() const;

private slots:
    void onDoubleClicked(int row, int column);

private:
    void initialize(const QVector<Mode> &availableModes, bool borderlessFullscreen);
    void selectDefaultRow();

    Ui::ResolutionDialog *_ui = nullptr;
    Mode _currentMode;
    Mode _preferredMode;
};
