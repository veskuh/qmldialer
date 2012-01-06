/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef HISTORYPROXY_H
#define HISTORYPROXY_H

#include "history_interface.h"
#include <QtDBus>
#include <QDebug>

#define OFONO_SERVICE "org.ofono"
#define OFONO_HISTORY_PATH "/"

class HistoryProxy: public org::ofono::CallHistory
{
    Q_OBJECT
    Q_PROPERTY(QSettings* cache READ cache)

public:
    virtual ~HistoryProxy();
    static HistoryProxy *instance();

public slots:
    QSettings* cache() const;

Q_SIGNALS:
    void historyChanged(QStringList ids);

private slots:
    void sendMissedCallNotification(QList<CallHistoryEvent> missed);
    void getHistoryFinished(QDBusPendingCallWatcher *call);
    void voiceHistoryChanged(uint count);
    void initCache();

protected:
    HistoryProxy(const QString &service=OFONO_SERVICE,
                 const QString &path=OFONO_HISTORY_PATH,
                 const QDBusConnection &connection=QDBusConnection::systemBus(),
                 QObject *parent = 0);

private:
    HistoryProxy(const HistoryProxy&);
    HistoryProxy& operator= (const HistoryProxy&);
    QString resolveName(QString &lineId);

    QSettings         *m_cache;

    static HistoryProxy *gHistory;
};

#endif
