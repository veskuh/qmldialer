/*
 * Copyright (c) 2011, Tom Swindell.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "common.h"
#include "dbusdialeradapter.h"
#include "managerproxy.h"

DBusDialerAdapter::DBusDialerAdapter(DialerApplication *application)
    : QDBusAbstractAdaptor(application)
{
    TRACE
}

DBusDialerAdapter::~DBusDialerAdapter()
{
    TRACE
}

void DBusDialerAdapter::accept()
{
    TRACE
}

void DBusDialerAdapter::call(const QString &msisdn)
{
    TRACE
    if(!ManagerProxy::instance()->callManager()) return;
    ManagerProxy::instance()->callManager()->dial(msisdn);
    DialerApplication::instance()->setPrestarted(false);
}
