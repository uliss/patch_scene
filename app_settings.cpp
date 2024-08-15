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
#include "app_settings.h"

#include <QMainWindow>
#include <QSettings>

constexpr const char* SETTINGS_ORG = "space.ceam";
constexpr const char* SETTINGS_APP = "PatchScene";

constexpr const char* SKEY_MAINWINDOW = "mainwindow";
constexpr const char* SKEY_FAVORITES = "favorites";
constexpr const char* SKEY_GEOMETRY = "geometry";
constexpr const char* SKEY_SAVESTATE = "savestate";
constexpr const char* SKEY_MAXIMIZED = "maximized";
constexpr const char* SKEY_POS = "pos";
constexpr const char* SKEY_SIZE = "size";
constexpr const char* SKEY_ITEMS = "items";

AppSettings::AppSettings() { }

QList<QVariant> AppSettings::readFavorites() const
{
    QSettings qs(SETTINGS_ORG, SETTINGS_APP);

    qs.beginGroup(SKEY_FAVORITES);
    return qs.value(SKEY_ITEMS).toList();
}

void AppSettings::writeFavorites(const QVariant& data) const
{
    QSettings qs(SETTINGS_ORG, SETTINGS_APP);

    qs.beginGroup(SKEY_FAVORITES);
    qs.setValue(SKEY_ITEMS, data);
    qs.endGroup();
}

void AppSettings::readWindowPos(QMainWindow* win) const
{
    if (!win)
        return;

    QSettings qs(SETTINGS_ORG, SETTINGS_APP);

    qs.beginGroup(SKEY_MAINWINDOW);

    win->restoreGeometry(qs.value(SKEY_GEOMETRY, win->saveGeometry()).toByteArray());
    win->restoreState(qs.value(SKEY_SAVESTATE, win->saveState()).toByteArray());
    win->move(qs.value(SKEY_POS, win->pos()).toPoint());
    win->resize(qs.value(SKEY_SIZE, win->size()).toSize());
    if (qs.value(SKEY_MAXIMIZED, win->isMaximized()).toBool())
        win->showMaximized();

    qs.endGroup();
}

void AppSettings::writeWindowPos(const QMainWindow* win) const
{
    if (!win)
        return;

    QSettings qs(SETTINGS_ORG, SETTINGS_APP);

    qs.beginGroup(SKEY_MAINWINDOW);

    qs.setValue(SKEY_GEOMETRY, win->saveGeometry());
    qs.setValue(SKEY_SAVESTATE, win->saveState());
    qs.setValue(SKEY_MAXIMIZED, win->isMaximized());
    if (!win->isMaximized()) {
        qs.setValue(SKEY_POS, win->pos());
        qs.setValue(SKEY_MAXIMIZED, win->size());
    }

    qs.endGroup();
}
