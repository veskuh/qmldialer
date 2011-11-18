/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "managerproxy.h"
#include "manager_interface.h"
#include "dialerapplication.h"
#include <QDebug>

ManagerProxy *ManagerProxy::gManager = 0;

ManagerProxy::ManagerProxy(const QString &service,
                           const QString &path,
                           const QDBusConnection &connection,
                           QObject *parent)
    : org::ofono::Manager(service, path, connection, parent),
      m_modemPath (""),
      m_modem(0),
      m_network(0),
      m_callManager(0),
      m_volumeManager(0),
      m_voicemail(0)
{
    if (gManager)
        qFatal("ManagerProxy: There can be only one!");

    if (!isValid()) {
        qDebug() << "Failed to connect to Ofono: \n\t" << lastError().message();
    } else {
        QDBusPendingReply<QArrayOfPathProperties> reply;
        QDBusPendingCallWatcher * watcher;

        reply = GetModems();
        watcher = new QDBusPendingCallWatcher(reply);

        // Force this to be sync to ensure we have initial properties
        watcher->waitForFinished();
        managerDBusGetModemsDone(watcher);

        connect(this,
                SIGNAL(ModemAdded(const QDBusObjectPath&, const QVariantMap&)),
                SLOT(modemAdded(const QDBusObjectPath&, const QVariantMap&)));
        connect(this,
                SIGNAL(ModemRemoved(const QDBusObjectPath&)),
                SLOT(modemRemoved(const QDBusObjectPath&)));
    }

    gManager = this;
}

ManagerProxy::~ManagerProxy()
{
    if (m_volumeManager)
        delete m_volumeManager;
    m_volumeManager = 0;

    if (m_voicemail)
        delete m_voicemail;
    m_voicemail = 0;

    if (m_callManager)
        delete m_callManager;
    m_callManager = 0;

    if (m_network)
        delete m_network;
    m_network = 0;

    if (m_modem)
        delete m_modem;
    m_modem = 0;

    gManager=0;
}

void ManagerProxy::managerDBusGetModemsDone(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QArrayOfPathProperties> reply = *call;

    if (reply.isError()) {
        // TODO: Handle this properly, by setting states, or disabling features
        qWarning() << "org.ofono.Manager.GetModems() failed: " <<
                      reply.error().message();
    } else {
        QArrayOfPathProperties modems = reply.value();
        if (modems.count() >= 1) {
            // FIXME: Handle multiple modems...
            foreach (OfonoPathProperties p, modems)
            {
                qDebug() << "modem: " << p.path.path();
                m_modemList << QString(p.path.path());
            }

            OfonoPathProperties p = modems[0];
            if (m_modemPath.isNull() || m_modemPath.isEmpty()) {
                qDebug() << QString("\n======\nUsing modem: \"%1\"\n======").arg(p.path.path());
                m_modemPath = QString(p.path.path());
                setModem(m_modemPath);
                setNetwork(m_modemPath);
                setCallManager(m_modemPath);
                setVolumeManager(m_modemPath);
                setVoicemail(m_modemPath);
            // TODO: Connect to service proxies as available/needed here
            }
        }
    }
}

void ManagerProxy::modemAdded(const QDBusObjectPath &in0,const QVariantMap &in1)
{
    Q_UNUSED(in1)
    TRACE

    // TODO: Handle modem additions, maybe...
    qWarning() << QString("Unhandled ModemAdded event: \"%1\"")
                  .arg(in0.path());

    qDebug() << QString("modem added: ").arg(in0.path());
    m_modemList << QString(in0.path());
    m_modemList.removeDuplicates();
}

void ManagerProxy::modemRemoved(const QDBusObjectPath &in0)
{
    TRACE

    // TODO: Handle modem removals, currently active for sure, others, maybe...
    qWarning() << QString("Unhandled ModemRemoved event: \"%1\"")
                  .arg(in0.path());

    qDebug() << QString("modem removed: ").arg(in0.path());
    m_modemList.removeOne(QString(in0.path()));
}

ManagerProxy *ManagerProxy::instance()
{
    if (!gManager)
        gManager = new ManagerProxy();

    return gManager;
}

ModemProxy* ManagerProxy::modem() const
{
    return m_modem;
}

NetworkProxy* ManagerProxy::network() const
{
    return m_network;
}

CallManager* ManagerProxy::callManager() const
{
    return m_callManager;
}

VolumeManager* ManagerProxy::volumeManager() const
{
    return m_volumeManager;
}

VoicemailProxy* ManagerProxy::voicemail() const
{
    return m_voicemail;
}

QStringList ManagerProxy::getModemList()
{
    return m_modemList;
}

void ManagerProxy::setModem(QString modemPath)
{
    if (m_modem &&
        m_modem->isValid() &&
        m_modem->path() == modemPath)
        return;

    if (m_modem)
    {
        delete m_modem;
        m_modem = NULL;
    }

    if (m_modemList.contains(modemPath)) {
        m_modem = new ModemProxy(modemPath);
        emit modemChanged();
    }
}

void ManagerProxy::setNetwork(QString modempath)
{
    if (!m_modem || !m_modem->isValid())
        return;

    if (modempath == m_modem->path()) {
        if (m_network && m_network->isValid()) {
            return;
        }
        else {
            delete m_network;
            m_network = new NetworkProxy(modempath);
            emit networkChanged();
        }
    }
    else {
        if(m_network || !m_network->isValid()) {
            delete m_network;
            m_network = new NetworkProxy(modempath);
            emit networkChanged();
        }
    }
}

void ManagerProxy::setCallManager(QString modempath)
{
    if (!m_modem || !m_modem->isValid())
        return;

    if (modempath == m_modem->path()) {
        if (m_callManager && m_callManager->isValid()) {
            return;
        }
        else {
            delete m_callManager;
            m_callManager = new CallManager(modempath);
            emit callManagerChanged();
        }
    }
    else {
        if(m_callManager || !m_callManager->isValid()) {
            delete m_callManager;
            m_callManager = new CallManager(modempath);
            emit callManagerChanged();
        }
    }
}

void ManagerProxy::setVolumeManager(QString modempath)
{
    if (!m_modem || !m_modem->isValid())
        return;

    if (modempath == m_modem->path()) {
        if (m_volumeManager && m_volumeManager->isValid()) {
            return;
        }
        else {
            delete m_volumeManager;
            m_volumeManager = new VolumeManager(modempath);
            emit volumeManagerChanged();
        }
    }
    else {
        if(m_volumeManager || !m_volumeManager->isValid()) {
            delete m_volumeManager;
            m_volumeManager = new VolumeManager(modempath);
            emit volumeManagerChanged();
        }
    }
}

void ManagerProxy::setVoicemail(QString modempath)
{
    if (!m_modem || !m_modem->isValid())
        return;

    if (modempath == m_modem->path()) {
        if (m_voicemail && m_voicemail->isValid()) {
            return;
        }
        else {
            delete m_voicemail;
            m_voicemail = new VoicemailProxy(modempath);
            emit voicemailChanged();
        }
    }
    else {
        if(m_voicemail || !m_voicemail->isValid()) {
            delete m_voicemail;
            m_voicemail = new VoicemailProxy(modempath);
            emit voicemailChanged();
        }
    }
}
