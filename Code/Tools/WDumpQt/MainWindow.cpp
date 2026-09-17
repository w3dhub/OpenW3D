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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "RecentFiles.h"

#include <utility>

#include <QAction>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStatusBar>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyle>
#include <QTableView>
#include <QToolBar>
#include <QTreeView>
#include <QUrl>
#include <QVariant>
#include <QVector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      _ui(new Ui::WDumpMainWindow)
{
    buildUi();
    buildMenus();
    setWindowTitle(windowTitleForPath(QString()));
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::buildUi()
{
    _ui->setupUi(this);
    _treeView = _ui->treeView;
    _tableView = _ui->tableView;
    _hexView = _ui->hexView;
    _toolbar = _ui->mainToolBar;
    _rightSplit = _ui->rightSplitter;
    _mainSplit = _ui->mainSplitter;

    _treeModel = new QStandardItemModel(this);
    _treeModel->setHorizontalHeaderLabels({tr("Chunks")});
    _tableModel = new QStandardItemModel(this);
    _tableModel->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Value")});

    _treeView->setModel(_treeModel);
    connect(_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onTreeSelectionChanged);
    _tableView->setModel(_tableModel);
    _hexView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::buildMenus()
{
    _recentMenu = _ui->recentMenu;
    _findNextAction = _ui->actionFindNext;
    updateRecentFilesMenu();

    _ui->actionOpen->setShortcuts(QKeySequence::Open);
    _ui->actionExit->setShortcuts(QKeySequence::Quit);
    _ui->actionFind->setShortcuts(QKeySequence::Find);
    _ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    _ui->actionAbout->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));

    connect(_ui->actionOpen, &QAction::triggered, this, &MainWindow::openFileDialog);
    connect(_ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(_ui->actionSplit, &QAction::triggered, this, &MainWindow::splitViews);
    connect(_ui->actionFind, &QAction::triggered, this, &MainWindow::openFindDialog);
    connect(_findNextAction, &QAction::triggered, this, &MainWindow::findNext);
    connect(_ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(_ui->actionToolbar, &QAction::toggled, _toolbar, &QWidget::setVisible);
    connect(_toolbar, &QToolBar::visibilityChanged, _ui->actionToolbar, &QAction::setChecked);
    connect(_ui->actionStatusBar, &QAction::toggled, statusBar(), &QWidget::setVisible);
    _toolbar->setVisible(_ui->actionToolbar->isChecked());
}

bool MainWindow::loadFile(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

    const QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        QMessageBox::warning(this, tr("WDump Qt"), tr("File not found:\n%1").arg(path));
        return false;
    }

    wdump::ChunkFile loadedFile;
    if (!loadedFile.load(path.toStdString())) {
        QMessageBox::warning(this, tr("WDump Qt"), tr("Failed to load file:\n%1").arg(path));
        return false;
    }

    clearViews();
    _file = std::move(loadedFile);
    rebuildTree();
    _currentFile = fileInfo.absoluteFilePath();
    setWindowTitle(windowTitleForPath(_currentFile));
    addRecentFile(_currentFile);
    if (statusBar()) {
        statusBar()->showMessage(_currentFile);
    }
    return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    const auto urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            event->acceptProposedAction();
            return;
        }
    }

    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        if (loadFile(url.toLocalFile())) {
            event->acceptProposedAction();
        }
        return;
    }
}

void MainWindow::openFileDialog()
{
    const QString startDir = _currentFile.isEmpty() ? QString() : QFileInfo(_currentFile).absolutePath();
    const QString filter = tr("W3D Files (*.w3d *.wlt *.wht *.wha *.wtm);;All Files (*.*)");
    const QString filename = QFileDialog::getOpenFileName(this, tr("Open W3D File"), startDir, filter);
    if (filename.isEmpty()) {
        return;
    }
    loadFile(filename);
}

void MainWindow::openFindDialog()
{
    bool ok = false;
    const QString text =
        QInputDialog::getText(this, tr("Find String"), tr("Find what:"), QLineEdit::Normal, _findString, &ok);
    if (!ok) {
        return;
    }
    _findString = text;
    if (_findString.isEmpty()) {
        return;
    }
    findNext();
}

void MainWindow::findNext()
{
    if (_findString.isEmpty()) {
        openFindDialog();
        return;
    }

    QVector<QModelIndex> indices;
    collectIndices(QModelIndex(), indices);
    if (indices.isEmpty()) {
        return;
    }

    int startIndex = 0;
    const QModelIndex current = _treeView->currentIndex();
    if (current.isValid()) {
        for (int i = 0; i < indices.size(); ++i) {
            if (indices[i] == current) {
                startIndex = i + 1;
                break;
            }
        }
    }

    auto matchesIndex = [this](const QModelIndex &index) {
        const auto chunkPtr =
            reinterpret_cast<const wdump::Chunk *>(index.data(Qt::UserRole + 1).value<quintptr>());
        return chunkPtr && chunkMatches(*chunkPtr, _findString);
    };

    for (int i = startIndex; i < indices.size(); ++i) {
        if (matchesIndex(indices[i])) {
            _treeView->setCurrentIndex(indices[i]);
            _treeView->scrollTo(indices[i]);
            return;
        }
    }

    for (int i = 0; i < startIndex; ++i) {
        if (matchesIndex(indices[i])) {
            _treeView->setCurrentIndex(indices[i]);
            _treeView->scrollTo(indices[i]);
            return;
        }
    }

    QMessageBox::warning(this, tr("Find String"), tr("Cannot find \"%1\".").arg(_findString));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this,
                       tr("About Westwood 3D File Viewer"),
                       tr("Westwood 3D File Viewer v3.0\n"
                          "Copyright (C) 1997 Westwood Studios\n"
                          "Written by Eric Cosky, Greg Hjelstrom\n"
                          "Qt port by the OpenW3D team"));
}

void MainWindow::splitViews()
{
    if (_mainSplit) {
        _mainSplit->setSizes({1, 2});
    }
    if (_rightSplit) {
        _rightSplit->setSizes({3, 2});
    }
}

void MainWindow::openRecentFile()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }

    const QString path = action->data().toString();
    if (!loadFile(path)) {
        QSettings settings;
        const QStringList files = qtcommon::RemoveRecentFile(qtcommon::ReadRecentFiles(settings), path);
        qtcommon::WriteRecentFiles(settings, files);
        updateRecentFilesMenu();
    }
}

void MainWindow::updateRecentFilesMenu()
{
    if (!_recentMenu) {
        return;
    }

    _recentMenu->clear();
    QSettings settings;
    const auto files = qtcommon::ReadRecentFiles(settings, QStringLiteral("recentFiles"), kMaxRecentFiles);
    if (files.isEmpty()) {
        auto *emptyAction = _recentMenu->addAction(tr("(No recent files)"));
        emptyAction->setEnabled(false);
        return;
    }

    int index = 1;
    for (const QString &path : files) {
        const QFileInfo info(path);
        const QString label = tr("&%1 %2").arg(index++).arg(info.fileName());
        auto *action = _recentMenu->addAction(label, this, &MainWindow::openRecentFile);
        action->setData(path);
        action->setToolTip(path);
    }
}

void MainWindow::addRecentFile(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    QSettings settings;
    const QStringList files = qtcommon::AddRecentFile(
        qtcommon::ReadRecentFiles(settings),
        path,
        kMaxRecentFiles);
    qtcommon::WriteRecentFiles(settings, files, QStringLiteral("recentFiles"), kMaxRecentFiles);
    updateRecentFilesMenu();
}

void MainWindow::clearViews()
{
    _treeModel->removeRows(0, _treeModel->rowCount());
    _tableModel->removeRows(0, _tableModel->rowCount());
    _hexView->clear();
}

QString MainWindow::windowTitleForPath(const QString &path) const
{
    if (path.isEmpty()) {
        return QStringLiteral("WDump");
    }

    return QStringLiteral("WDump - %1").arg(QFileInfo(path).fileName());
}

void MainWindow::rebuildTree()
{
    for (const auto &root : _file.roots())
    {
        addChunkItem(nullptr, *root);
    }
}

QStandardItem *MainWindow::addChunkItem(QStandardItem *parent, const wdump::Chunk &chunk)
{
    const char *name = wdump::chunk_name(chunk.id);
    QString label;
    if (name) {
        label = QString::fromLatin1(name);
    } else {
        const QString idText = QString::number(chunk.id, 16).toUpper();
        label = QStringLiteral("Unknown: id=0x%1").arg(idText);
    }
    auto *item = new QStandardItem(label);
    item->setEditable(false);
    item->setData(QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(&chunk)), Qt::UserRole + 1);

    if (parent)
    {
        parent->appendRow(item);
    }
    else
    {
        _treeModel->appendRow(item);
    }

    for (const auto &child : chunk.children)
    {
        addChunkItem(item, *child);
    }

    return item;
}

void MainWindow::onTreeSelectionChanged(const QModelIndex &current)
{
    const auto chunkPtr = reinterpret_cast<const wdump::Chunk *>(current.data(Qt::UserRole + 1).value<quintptr>());
    showChunk(chunkPtr);
}

void MainWindow::showChunk(const wdump::Chunk *chunk)
{
    _tableModel->removeRows(0, _tableModel->rowCount());
    if (!chunk)
    {
        _hexView->clear();
        return;
    }

    auto addRow = [&](const QString &name, const QString &type, const QString &value) {
        QList<QStandardItem *> row;
        for (const QString &text : {name, type, value}) {
            auto *item = new QStandardItem(text);
            item->setEditable(false);
            row << item;
        }
        _tableModel->appendRow(row);
    };

    const auto fields = wdump::describe_chunk(*chunk);
    for (const auto &field : fields) {
        addRow(QString::fromStdString(field.name),
               QString::fromStdString(field.type),
               QString::fromStdString(field.value));
    }

    _hexView->setPlainText(QString::fromStdString(wdump::build_hex_view(*chunk)));
}

bool MainWindow::chunkMatches(const wdump::Chunk &chunk, const QString &needle) const
{
    if (needle.isEmpty()) {
        return false;
    }

    const auto fields = wdump::describe_chunk(chunk);
    for (const auto &field : fields) {
        if (QString::fromStdString(field.value).contains(needle, Qt::CaseSensitive)) {
            return true;
        }
    }

    return false;
}

void MainWindow::collectIndices(const QModelIndex &parent, QVector<QModelIndex> &out) const
{
    const int rows = _treeModel->rowCount(parent);
    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = _treeModel->index(row, 0, parent);
        out.push_back(index);
        collectIndices(index, out);
    }
}
