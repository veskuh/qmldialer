/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "historyproxy.h"
#include "dialerapplication.h"
#include <MNotification>
#include <QDebug>

/*
 * Voice Call History ofono plugin manager class implimentation
 */

HistoryProxy *HistoryProxy::gHistory = 0;

HistoryProxy::HistoryProxy(const QString &service,
                           const QString &path,
                           const QDBusConnection &connection,
                           QObject *parent)
    : org::ofono::CallHistory(service, path, connection, parent),
      m_cache(0)
{
    TRACE
    if (gHistory)
        qFatal("HistoryProxy: There can be only one!");

    if (!isValid()) {
        qWarning() << "HistoryProxy: Failed to connect to Ofono: \n\t"
                   << lastError().message();
    } else {
        initCache();

        connect(this, SIGNAL(VoiceHistoryChanged(uint)),
                this, SLOT(voiceHistoryChanged(uint)));
    }
    gHistory = this;
}

HistoryProxy::~HistoryProxy()
{
    TRACE
    m_cache->sync();

    gHistory=0;
}

HistoryProxy *HistoryProxy::instance()
{
    TRACE
    if (!gHistory)
        gHistory = new HistoryProxy();
    return gHistory;
}

void HistoryProxy::sendMissedCallNotification(QList<CallHistoryEvent> missed)
{
    TRACE
    foreach (CallHistoryEvent e, missed) {
        QString name;
        QString photo  = DEFAULT_AVATAR_ICON;
        //% "Private"
        QString lineid = qtTrId("xx_private");
        //% "Missed call"
        QString summary(qtTrId("xx_missed_call"));
        QString body;
        MNotification notice(NOTIFICATION_CALL_EVENT);

        if (!e.lineid.isEmpty()) {
            lineid = stripLineID(e.lineid);
            SeasideSyncModel *contacts = DA_SEASIDEMODEL;
            QModelIndex first = contacts->index(0,Seaside::ColumnPhoneNumbers);
            QModelIndexList matches = contacts->match(first, Seaside::SearchRole,
                                                      QVariant(lineid),-1);
            if (!matches.isEmpty()) {
                QModelIndex person = matches.at(0); //First match wins
                SEASIDE_SHORTCUTS
                SEASIDE_SET_MODEL_AND_ROW(person.model(), person.row());

                QString firstName = SEASIDE_FIELD(FirstName, String);
                QString lastName = SEASIDE_FIELD(LastName, String);

                if (lastName.isEmpty())
                    // Contacts first (common) name
                    //% "%1"
                    name = qtTrId("xx_first_name").arg(firstName);
                else
                    name = firstName+ " " + lastName;

                photo = SEASIDE_FIELD(Avatar, String);
            }
        } else {
            //% "Unavailable"
            lineid = qtTrId("xx_unavailable");
        }

        //% "You missed a call from %1"
        body = QString(qtTrId("xx_missing_body"))
                              .arg(name.isEmpty()?lineid:name);

        notice.setSummary(summary);
        notice.setBody(body);
        notice.setImage(photo);
        notice.publish();

        qDebug() << QString("%1: %2").arg(summary).arg(body);
    }
}


QString HistoryProxy::resolveName(QString &lineId) {
    QString name = "";

    SeasideSyncModel *contacts = DA_SEASIDEMODEL;
    QModelIndex first = contacts->index(0,Seaside::ColumnPhoneNumbers);
    QModelIndexList matches = contacts->match(first, Seaside::SearchRole,
                                              QVariant(stripLineID(lineId)),-1);

    if (!matches.isEmpty()) {
        QModelIndex person = matches.at(0); //First match wins
        SEASIDE_SHORTCUTS
        SEASIDE_SET_MODEL_AND_ROW(person.model(), person.row());

        QString firstName = SEASIDE_FIELD(FirstName, String);
        QString lastName = SEASIDE_FIELD(LastName, String);

        if (lastName.isEmpty())
            // Contacts first (common) name
            //% "%1"
            name = qtTrId("xx_first_name").arg(firstName);
        else
            name = firstName+ " " + lastName;
    }
    return name;
}


void HistoryProxy::getHistoryFinished(QDBusPendingCallWatcher *call)
{
    TRACE
    QDBusPendingReply<QArrayOfHistoryEvent> reply = *call;

    if (reply.isError()) {
        qWarning() << QString("HistoryProxy: GetVoiceHistory() failed: %1")
                             .arg(reply.error().message());
    } else {
        QArrayOfHistoryEvent events = reply.value();

        if (!events.isEmpty()) {
            QList<CallHistoryEvent> missedCalls;

            m_cache->beginGroup("CallHistory");

            // Cache the new events
            QStringList ids;
            foreach (CallHistoryEvent e, events) {
                ids << QString::number(e.start);
                // Data fields from ofono are:
                //     e.id     (unit)   GUID for the call
                //     e.lineid (char*)  Phone number
                //     e.type   (ushort) Event type (missed, in, out, etc...)
                //     e.start  (int)    start time (as string)
                //     e.end    (int)    end time (as string)
                m_cache->beginGroup(QString::number(e.start));
                m_cache->setValue("LineID", e.lineid);
                m_cache->setValue("Type",   QString::number(e.type));
                m_cache->setValue("Start",  QString::number(e.start));
                m_cache->setValue("End",    QString::number(e.end));
                m_cache->setValue("Name", resolveName(e.lineid));
                m_cache->endGroup(); // e.id;

                // Send notifications of "Missed" (type == 2) calls
                if (e.type == 2) {
                    missedCalls << e;
                }
            }
            m_cache->endGroup(); // "CallHistory"

            if (!missedCalls.isEmpty())
                sendMissedCallNotification(missedCalls);

            m_cache->sync();

            // Now ACK back to ofono that we've cached the latest events
            QDBusPendingReply<> ack = SetVoiceHistoryRead();

            // Notify that history has changed
            // FIXME:  I really need to properly hook up the model directly
            //         to this signal rather than forcing it from here.
            //emit historyChanged(ids);
            DialerApplication::instance()->historyModel()->appendRows(ids);
        }
    }
}

void HistoryProxy::voiceHistoryChanged(uint count)
{
    TRACE
    Q_UNUSED(count);
    QDBusPendingReply<QArrayOfHistoryEvent> reply;
    QDBusPendingCallWatcher * watcher;

    reply = GetVoiceHistory();
    watcher = new QDBusPendingCallWatcher(reply);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,    SLOT(getHistoryFinished(QDBusPendingCallWatcher*)));
}

void HistoryProxy::initCache()
{
    TRACE
    // Initialize the on disk event cache (possibly poor use of QSettings?)
    m_cache = new QSettings("com.meego");
    if (m_cache->status() != QSettings::NoError) {
        if (m_cache->status() == QSettings::AccessError)
            qWarning("HistoryProxy: AccessError while initializing QSettings");
        else
            qWarning("HistoryProxy: FormatError while initializing QSettings");
    }

    // Manually call the VoiceHistoryChanged signal to get any pending
    // events that we may have missed since last instantiation
    voiceHistoryChanged(0);
}

QSettings *HistoryProxy::cache() const
{
    TRACE
    return (m_cache->status() == QSettings::NoError)?m_cache:NULL;
}
