#pragma once

#include <QList>

class QAction;
class QKeySequence;
class QWidget;

namespace qtcommon {

QAction *CreateWindowShortcutAction(QWidget *window, const QList<QKeySequence> &shortcuts);

} // namespace qtcommon
