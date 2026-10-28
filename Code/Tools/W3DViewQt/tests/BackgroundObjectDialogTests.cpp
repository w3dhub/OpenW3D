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

#include "BackgroundObjectDialog.h"

#include "assetmgr.h"
#include "proto.h"
#include "rendobj.h"

#include <QByteArray>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QtTest/QTest>

namespace {
class TestPrototype final : public PrototypeClass
{
public:
    explicit TestPrototype(const QByteArray &name)
        : _name(name)
    {
    }

    const char *Get_Name() const override { return _name.constData(); }
    int Get_Class_ID() const override { return RenderObjClass::CLASSID_HMODEL; }
    RenderObjClass *Create() override { return nullptr; }

private:
    QByteArray _name;
};

void addPrototype(WW3DAssetManager &assetManager, const char *name)
{
    assetManager.Add_Prototype(new TestPrototype(name));
}

QListWidget *objectList(BackgroundObjectDialog &dialog)
{
    auto *list = dialog.findChild<QListWidget *>("listWidget");
    if (!list) {
        QTest::qFail("listWidget was not created from the Designer form", __FILE__, __LINE__);
    }
    return list;
}

QStringList itemTexts(const QListWidget &list)
{
    QStringList texts;
    for (int row = 0; row < list.count(); ++row) {
        texts.push_back(list.item(row)->text());
    }
    return texts;
}
} // namespace

class BackgroundObjectDialogTests final : public QObject
{
    Q_OBJECT

private slots:
    void sortsObjectsAndRestoresCurrentSelection();
    void clearRemovesSelectionAndCurrentObject();
};

void BackgroundObjectDialogTests::sortsObjectsAndRestoresCurrentSelection()
{
    WW3DAssetManager assetManager;
    addPrototype(assetManager, "zulu");
    addPrototype(assetManager, "Charlie");
    addPrototype(assetManager, "alpha");
    addPrototype(assetManager, "Bravo");

    BackgroundObjectDialog dialog("Charlie");
    QListWidget *list = objectList(dialog);
    QVERIFY(list);

    QCOMPARE(itemTexts(*list), QStringList({"alpha", "Bravo", "Charlie", "zulu"}));
    QVERIFY(list->currentItem());
    QCOMPARE(list->currentItem()->text(), QString("Charlie"));
    QVERIFY(list->currentItem()->isSelected());
    QCOMPARE(dialog.selectedName(), QString("Charlie"));
}

void BackgroundObjectDialogTests::clearRemovesSelectionAndCurrentObject()
{
    WW3DAssetManager assetManager;
    addPrototype(assetManager, "ObjectB");
    addPrototype(assetManager, "ObjectA");

    BackgroundObjectDialog dialog("ObjectB");
    QListWidget *list = objectList(dialog);
    QVERIFY(list);
    QCOMPARE(dialog.selectedName(), QString("ObjectB"));

    auto *clearButton = dialog.findChild<QPushButton *>("clearButton");
    QVERIFY(clearButton);
    clearButton->click();

    QVERIFY(list->selectedItems().isEmpty());
    QCOMPARE(list->currentRow(), -1);
    QVERIFY(dialog.selectedName().isEmpty());

    auto *currentLabel = dialog.findChild<QLabel *>("currentLabel");
    QVERIFY(currentLabel);
    QCOMPARE(currentLabel->text(), QString("Current Object: (none)"));
}

QTEST_MAIN(BackgroundObjectDialogTests)

#include "BackgroundObjectDialogTests.moc"
