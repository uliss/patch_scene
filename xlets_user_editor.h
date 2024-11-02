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
#ifndef XLETS_USER_EDITOR_H
#define XLETS_USER_EDITOR_H

#include "device_common.h"
#include "device_xlet_view.h"

#include <QDialog>
#include <QGraphicsScene>

namespace Ui {
class XletsUserEditor;
}

namespace ceam {

class XletsUserEditor : public QDialog {
    Q_OBJECT

public:
    explicit XletsUserEditor(QWidget* parent, const SharedDeviceData& data);
    ~XletsUserEditor();

private:
    void initInlets();
    void initOutlets();

private:
    Ui::XletsUserEditor* ui;
    SharedDeviceData data_;
    DeviceXlets inlets_, outlets_;
    QGraphicsScene in_scene_, out_scene_, view_scene_;
};
}

#endif // XLETS_USER_EDITOR_H
