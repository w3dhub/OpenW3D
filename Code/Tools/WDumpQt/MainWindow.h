/*
**  Command & Conquer Renegade(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>
#include <QMainWindow>
#include <QStringList>
#include <QVector>

#include "wdump_core.h"

class QAction;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QPlainTextEdit;
class QSplitter;
class QStandardItem;
class QTableView;
class QTreeView;
class QStandardItemModel;
class QToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool loadFile(const QString &path);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void buildUi();
    void buildMenus();
    void openFileDialog();
    void openRecentFile();
    void openFindDialog();
    void findNext();
    void showAbout();
    void splitViews();
    void updateRecentFilesMenu();
    void addRecentFile(const QString &path);
    void clearViews();
    QString windowTitleForPath(const QString &path) const;
    void rebuildTree();
    void onTreeSelectionChanged(const QModelIndex &current);
    QStandardItem *addChunkItem(QStandardItem *parent, const wdump::Chunk &chunk);
    void showChunk(const wdump::Chunk *chunk);
    bool chunkMatches(const wdump::Chunk &chunk, const QString &needle) const;
    void collectIndices(const QModelIndex &parent, QVector<QModelIndex> &out) const;

    static constexpr int kMaxRecentFiles = 10;

    wdump::ChunkFile _file;
    QString _currentFile;
    QString _findString;
    QTreeView *_treeView = nullptr;
    QTableView *_tableView = nullptr;
    QPlainTextEdit *_hexView = nullptr;
    QToolBar *_toolbar = nullptr;
    QSplitter *_rightSplit = nullptr;
    QSplitter *_mainSplit = nullptr;
    QStandardItemModel *_treeModel = nullptr;
    QStandardItemModel *_tableModel = nullptr;
    QMenu *_recentMenu = nullptr;
    QAction *_findNextAction = nullptr;
};
