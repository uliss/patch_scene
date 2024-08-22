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
#ifndef MACOS_UTILS_H
#define MACOS_UTILS_H

#include <QMutex>
#include <QObject>

namespace ceam {
namespace macos {

    void hideApplication();
    void showApplication();

    class NativeAlertDialog : public QObject {
        Q_OBJECT
    public:
        NativeAlertDialog(QObject* parent = nullptr);

        /**
         * creates local loop event cycle and waits for signal
         */
        long waitForAnswer();

    public slots:
        void execDeferred(const QString& message, const QString& info = {});

    private slots:
        void execNow();

    signals:
        void alertDone(long);

    private:
        QString message_, info_;
        QMutex mtx_;
        int rc_;
    };
}
}

#endif // MACOS_UTILS_H
