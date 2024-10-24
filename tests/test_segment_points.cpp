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
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[0].second, false);
    }

    {
        SegmentPoints segs;
        segs.append({ 50, 100 });
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 5);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(50, 100));
        QCOMPARE(x[3].first, QPoint(100, 100));
        QCOMPARE(x[4].first, QPoint(100, 200));
    }

    {
        SegmentPoints segs;
        segs.append({ 50, 100 });
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 5);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(50, 100));
        QCOMPARE(x[3].first, QPoint(100, 100));
        QCOMPARE(x[4].first, QPoint(100, 200));
    }

    {
        SegmentPoints segs;
        segs.append({ 50, 100 });
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 5);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(50, 100));
        QCOMPARE(x[3].first, QPoint(100, 100));
        QCOMPARE(x[4].first, QPoint(100, 200));

        // F
        // |
        // .______o____.
        //             |
        //             T
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 4);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(100, 100));
        QCOMPARE(x[3].first, QPoint(100, 200));

        // F
        // |
        // .______o
        //        |
        //        T
    }

    {
        SegmentPoints segs;
        segs.append({ 1000, 100 });
        auto x = segs.makePointList({}, { 100, 200 });
        QCOMPARE(x.size(), 6);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(1000, 100));
        QCOMPARE(x[3].first, QPoint(1000, 120));
        QCOMPARE(x[4].first, QPoint(100, 120));
        QCOMPARE(x[5].first, QPoint(100, 200));

        // F
        // |
        // .______o
        //        |
        //    .___.
        //    |
        //    T
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });
        segs.append({ 100, -100 });
        auto x = segs.makePointList({ 0, 0 }, { 200, 0 });
        QCOMPARE(x.size(), 6);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(100, 100));
        QCOMPARE(x[3].first, QPoint(100, -100));
        QCOMPARE(x[4].first, QPoint(200, -100));
        QCOMPARE(x[5].first, QPoint(200, 0));

        //        o_____.
        //        |     |
        // F      |     T
        // |      |
        // .______o
        //
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });
        segs.append({ 50, -100 });
        auto x = segs.makePointList({ 0, 0 }, { 200, 100 });
        QCOMPARE(x.size(), 8);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(100, 100));
        QCOMPARE(x[3].first, QPoint(100, -100));
        QCOMPARE(x[4].first, QPoint(50, -100));
        QCOMPARE(x[5].first, QPoint(50, -80));
        QCOMPARE(x[6].first, QPoint(200, -80));
        QCOMPARE(x[7].first, QPoint(200, 100));

        //     o__.
        //     |  |
        // F   .__|_____.
        // |      |     |
        // .______o     T
        //
    }

    {
        SegmentPoints segs;
        segs.append({ 100, 100 });
        segs.append({ 250, -100 });
        auto x = segs.makePointList({ 0, 0 }, { 200, 100 });
        QCOMPARE(x.size(), 8);
        QCOMPARE(x[0].first, QPoint(0, 0));
        QCOMPARE(x[1].first, QPoint(0, 100));
        QCOMPARE(x[2].first, QPoint(100, 100));
        QCOMPARE(x[3].first, QPoint(100, -100));
        QCOMPARE(x[4].first, QPoint(250, -100));
        QCOMPARE(x[5].first, QPoint(250, -80));
        QCOMPARE(x[6].first, QPoint(200, -80));
        QCOMPARE(x[7].first, QPoint(200, 100));

        //        .__________o
        //        |          |
        // F      |     .____.
        // |      |     |
        // .______o     T
        //
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
        auto pts = segs.makePointList({}, { 100, 200 });
        QCOMPARE(pts.size(), 4);
        QCOMPARE(pts[0].first, QPoint(0, 0));
        QCOMPARE(pts[1].first, QPoint(0, 100));
        QCOMPARE(pts[2].first, QPoint(100, 100));
        QCOMPARE(pts[3].first, QPoint(100, 200));

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

        pts = segs.makePointList({}, { 100, 200 });
        qDebug() << pts;
        QCOMPARE(pts.size(), 5);
        QCOMPARE(pts[0].first, QPoint(0, 0));
        QCOMPARE(pts[1].first, QPoint(0, 100));
        QCOMPARE(pts[2].first, QPoint(50, 100));
        QCOMPARE(pts[3].first, QPoint(100, 100));
        QCOMPARE(pts[4].first, QPoint(100, 200));

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
        segs.append({ 0, 100 });

        // F
        // |
        // o______.
        //        |
        //        T

        QVERIFY(segs.splitAt({ 0, 50 }, {}, { 100, 200 }));
        QVERIFY(segs.splitAt({ 0, 75 }, {}, { 100, 200 }));

        // F
        // |
        // x
        // x
        // o______.
        //        |
        //        T
        QCOMPARE(segs.size(), 3);
        auto pts = segs.makePointList({}, { 100, 200 });

        QCOMPARE(pts.size(), 6);
        QCOMPARE(pts[0].first, QPoint(0, 0));
        QCOMPARE(pts[1].first, QPoint(0, 50));
        QCOMPARE(pts[2].first, QPoint(0, 75));
        QCOMPARE(pts[3].first, QPoint(0, 100));
        QCOMPARE(pts[4].first, QPoint(100, 100));
        QCOMPARE(pts[5].first, QPoint(100, 200));
        QCOMPARE(segs.pointAt(0), QPoint(0, 50));
        QCOMPARE(segs.pointAt(1), QPoint(0, 75));
        QCOMPARE(segs.pointAt(2), QPoint(0, 100));
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
