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
#include "macos_utils.h"

#include <AppKit/AppKit.h>
#include <QEventLoop>
#include <QMessageBox>

void ceam::macos::hideApplication()
{
    [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyProhibited];
}

void ceam::macos::showApplication()
{
    NSApplication* applicationInstance = [NSApplication sharedApplication];

    [applicationInstance setActivationPolicy:NSApplicationActivationPolicyRegular];
    [applicationInstance activateIgnoringOtherApps:YES];
}

ceam::macos::NativeAlertDialog::NativeAlertDialog(QObject* parent)
    : QObject(parent)
{
}

long ceam::macos::NativeAlertDialog::waitForAnswer()
{
    QEventLoop loop;
    connect(
        this, &NativeAlertDialog::alertDone, &loop, [&loop, this](long rc) {
            {
                QMutexLocker<QMutex> lock(&mtx_);
                rc_ = rc;
            }
            loop.exit();
        },
        Qt::QueuedConnection);
    loop.exec();

    switch (rc_) {
    case NSAlertThirdButtonReturn:
        return QMessageBox::Cancel;
    case NSAlertSecondButtonReturn:
        return QMessageBox::No;
    case NSAlertFirstButtonReturn:
    default:
        return QMessageBox::Yes;
    }
}

void ceam::macos::NativeAlertDialog::execDeferred(const QString& message, const QString& info)
{
    {
        QMutexLocker<QMutex> lock(&mtx_);
        message_ = message;
        info_ = info;
        rc_ = QMessageBox::NoButton;
    }

    QMetaObject::invokeMethod(this, "execNow", Qt::QueuedConnection);
}

void ceam::macos::NativeAlertDialog::execNow()
{
    auto txt_yes = tr("Yes");
    auto txt_no = tr("No");
    auto txt_cancel = tr("Cancel");

    QMutexLocker<QMutex> lock(&mtx_);

    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:message_.toNSString()];
    [alert setInformativeText:info_.toNSString()];
    [alert addButtonWithTitle:txt_yes.toNSString()];
    [alert addButtonWithTitle:txt_no.toNSString()];
    [alert addButtonWithTitle:txt_cancel.toNSString()];
    [alert setAlertStyle:NSAlertStyleCritical];
    auto res = [alert runModal];
    emit alertDone(res);
}
