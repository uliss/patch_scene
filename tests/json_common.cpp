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
#include "json_common.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>

using namespace ceam;

QJsonObject read_json_file_object(const QString& filename)
{
    auto path = QDir(TEST_DATA_DIR).filePath(QFileInfo(filename).fileName());
    QFile f(path);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "can't open file:" << path;
        return {};
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (doc.isNull())
        qWarning() << "json parse error:" << err.errorString();

    return doc.object();
}

SharedDeviceData read_device_json_file(const QString& filename)
{
    auto obj = read_json_file_object(filename);
    SharedDeviceData data(new DeviceData(DEV_NULL_ID));
    if (!data->setJson(obj))
        qWarning() << "can't set device JSON from file:";

    return data;
}
