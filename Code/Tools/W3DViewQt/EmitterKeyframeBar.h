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

#include <QVector>
#include <QWidget>

// Designer-promoted view of an emitter channel. Index zero is the fixed
// starting value. The dialog owns the data and handles editing requests.
class EmitterKeyframeBar final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool opacityMode READ opacityMode WRITE setOpacityMode)
    Q_PROPERTY(bool scalarMode READ scalarMode WRITE setScalarMode)

public:
    struct Key {
        double time = 0.0;
        double red = 1.0; // Also the unscaled value for scalar/opacity channels.
        double green = 1.0;
        double blue = 1.0;
    };

    explicit EmitterKeyframeBar(QWidget *parent = nullptr);
    bool opacityMode() const { return _opacityMode; }
    void setOpacityMode(bool opacity);
    bool scalarMode() const { return _scalarMode; }
    void setScalarMode(bool scalar);
    QPair<double, double> valueRange() const;
    void setKeys(const QVector<Key> &keys, double duration);
    const QVector<Key> &keys() const { return _keys; }
    int selectedKey() const { return _selected; }
    void setSelectedKey(int index);
    double duration() const { return _duration; }
    Key interpolatedKey(double time) const;
    QPoint keyPosition(int index) const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void keySelected(int index);
    void keyMoved(int index, double time);
    void keyInserted(double time);
    void keyRemoved(int index);
    void editRequested(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    QRectF gradientRect() const;
    int keyAt(const QPointF &position) const;
    double timeAt(double x) const;
    void moveKey(double time);
    QVector<Key> orderedKeys() const;

    QVector<Key> _keys;
    double _duration = 1.0;
    int _selected = -1;
    bool _opacityMode = false;
    bool _scalarMode = false;
    bool _dragging = false;
};
