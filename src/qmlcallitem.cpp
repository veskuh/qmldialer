/*
 * dialer - Declarative Dialer UX Main Window.
 * Copyright (c) 2011, Tom Swindell.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "common.h"
#include "qmlcallitem.h"

class QMLCallItemPrivate
{
public:
    CallItem *proxy;
};

QMLCallItem::QMLCallItem(CallItem *proxy, QObject *parent)
    : QObject(parent), d(new QMLCallItemPrivate)
{
    TRACE
    d->proxy = proxy;
    QObject::connect(proxy->callProxy(), SIGNAL(stateChanged()), SLOT(onStateChanged()));
}

QMLCallItem::~QMLCallItem()
{
    TRACE
    delete this->d;
}

CallItem* QMLCallItem::proxy() const
{
    TRACE
    return d->proxy;
}

QString QMLCallItem::msisdn() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->lineID();
    return QString();
}

QString QMLCallItem::name() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->name();
    return QString();
}

QString QMLCallItem::state() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->state();
    return QString();
}

QString QMLCallItem::path() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->path();
    return QString();
}

QString QMLCallItem::reason() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->reason();
    return QString();
}

QDateTime QMLCallItem::startedAt() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->startTime();
    return QDateTime();
}

int QMLCallItem::duration() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->duration();
    return 0;
}

bool QMLCallItem::isMultiparty() const
{
    TRACE
    if (d->proxy->callProxy())
        return d->proxy->callProxy()->multiparty();
    return false;
}

void QMLCallItem::answer()
{
    TRACE
    if (d->proxy->callProxy())
        d->proxy->callProxy()->answer();
}

void QMLCallItem::deflect(const QString &msisdn)
{
    TRACE
    if (d->proxy->callProxy())
        d->proxy->callProxy()->deflect(msisdn);
}

void QMLCallItem::hangup()
{
    TRACE
    if (d->proxy->callProxy())
        d->proxy->callProxy()->hangup();
}

void QMLCallItem::onStateChanged()
{
    TRACE
    emit this->stateChanged(d->proxy->callProxy()->state());
}
