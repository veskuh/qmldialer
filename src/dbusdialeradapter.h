/*
 * Copyright (c) 2011, Tom Swindell.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
#ifndef DBUSDIALERADAPTER_H
#define DBUSDIALERADAPTER_H

#include "dialerapplication.h"
#include <QDBusAbstractAdaptor>

class DBusDialerAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meego.dialer")

public:
    explicit DBusDialerAdapter(DialerApplication *application);
            ~DBusDialerAdapter();

Q_SIGNALS:

public Q_SLOTS:
    void accept();
    void call(const QString &msisdn);
};

#endif // DBUSDIALERADAPTER_H
