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
#ifndef DEVICE_ITEM_H
#define DEVICE_ITEM_H

#include "scene_item.h"

namespace ceam {

class DeviceItem : public SceneItem {
public:
    DeviceItem();
    explicit DeviceItem(const SharedDeviceData& data);
};

} // namespace ceam

#endif // DEVICE_ITEM_H
