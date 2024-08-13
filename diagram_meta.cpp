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
#include "diagram_meta.h"

#include <QJsonArray>
#include <QJsonObject>

constexpr const char* JSON_KEY_NAME = "name";
constexpr const char* JSON_KEY_WORK = "work";
constexpr const char* JSON_KEY_PHONE = "phone";
constexpr const char* JSON_KEY_EMAIL = "email";

constexpr const char* JSON_KEY_TITLE = "title";
constexpr const char* JSON_KEY_INFO = "info";
constexpr const char* JSON_KEY_CREATION_DATE = "creation-date";
constexpr const char* JSON_KEY_EVENT_DATE = "event-date";
constexpr const char* JSON_KEY_CONTACTS = "contacts";

QJsonValue Contact::toJson() const
{
    QJsonObject obj;

    obj[JSON_KEY_NAME] = name_;
    obj[JSON_KEY_WORK] = work_;
    obj[JSON_KEY_PHONE] = phone_;
    obj[JSON_KEY_EMAIL] = email_;

    return obj;
}

std::optional<Contact> Contact::fromJson(const QJsonValue& val)
{
    if (!val.isObject())
        return {};

    auto obj = val.toObject();

    Contact c;

    c.name_ = obj.value(JSON_KEY_NAME).toString();
    c.work_ = obj.value(JSON_KEY_WORK).toString();
    c.phone_ = obj.value(JSON_KEY_PHONE).toString();
    c.email_ = obj.value(JSON_KEY_EMAIL).toString();

    return c;
}

DiagramMeta::DiagramMeta()
{
    creation_date_ = QDate::currentDate();
}

QJsonValue DiagramMeta::toJson() const
{
    QJsonObject obj;

    obj[JSON_KEY_TITLE] = title_;
    obj[JSON_KEY_INFO] = info_;
    obj[JSON_KEY_CREATION_DATE] = creation_date_.toString();
    obj[JSON_KEY_EVENT_DATE] = event_date_.toString();

    QJsonArray arr;
    for (auto& c : contacts_)
        arr.append(c.toJson());

    obj[JSON_KEY_CONTACTS] = arr;

    return obj;
}

std::optional<DiagramMeta> DiagramMeta::fromJson(const QJsonValue& val)
{
    if (!val.isObject())
        return {};

    auto obj = val.toObject();

    DiagramMeta meta;

    meta.title_ = obj.value(JSON_KEY_TITLE).toString();
    meta.info_ = obj.value(JSON_KEY_INFO).toString();
    meta.creation_date_ = QDate::fromString(obj.value(JSON_KEY_CREATION_DATE).toString());
    meta.event_date_ = QDate::fromString(obj.value(JSON_KEY_EVENT_DATE).toString());

    auto arr = obj.value(JSON_KEY_CONTACTS).toArray();
    for (const auto& item : arr) {
        auto contact = Contact::fromJson(item);
        if (contact)
            meta.contacts_.push_back(contact.value());
    }

    return meta;
}
