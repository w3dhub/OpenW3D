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
#include "EmitterKeyframeBar.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include <algorithm>
#include <cmath>

namespace {
double unit(double value)
{
    return std::isfinite(value) ? std::clamp(value, 0.0, 1.0) : 0.0;
}

QColor keyColor(const EmitterKeyframeBar::Key &key, bool opacity)
{
    return opacity ? QColor::fromRgbF(1.0f, 1.0f, 1.0f, static_cast<float>(unit(key.red)))
                   : QColor::fromRgbF(static_cast<float>(unit(key.red)),
                                      static_cast<float>(unit(key.green)),
                                      static_cast<float>(unit(key.blue)));
}
}

EmitterKeyframeBar::EmitterKeyframeBar(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAccessibleName(tr("Color keyframe timeline"));
    setAccessibleDescription(tr("Ctrl-click to insert a key. Drag to move. Double-click or Enter to edit. "
                               "Delete removes the selected key. Left and Right select keys; "
                               "Ctrl+Left and Ctrl+Right move them."));
}

void EmitterKeyframeBar::setOpacityMode(bool opacity)
{
    _opacityMode = opacity;
    setAccessibleName(opacity ? tr("Opacity keyframe timeline") : tr("Color keyframe timeline"));
    update();
}

void EmitterKeyframeBar::setScalarMode(bool scalar)
{
    _scalarMode = scalar;
    setAccessibleName(scalar ? tr("Value keyframe timeline") : tr("Color keyframe timeline"));
    updateGeometry();
    update();
}

QPair<double, double> EmitterKeyframeBar::valueRange() const
{
    // Scale only the display, not the stored values. Include zero and keep a
    // useful range even for empty or constant channels.
    double minimum = 0.0;
    double maximum = 0.0;
    for (const Key &key : _keys) {
        if (std::isfinite(key.red)) {
            minimum = std::min(minimum, key.red);
            maximum = std::max(maximum, key.red);
        }
    }
    if (minimum == maximum) maximum = minimum + 1.0;
    return {minimum, maximum};
}

void EmitterKeyframeBar::setKeys(const QVector<Key> &keys, double duration)
{
    _keys = keys;
    _duration = std::isfinite(duration) && duration > 0.0 ? duration : 1.0;
    for (const Key &key : _keys) {
        if (std::isfinite(key.time)) _duration = std::max(_duration, key.time);
    }
    const int selected = _keys.isEmpty() ? -1 : std::clamp(_selected, 0, static_cast<int>(_keys.size()) - 1);
    setSelectedKey(selected);
    update();
}

void EmitterKeyframeBar::setSelectedKey(int index)
{
    if (index < -1 || index >= _keys.size() || index == _selected) return;
    _selected = index;
    update();
    emit keySelected(index);
}

QVector<EmitterKeyframeBar::Key> EmitterKeyframeBar::orderedKeys() const
{
    QVector<Key> result = _keys;
    std::stable_sort(result.begin(), result.end(), [](const Key &a, const Key &b) { return a.time < b.time; });
    return result;
}

EmitterKeyframeBar::Key EmitterKeyframeBar::interpolatedKey(double time) const
{
    const QVector<Key> ordered = orderedKeys();
    Key result;
    result.time = time;
    if (ordered.isEmpty()) return result;
    Key previous = ordered.first();
    for (const Key &next : ordered) {
        if (next.time > time) {
            const double span = next.time - previous.time;
            const double blend = span > 0.0 ? unit((time - previous.time) / span) : 0.0;
            result.red = previous.red + (next.red - previous.red) * blend;
            result.green = previous.green + (next.green - previous.green) * blend;
            result.blue = previous.blue + (next.blue - previous.blue) * blend;
            return result;
        }
        previous = next;
    }
    previous.time = time;
    return previous;
}

QSize EmitterKeyframeBar::sizeHint() const { return QSize(540, minimumSizeHint().height()); }
QSize EmitterKeyframeBar::minimumSizeHint() const { return QSize(260, (_scalarMode ? 180 : 76) + fontMetrics().height()); }

QRectF EmitterKeyframeBar::gradientRect() const
{
    int left = 12;
    if (_scalarMode) {
        const auto range = valueRange();
        left += std::max(fontMetrics().horizontalAdvance(QString::number(range.first, 'g', 5)),
                         fontMetrics().horizontalAdvance(QString::number(range.second, 'g', 5)));
    }
    return QRectF(left, 6.0, std::max(1, width() - left - 12), std::max(20, height() - fontMetrics().height() - 35));
}

QPoint EmitterKeyframeBar::keyPosition(int index) const
{
    if (index < 0 || index >= _keys.size()) return QPoint();
    const QRectF bar = gradientRect();
    return QPoint(qRound(bar.left() + unit(_keys[index].time / _duration) * bar.width()),
                  qRound(bar.bottom() + 8.0));
}

double EmitterKeyframeBar::timeAt(double x) const
{
    const QRectF bar = gradientRect();
    return unit((x - bar.left()) / bar.width()) * _duration;
}

int EmitterKeyframeBar::keyAt(const QPointF &position) const
{
    if (position.y() < gradientRect().top() || position.y() > gradientRect().bottom() + 18.0) return -1;
    int closest = -1;
    double distance = 9.0;
    for (int index = 0; index < _keys.size(); ++index) {
        const double next = std::abs(position.x() - keyPosition(index).x());
        if (next < distance || (next == distance && index == _selected)) {
            closest = index;
            distance = next;
        }
    }
    return closest;
}

void EmitterKeyframeBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QRectF bar = gradientRect();
    if (_opacityMode && !_scalarMode) {
        painter.save();
        painter.setClipRect(bar);
        for (int y = static_cast<int>(bar.top()); y < bar.bottom(); y += 8) {
            for (int x = static_cast<int>(bar.left()); x < bar.right(); x += 8) {
                const bool dark = ((x - static_cast<int>(bar.left())) / 8 + (y - static_cast<int>(bar.top())) / 8) % 2 != 0;
                painter.fillRect(QRect(x, y, 8, 8), dark ? QColor(150, 150, 150) : QColor(210, 210, 210));
            }
        }
        painter.restore();
    }
    if (_scalarMode) {
        painter.fillRect(bar, palette().brush(QPalette::Base));
        const auto range = valueRange();
        const auto point = [&](const Key &key) {
            return QPointF(bar.left() + unit(key.time / _duration) * bar.width(),
                           bar.bottom() - unit((key.red - range.first) / (range.second - range.first)) * bar.height());
        };
        painter.setPen(QPen(palette().color(QPalette::Mid), 1.0, Qt::DotLine));
        for (int step = 1; step < 4; ++step) {
            const double x = bar.left() + bar.width() * step / 4.0;
            const double y = bar.top() + bar.height() * step / 4.0;
            painter.drawLine(QPointF(x, bar.top()), QPointF(x, bar.bottom()));
            painter.drawLine(QPointF(bar.left(), y), QPointF(bar.right(), y));
        }
        if (range.first < 0.0 && range.second > 0.0) {
            const double zero = point({0.0, 0.0}).y();
            painter.setPen(palette().color(QPalette::Mid));
            painter.drawLine(QPointF(bar.left(), zero), QPointF(bar.right(), zero));
        }
        painter.setPen(palette().color(QPalette::Text));
        const QRectF labels(0, bar.top(), bar.left() - 6, bar.height());
        painter.drawText(labels, Qt::AlignRight | Qt::AlignTop, QString::number(range.second, 'g', 5));
        painter.drawText(labels, Qt::AlignRight | Qt::AlignBottom, QString::number(range.first, 'g', 5));
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath curve;
        curve.moveTo(point(interpolatedKey(0.0)));
        for (const Key &key : orderedKeys()) curve.lineTo(point(key));
        curve.lineTo(point(interpolatedKey(_duration)));
        painter.setPen(QPen(palette().color(QPalette::Highlight), 2.0));
        painter.drawPath(curve);
        for (int index = 0; index < _keys.size(); ++index) {
            const QPointF position = point(_keys[index]);
            painter.setBrush(index == _selected ? palette().brush(QPalette::Highlight) : palette().brush(QPalette::Base));
            painter.drawEllipse(position, index == _selected ? 5.0 : 3.5, index == _selected ? 5.0 : 3.5);
        }
    } else {
        QLinearGradient gradient(bar.topLeft(), bar.topRight());
        gradient.setColorAt(0.0, keyColor(interpolatedKey(0.0), _opacityMode));
        for (const Key &key : orderedKeys()) {
            gradient.setColorAt(unit(key.time / _duration), keyColor(key, _opacityMode));
        }
        gradient.setColorAt(1.0, keyColor(interpolatedKey(_duration), _opacityMode));
        painter.fillRect(bar, gradient);
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(palette().color(QPalette::Mid));
    painter.drawRect(bar);

    painter.setRenderHint(QPainter::Antialiasing);
    for (int index = 0; index < _keys.size(); ++index) {
        const QPoint center = keyPosition(index);
        QPainterPath marker;
        marker.moveTo(center.x(), bar.bottom() + 1.0);
        marker.lineTo(center.x() - 7.0, bar.bottom() + 13.0);
        marker.lineTo(center.x() + 7.0, bar.bottom() + 13.0);
        marker.closeSubpath();
        const bool selected = index == _selected;
        painter.setPen(QPen(selected ? palette().color(QPalette::Highlight) : palette().color(QPalette::Text), selected ? 3.0 : 1.0));
        const Key &key = _keys[index];
        painter.setBrush(_scalarMode ? palette().color(selected ? QPalette::Highlight : QPalette::Base)
            : _opacityMode ? QColor::fromRgbF(static_cast<float>(unit(key.red)), static_cast<float>(unit(key.red)), static_cast<float>(unit(key.red))) : keyColor(key, false));
        painter.drawPath(marker);
    }
    painter.setPen(palette().color(QPalette::Text));
    const QRect labelRect(qRound(bar.left()), height() - fontMetrics().height(), qRound(bar.width()), fontMetrics().height());
    painter.drawText(labelRect, Qt::AlignLeft, tr("0 s"));
    painter.drawText(labelRect, Qt::AlignRight, tr("%1 s").arg(_duration, 0, 'g', 6));
    if (hasFocus()) {
        painter.setPen(QPen(palette().color(QPalette::Highlight), 1.0, Qt::DotLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect().adjusted(1, 1, -2, -2));
    }
}

void EmitterKeyframeBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) { QWidget::mousePressEvent(event); return; }
    setFocus(Qt::MouseFocusReason);
    const int index = keyAt(event->position());
    if (index >= 0) {
        setSelectedKey(index);
        _dragging = index > 0;
    } else if ((event->modifiers() & Qt::ControlModifier) && gradientRect().contains(event->position())) {
        emit keyInserted(timeAt(event->position().x()));
    }
    event->accept();
}

void EmitterKeyframeBar::moveKey(double time)
{
    if (_selected <= 0 || _selected >= _keys.size()) return;
    // Do not reorder keys under the pointer or let a key cross its neighbours.
    double minimum = 0.0;
    double maximum = _duration;
    const double original = _keys[_selected].time;
    for (int index = 0; index < _keys.size(); ++index) {
        if (index == _selected) continue;
        const double other = _keys[index].time;
        if (other < original || (other == original && index < _selected)) minimum = std::max(minimum, other + 0.000001);
        else maximum = std::min(maximum, other - 0.000001);
    }
    if (minimum > maximum) return;
    const double next = std::clamp(time, minimum, maximum);
    if (next != original) emit keyMoved(_selected, next);
}

void EmitterKeyframeBar::mouseMoveEvent(QMouseEvent *event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton)) {
        moveKey(timeAt(event->position().x()));
    } else {
        const int index = keyAt(event->position());
        setCursor(index > 0 ? Qt::SizeHorCursor : Qt::ArrowCursor);
        if (index >= 0) {
            const Key &key = _keys[index];
            const QString value = _scalarMode ? tr("Value: %1").arg(key.red, 0, 'g', 6)
                : _opacityMode ? tr("Opacity: %1%").arg(key.red * 100.0, 0, 'g', 6)
                : tr("RGB: %1, %2, %3").arg(key.red, 0, 'g', 6).arg(key.green, 0, 'g', 6).arg(key.blue, 0, 'g', 6);
            QToolTip::showText(event->globalPosition().toPoint(), tr("%1 s — %2").arg(key.time, 0, 'g', 6).arg(value), this);
        } else {
            QToolTip::hideText();
        }
    }
    event->accept();
}

void EmitterKeyframeBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (_dragging && event->button() == Qt::LeftButton) moveKey(timeAt(event->position().x()));
    _dragging = false;
    event->accept();
}

void EmitterKeyframeBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    _dragging = false;
    const int index = keyAt(event->position());
    if (event->button() == Qt::LeftButton && index >= 0) {
        setSelectedKey(index);
        emit editRequested(index);
    }
    event->accept();
}

void EmitterKeyframeBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete && _selected > 0) emit keyRemoved(_selected);
    else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) && _selected >= 0) emit editRequested(_selected);
    else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        const int direction = event->key() == Qt::Key_Left ? -1 : 1;
        if (event->modifiers() & Qt::ControlModifier) {
            if (_selected >= 0) moveKey(_keys[_selected].time + direction * _duration / 100.0);
        } else if (!_keys.isEmpty()) {
            setSelectedKey(std::clamp(_selected + direction, 0, static_cast<int>(_keys.size()) - 1));
        }
    } else { QWidget::keyPressEvent(event); return; }
    event->accept();
}

void EmitterKeyframeBar::focusInEvent(QFocusEvent *event) { QWidget::focusInEvent(event); update(); }
void EmitterKeyframeBar::focusOutEvent(QFocusEvent *event) { QWidget::focusOutEvent(event); update(); }
