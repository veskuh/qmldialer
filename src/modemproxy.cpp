/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "modemproxy.h"
#include <QDebug>

ModemProxy::ModemProxy(const QString &objectPath)
    : org::ofono::Modem(OFONO_SERVICE,
                        objectPath,
                        QDBusConnection::systemBus()),
      m_interfaces(0)
{
    if (!isValid()) {
        qDebug() << "Failed to connect to Ofono: \n\t" << lastError().message();
    } else {
        m_path = objectPath;
        QDBusPendingReply<QVariantMap> reply;
        QDBusPendingCallWatcher * watcher;

        reply = GetProperties();
        watcher = new QDBusPendingCallWatcher(reply);

        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(modemDBusGetPropDone(QDBusPendingCallWatcher*)));

        connect(this,
                SIGNAL(PropertyChanged(const QString&,const QDBusVariant&)),
                SLOT(propertyChanged(const QString&,const QDBusVariant&)));
    }
}

ModemProxy::~ModemProxy()
{
}

QStringList ModemProxy::interfaces() const { return m_interfaces; }
QString ModemProxy::path() const { return m_path; }
QString ModemProxy::manufacturer() const { return m_manufacturer; }
QString ModemProxy::model() const { return m_model; }
QString ModemProxy::revision() const { return m_revision; }
QString ModemProxy::serial() const { return m_serial; }
bool    ModemProxy::powered() const { return m_powered; }
bool    ModemProxy::online() const { return m_online; }

void ModemProxy::setPowered(bool is_powered)
{
    if (m_powered == is_powered)
        return;

    QDBusPendingReply<QVariantMap> reply;
    reply = SetProperty("Powered", QDBusVariant(is_powered?true:false));
    if (reply.isError())
        qCritical() << "SetProperty \"Powered\" failed!";
}

void ModemProxy::setOnline(bool is_online)
{
    if (m_online == is_online)
        return;

    QDBusPendingReply<QVariantMap> reply;
    reply = SetProperty("Online", QDBusVariant(m_online?"true":"false"));
    if (reply.isError())
        qCritical() << "SetProperty \"Powered\" failed!";
    else
        m_online = is_online;
}

void ModemProxy::modemDBusGetPropDone(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;

    if (reply.isError()) {
      // TODO: Handle this properly, by setting states, or disabling features...
      qDebug() << "org.ofono.ModemProxy.getProperties() failed: " <<
                  reply.error().message();
    } else {
      QVariantMap properties = reply.value();
      m_interfaces = qdbus_cast<QStringList >(properties["Interfaces"]);
      m_manufacturer = qdbus_cast<QString>(properties["Manufacturer"]);
      m_model = qdbus_cast<QString>(properties["Model"]);
      m_powered = qdbus_cast<bool>(properties["Powered"]);
      m_revision = qdbus_cast<QString>(properties["Revision"]);
      m_serial = qdbus_cast<QString>(properties["Serial"]);
      m_online = qdbus_cast<bool>(properties["Online"]);

      // First sucessfull GetProperties == connected
      if (!m_connected) {
          m_connected = true;
          emit connected();
      }
    }
}

void ModemProxy::propertyChanged(const QString &in0, const QDBusVariant &in1)
{
    qDebug() << "org.ofono.ModemProxy.propertyChanged()"
             << in0 << ": " << in1.variant();
    if (in0 == "Interfaces") {
        m_interfaces = qdbus_cast<QStringList>(in1.variant());
        emit interfacesChanged(m_interfaces);
    }
    else if (in0 == "Powered") {
        m_powered = qdbus_cast<bool>(in1.variant());
        emit poweredChanged(m_powered);
    } else if (in0 == "Online") {
        m_online = qdbus_cast<bool>(in1.variant());
        emit onlineChanged(m_online);
    } else {
        qDebug() << QString("Unhandled property \"%1\" changed...").arg(in0);
    }
}

/*
 * VoicemailProxy (aka MessageWaiting) implementation
 */

VoicemailProxy::VoicemailProxy(const QString &objectPath)
    : org::ofono::MessageWaiting(OFONO_SERVICE,
                                 objectPath,
                                 QDBusConnection::systemBus()),
                                 m_connected(false)
{
    if (!isValid()) {
        qDebug() << "Failed to connect to Ofono: \n\t" << lastError().message();
    } else {
        m_path = objectPath;
        QDBusPendingReply<QVariantMap> reply;
        QDBusPendingCallWatcher * watcher;

        reply = GetProperties();
        watcher = new QDBusPendingCallWatcher(reply);

        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(voicemailDBusGetPropDone(QDBusPendingCallWatcher*)));
        connect(this, SIGNAL(PropertyChanged(const QString&, const QDBusVariant&)),
                SLOT(voicemailPropertyChanged(const QString&, const QDBusVariant&)));
    }
}

VoicemailProxy::~VoicemailProxy()
{
}

bool    VoicemailProxy::isConnected() const { return m_connected; }
QString VoicemailProxy::path() const { return m_path; }
QString VoicemailProxy::mailbox() const { return m_mailbox; }
int     VoicemailProxy::count() const { return m_count; }
bool    VoicemailProxy::waiting() const { return m_waiting; }

void VoicemailProxy::setMailbox(QString lineid)
{
    if (lineid.isEmpty() || (m_mailbox == lineid))
        return;

    QDBusPendingReply<> reply;
    reply = SetProperty("VoicemailMailboxNumber", QDBusVariant(lineid));
    reply.waitForFinished();

    if (reply.isError())
        qCritical() << "SetProperty \"VoicemailMailboxNumber\" failed: " <<
                       reply.error().message();
    else
        m_mailbox = lineid;
}

void VoicemailProxy::voicemailDBusGetPropDone(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;

    if (reply.isError()) {
      // TODO: Handle this properly, by setting states, or disabling features...
      qDebug() << "org.ofono.MessageWaiting.getProperties() failed: " <<
                  reply.error().message();
    } else {
      QVariantMap properties = reply.value();
      bool waiting = qdbus_cast<bool>(properties["VoicemailWaiting"]);
      int count = qdbus_cast<int>(properties["VoicemailMessageCount"]);
      QString mailbox = qdbus_cast<QString>(properties["VoicemailMailboxNumber"]);

      if (count != m_count) {
          m_count = count;
          emit messagesWaitingChanged();
      }
      if (waiting != m_waiting) {
          m_waiting = waiting;
          emit messagesWaitingChanged();
      }
      if (!mailbox.isEmpty() && (mailbox != m_mailbox)) {
          m_mailbox = mailbox;
          emit mailboxChanged();
      }

      // First sucessfull GetProperties == connected
      if (!m_connected) {
          m_connected = true;
          emit connected();
      }
    }
}

void VoicemailProxy::voicemailPropertyChanged(const QString &in0, const QDBusVariant &in1)
{
    qDebug() << QString("Property \"%1\" changed...").arg(in0);
    bool waiting;
    int count;
    QString mailbox;
    if (in0 == "VoicemailWaiting") {
        waiting = qdbus_cast<bool>(in1.variant());
    } else if (in0 == "VoicemailMessageCount") {
        count = qdbus_cast<int>(in1.variant());
    } else if (in0 == "VoicemailMailboxNumber") {
        mailbox = qdbus_cast<QString>(in1.variant());
    } else
        qDebug() << QString("Unexpected property changed...");

    if ((count != m_count) || (waiting != m_waiting)) {
        m_count = count;
        m_waiting = waiting;
        emit messagesWaitingChanged();
    }
    if (!mailbox.isEmpty() && (mailbox != m_mailbox)) {
        m_mailbox = mailbox;
        emit mailboxChanged();
    }
}

/*
 * CallVolume Manager implementation
 */

VolumeManager::VolumeManager(const QString &objectPath)
    : org::ofono::CallVolume(OFONO_SERVICE,
                             objectPath,
                             QDBusConnection::systemBus())
{
    if (!isValid()) {
        qDebug() << "Failed to connect to Ofono: \n\t" << lastError().message();
    } else {
        m_path = objectPath;
        QDBusPendingReply<QVariantMap> reply;
        QDBusPendingCallWatcher * watcher;

        reply = GetProperties();
        watcher = new QDBusPendingCallWatcher(reply);

        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(volumeDBusGetPropDone(QDBusPendingCallWatcher*)));
    }
}

VolumeManager::~VolumeManager()
{
}

QString VolumeManager::path() const { return m_path; }
int     VolumeManager::speakerVolume() const { return m_speakerVolume; }
int     VolumeManager::micVolume() const { return m_micVolume; }
bool    VolumeManager::muted() const { return m_muted; }
bool    VolumeManager::isConnected() const { return m_connected; }

void VolumeManager::setSpeakerVolume(int volume)
{
    if (m_speakerVolume == volume)
        return;

    if ((volume < 0) || (volume > 100)) {
        qWarning() << "SpeakerVolume value out of range (0<>100)";
        return;
    }

    QDBusPendingReply<> reply;
    reply = SetProperty("SpeakerVolume", QDBusVariant(volume));
    reply.waitForFinished();

    if (reply.isError())
        qCritical() << "SetProperty \"SpeakerVolume\" failed: " <<
                       reply.error().message();
    else
        m_speakerVolume = volume;
}

void VolumeManager::setMicVolume(int volume)
{
    if (m_micVolume == volume)
        return;

    if ((volume < 0) || (volume > 100)) {
        qWarning() << "MicrophoneVolume value out of range (0<>100)";
        return;
    }

    QDBusPendingReply<> reply;
    reply = SetProperty("MicrophoneVolume", QDBusVariant(volume));
    reply.waitForFinished();

    if (reply.isError())
        qCritical() << "SetProperty \"MicrophoneVolume\" failed: " <<
                       reply.error().message();
    else
        m_micVolume = volume;
}

void VolumeManager::setMuted(bool is_muted)
{
    if (m_muted == is_muted)
        return;

    QDBusPendingReply<> reply;
    reply = SetProperty("Muted", QDBusVariant(is_muted));
    reply.waitForFinished();

    if (reply.isError())
        qCritical() << "SetProperty \"Muted\" failed: " <<
                       reply.error().message();
    else
        m_muted = is_muted;
}

void VolumeManager::volumeDBusGetPropDone(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;

    if (reply.isError()) {
      // TODO: Handle this properly, by setting states, or disabling features...
      qDebug() << "org.ofono.CallVolume.getProperties() failed: " <<
                  reply.error().message();
    } else {
      QVariantMap properties = reply.value();
      m_speakerVolume = qdbus_cast<int>(properties["SpeakerVolume"]);
      m_micVolume = qdbus_cast<int>(properties["MicrophoneVolume"]);
      m_muted = qdbus_cast<bool>(properties["Muted"]);

      // First sucessfull GetProperties == connected
      if (!m_connected) {
          m_connected = true;
          emit connected();
      }
    }
}
