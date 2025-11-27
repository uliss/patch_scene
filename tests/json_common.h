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
#ifndef JSON_COMMON_H
#define JSON_COMMON_H

#include "device_common.h"

#include <QJsonObject>

QString make_test_filename(const QString& filename);

QJsonObject read_json_file_object(const QString& filename);
QJsonArray read_json_file_array(const QString& filename);
ceam::SharedItemData read_device_json_file(const QString& filename);

bool compare_json_files(const QString& file1, const QString& file2);

#endif // JSON_COMMON_H
