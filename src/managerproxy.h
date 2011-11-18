/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef MANAGERPROXY_H
#define MANAGERPROXY_H

#include "manager_interface.h"
#include "modemproxy.h"
#include "networkproxy.h"
#include "callmanager.h"
#include "resourceproxy.h"
#include <QtDBus>
#include <QDebug>

#define OFONO_SERVICE "org.ofono"
#define OFONO_MANAGER_PATH "/"

class ManagerProxy: public org::ofono::Manager
{
    Q_OBJECT
    Q_PROPERTY(ModemProxy* modem READ modem)
    Q_PROPERTY(NetworkProxy* network READ network)

public:
    virtual ~ManagerProxy();

    static ManagerProxy *instance();
    QStringList getModemList();
    void setModem(QString modempath);
    void setNetwork(QString modempath);
    void setCallManager(QString modempath);
    void setVolumeManager(QString modempath);
    void setVoicemail(QString modempath);

    ModemProxy* modem() const;
    NetworkProxy* network() const;
    CallManager* callManager() const;
    VolumeManager* volumeManager() const;
    VoicemailProxy* voicemail() const;

public slots:
    void managerDBusGetModemsDone(QDBusPendingCallWatcher *call);

Q_SIGNALS:
    void modemChanged();
    void networkChanged();
    void callManagerChanged();
    void volumeManagerChanged();
    void voicemailChanged();

private Q_SLOTS:
    void modemAdded(const QDBusObjectPath &in0, const QVariantMap &in1);
    void modemRemoved(const QDBusObjectPath &in0);

protected:
    ManagerProxy(const QString &service=OFONO_SERVICE,
                 const QString &path=OFONO_MANAGER_PATH,
                 const QDBusConnection &connection=QDBusConnection::systemBus(),
                 QObject *parent = 0);

private:
    ManagerProxy(const ManagerProxy&);
    ManagerProxy& operator= (ManagerProxy&);

    QString       m_modemPath;
    ModemProxy   *m_modem;
    NetworkProxy *m_network;
    CallManager  *m_callManager;
    VolumeManager *m_volumeManager;
    VoicemailProxy *m_voicemail;
    QStringList m_modemList;

    static ManagerProxy *gManager;
};

#endif
