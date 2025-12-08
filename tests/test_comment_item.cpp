/*****************************************************************************
 * Copyright 2025 Serge Poltavski. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#include "test_comment_item.h"
#include "comment_item.h"
#include "diagram_scene.h"

#include <QSignalSpy>
#include <QTest>

using namespace ceam;

void TestCommentItem::init()
{
    QCOMPARE(CommentItem::Type, SceneItem::Type);
}

void TestCommentItem::setEdited()
{
    DiagramScene ds(100, 100);
    auto item = ds.addSceneItem(ItemData::makeComment("test comment"));
    QVERIFY(item);
    auto comm = dynamic_cast<CommentItem*>(item);
    QVERIFY(comm);
    QCOMPARE(ds.itemCount(), 1);
    auto id = comm->itemData()->id();

    QSignalSpy sig_conn_edit(comm, &CommentItem::editComment);
    QSignalSpy sig_scene_show_comment_editor(&ds.itemScene(), &Scene::showCommentEditor);

    QVERIFY(!comm->isEdited());
    QCOMPARE(sig_conn_edit.count(), 0);

    comm->setEditable(true);
    QCOMPARE(sig_conn_edit.count(), 1);
    QCOMPARE(sig_conn_edit.at(0).at(0).value<SceneItemId>(), id);
    QCOMPARE(sig_scene_show_comment_editor.count(), 1);
    QCOMPARE(sig_scene_show_comment_editor.at(0).at(0).toBool(), true);
    QVERIFY(comm->isEdited());

    comm->setEditable(false);
    QCOMPARE(sig_conn_edit.count(), 2);
    QCOMPARE(sig_conn_edit.at(1).at(0).value<SceneItemId>(), SCENE_ITEM_NULL_ID);
    QCOMPARE(sig_scene_show_comment_editor.count(), 2);
    QCOMPARE(sig_scene_show_comment_editor.at(1).at(0).toBool(), false);
    QVERIFY(!comm->isEdited());

    comm->setEditable(true);
    QCOMPARE(sig_conn_edit.count(), 3);
    QCOMPARE(sig_conn_edit.at(2).at(0).value<SceneItemId>(), id);
    QCOMPARE(sig_scene_show_comment_editor.count(), 3);
    QCOMPARE(sig_scene_show_comment_editor.at(2).at(0).toBool(), true);
    QVERIFY(comm->isEdited());
    ds.itemScene().doneCommentEditor();

    QCOMPARE(sig_conn_edit.count(), 4);
    QCOMPARE(sig_conn_edit.at(3).at(0).value<SceneItemId>(), SCENE_ITEM_NULL_ID);
    QCOMPARE(sig_scene_show_comment_editor.count(), 4);
    QCOMPARE(sig_scene_show_comment_editor.at(3).at(0).toBool(), false);
    QVERIFY(!comm->isEdited());

}
