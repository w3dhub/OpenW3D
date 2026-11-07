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
