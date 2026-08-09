#include "ShortcutHelpers.h"

#include <QAction>
#include <QKeySequence>
#include <QWidget>

namespace qtcommon {

QAction *CreateWindowShortcutAction(QWidget *window, const QList<QKeySequence> &shortcuts)
{
    if (!window || shortcuts.isEmpty()) {
        return nullptr;
    }

    auto *action = new QAction(window);
    action->setShortcuts(shortcuts);
    action->setShortcutContext(Qt::WindowShortcut);
    window->addAction(action);
    return action;
}

} // namespace qtcommon
