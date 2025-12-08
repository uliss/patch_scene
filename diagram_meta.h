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
#ifndef DIAGRAM_META_H
#define DIAGRAM_META_H

#include <QDate>
#include <QJsonValue>
#include <QString>

namespace ceam {
class Contact {
    QString name_, work_, phone_, email_;

public:
    const QString& name() const { return name_; }
    const QString& work() const { return work_; }
    const QString& phone() const { return phone_; }
    const QString& email() const { return email_; }

    void setName(const QString& name) { name_ = name; }
    void setWork(const QString& work) { work_ = work; }
    void setPhone(const QString& phone) { phone_ = phone; }
    void setEmail(const QString& email) { email_ = email; }

    QJsonValue toJson() const;
    static std::optional<Contact> fromJson(const QJsonValue& val);
};

class DiagramMeta {
    QString title_, info_;
    QDate event_date_, creation_date_;
    QList<Contact> contacts_;

public:
    DiagramMeta();

    const QString& title() const { return title_; }
    void setTitle(const QString& title) { title_ = title; }

    const QString& info() const { return info_; }
    void setInfo(const QString& info) { info_ = info; }

    const QDate& eventDate() const { return event_date_; }
    void setEventDate(const QDate& date) { event_date_ = date; }

    const QDate& creationDate() const { return creation_date_; }

    const QList<Contact>& contacts() const { return contacts_; }
    QList<Contact>& contacts() { return contacts_; }

    QJsonValue toJson() const;
    static std::optional<DiagramMeta> fromJson(const QJsonValue& val);
};
} // namespace ceam

#endif // DIAGRAM_META_H
