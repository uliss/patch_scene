/*****************************************************************************
 * Copyright 2024 Serge Poltavski. All rights reserved.
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
#include "test_segment_points.h"
#include "connection_data.h"

#include <QTest>

using namespace ceam;

void TestSegmentPoints::testInit()
{
    SegmentPoints segs;
    QCOMPARE(segs.size(), 0);
    QVERIFY(segs.isEmpty());
}

void TestSegmentPoints::testMakePointList()
{
    {
        SegmentPoints segs;
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 1);
        QCOMPARE(x[0], QPoint(0, 0));
    }

    {
        SegmentPoints segs;
        segs.append({ 50, 100 });
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 5);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(50, 100));
        QCOMPARE(x[3], QPoint(100, 100));
        QCOMPARE(x[4], QPoint(100, 200));
    }

    {
        SegmentPoints segs;
        segs.append({ 50, 100 });
        auto x = segs.makePointList({}, { 100, 200 }, nullptr);
        QCOMPARE(x.size(), 5);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(50, 100));
        QCOMPARE(x[3], QPoint(100, 100));
        QCOMPARE(x[4], QPoint(100, 200));
    }

    {
        QList<int> idxs;
        SegmentPoints segs;
        segs.append({ 50, 100 });
        auto x = segs.makePointList({}, { 100, 200 }, &idxs);
        QCOMPARE(x.size(), 5);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(50, 100));
        QCOMPARE(x[3], QPoint(100, 100));
        QCOMPARE(x[4], QPoint(100, 200));

        // F
        // |
        // .______o____.
        //             |
        //             T

        QCOMPARE(idxs.size(), 3);
        QCOMPARE(idxs[0], -1);
        QCOMPARE(idxs[1], 0);
        QCOMPARE(idxs[2], 0);
    }

    {
        QList<int> idxs;
        SegmentPoints segs;
        segs.append({ 100, 100 });
        auto x = segs.makePointList({}, { 100, 200 }, &idxs);
        QCOMPARE(x.size(), 4);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(100, 100));
        QCOMPARE(x[3], QPoint(100, 200));

        // F
        // |
        // .______o
        //        |
        //        T

        QCOMPARE(idxs.size(), 3);
        QCOMPARE(idxs[0], -1);
        QCOMPARE(idxs[1], 0);
        QCOMPARE(idxs[2], 0);
    }

    {
        QList<int> idxs;
        SegmentPoints segs;
        segs.append({ 1000, 100 });
        auto x = segs.makePointList({}, { 100, 200 }, &idxs);
        QCOMPARE(x.size(), 6);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(1000, 100));
        QCOMPARE(x[3], QPoint(1000, 120));
        QCOMPARE(x[4], QPoint(100, 120));
        QCOMPARE(x[5], QPoint(100, 200));

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T

        QCOMPARE(idxs.size(), 3);
        QCOMPARE(idxs[0], -1);
        QCOMPARE(idxs[1], 0);
        QCOMPARE(idxs[2], 0);
    }

    {
        QList<int> idxs;
        SegmentPoints segs;
        segs.append({ 100, 100 });
        segs.append({ 100, -100 });
        auto x = segs.makePointList({ 0, 0 }, { 200, 0 }, &idxs);
        QCOMPARE(x.size(), 6);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(100, 100));
        QCOMPARE(x[3], QPoint(100, -100));
        QCOMPARE(x[4], QPoint(200, -100));
        QCOMPARE(x[5], QPoint(200, 0));

        //        o_____.
        //        |     |
        // F      |     T
        // |      |
        // .______o
        //

        QCOMPARE(idxs.size(), 4);
        QCOMPARE(idxs[0], -1);
        QCOMPARE(idxs[1], 0);
        QCOMPARE(idxs[2], 0);
        QCOMPARE(idxs[3], 1);
    }

    {
        QList<int> idxs;
        SegmentPoints segs;
        segs.append({ 100, 100 });
        segs.append({ 50, -100 });
        auto x = segs.makePointList({ 0, 0 }, { 200, 100 }, &idxs);
        QCOMPARE(x.size(), 8);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(100, 100));
        QCOMPARE(x[3], QPoint(100, -100));
        QCOMPARE(x[4], QPoint(50, -100));
        QCOMPARE(x[5], QPoint(50, -80));
        QCOMPARE(x[6], QPoint(200, -80));
        QCOMPARE(x[7], QPoint(200, 100));

        //     o__.
        //     |  |
        // F   .__|_____.
        // |      |     |
        // .______o     T
        //

        QCOMPARE(idxs.size(), 5);
        QCOMPARE(idxs[0], -1);
        QCOMPARE(idxs[1], 0);
        QCOMPARE(idxs[2], 0);
        QCOMPARE(idxs[3], 1);
        QCOMPARE(idxs[4], 1);
    }

    {
        QList<int> idxs;
        SegmentPoints segs;
        segs.append({ 100, 100 });
        segs.append({ 250, -100 });
        auto x = segs.makePointList({ 0, 0 }, { 200, 100 }, &idxs);
        QCOMPARE(x.size(), 8);
        QCOMPARE(x[0], QPoint(0, 0));
        QCOMPARE(x[1], QPoint(0, 100));
        QCOMPARE(x[2], QPoint(100, 100));
        QCOMPARE(x[3], QPoint(100, -100));
        QCOMPARE(x[4], QPoint(250, -100));
        QCOMPARE(x[5], QPoint(250, -80));
        QCOMPARE(x[6], QPoint(200, -80));
        QCOMPARE(x[7], QPoint(200, 100));

        //        .__________o
        //        |          |
        // F      |     .____.
        // |      |     |
        // .______o     T
        //

        QCOMPARE(idxs.size(), 5);
        QCOMPARE(idxs[0], -1);
        QCOMPARE(idxs[1], 0);
        QCOMPARE(idxs[2], 0);
        QCOMPARE(idxs[3], 1);
        QCOMPARE(idxs[4], 1);
    }
}

void TestSegmentPoints::testSplitAt()
{
    {
        SegmentPoints segs;
        QVERIFY(!segs.splitAt({}, {}, { 100, 200 }));
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });

        // F
        // |
        // .______o
        //        |
        //        T

        QVERIFY(segs.splitAt({ 50, 100 }, {}, { 100, 200 }));

        // F
        // |
        // .__x___o
        //        |
        //        T
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 5);
        QCOMPARE(segs.pointAt(0), QPoint(50, 100));
        QCOMPARE(segs.pointAt(1), QPoint(100, 100));
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });

        // F
        // |
        // .______o
        //        |
        //        T

        QVERIFY(segs.splitAt({ 0, 50 }, {}, { 100, 200 }));

        // F
        // |
        // x
        // |
        // .______o
        //        |
        //        T
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 5);
        QCOMPARE(segs.pointAt(0), QPoint(0, 50));
        QCOMPARE(segs.pointAt(1), QPoint(100, 100));
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });

        // F
        // |
        // .______o
        //        |
        //        T

        QVERIFY(segs.splitAt({ 100, 150 }, {}, { 100, 200 }));

        // F
        // |
        // .______o
        //        |
        //        x
        //        |
        //        T
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 5);
        QCOMPARE(segs.pointAt(0), QPoint(100, 100));
        QCOMPARE(segs.pointAt(1), QPoint(100, 150));
    }

    {
        SegmentPoints segs;
        segs.append({ 1000, 100 });

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T

        QVERIFY(segs.splitAt({ 0, 50 }, {}, { 100, 200 }));
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 7);
        QCOMPARE(segs.pointAt(0), QPoint(0, 50));
        QCOMPARE(segs.pointAt(1), QPoint(1000, 100));

        // F
        // |
        // x
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T
    }

    {
        SegmentPoints segs;
        segs.append({ 1000, 100 });

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T

        QVERIFY(segs.splitAt({ 100, 100 }, {}, { 100, 200 }));
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 7);
        QCOMPARE(segs.pointAt(0), QPoint(100, 100));
        QCOMPARE(segs.pointAt(1), QPoint(1000, 100));

        // F
        // |
        // .__x___o
        //        |
        //    .___.
        //    |
        //    T
    }

    {
        SegmentPoints segs;
        segs.append({ 1000, 100 });

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T

        QVERIFY(segs.splitAt({ 1000, 110 }, {}, { 100, 200 }));
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 6);
        QCOMPARE(segs.pointAt(0), QPoint(1000, 100));
        QCOMPARE(segs.pointAt(1), QPoint(1000, 110));

        // F
        // |
        // .______o
        //    .___x
        //    |
        //    T
    }

    {
        SegmentPoints segs;
        segs.append({ 1000, 100 });

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T

        QVERIFY(segs.splitAt({ 500, 120 }, {}, { 100, 200 }));
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 7);
        QCOMPARE(segs.pointAt(0), QPoint(1000, 100));
        QCOMPARE(segs.pointAt(1), QPoint(500, 120));

        // F
        // |
        // .______o
        //        |
        //    ._x_.
        //    |
        //    T
    }

    {
        SegmentPoints segs;
        segs.append({ 1000, 100 });

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T

        QVERIFY(segs.splitAt({ 100, 180 }, {}, { 100, 200 }));
        QCOMPARE(segs.size(), 2);
        QCOMPARE(segs.makePointList({}, { 100, 200 }).size(), 6);
        QCOMPARE(segs.pointAt(0), QPoint(1000, 100));
        QCOMPARE(segs.pointAt(1), QPoint(100, 180));

        // F
        // |
        // .______o
        //        |
        //        |
        //    x___.
        //    T
    }
}
