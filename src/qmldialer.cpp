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
#include "qmldialer.h"
#include "dialerapplication.h"
#include "dialercontext.h"
#include "callmanager.h"
#include "managerproxy.h"

#define CONFIG_KEY_VOICEMAIL_NUMBER "/apps/dialer/voicemail/number"

class QMLDialerPrivate
{
public:
    QMLDialerPrivate()
        : currentCall(NULL)
    { TRACE }

    QMLCallItem *currentCall;
};

QMLDialer::QMLDialer(QObject *parent)
    : QObject(parent), d(new QMLDialerPrivate)
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();

    this->connectAll();
    connect(ManagerProxy::instance(), SIGNAL(callManagerChanged()), SLOT(onCallManagerChanged()));

    if(cm && cm->activeCall()) d->currentCall = new QMLCallItem(cm->activeCall(), this);
}

QMLDialer::~QMLDialer()
{
    TRACE
    delete this->d;
}

QString QMLDialer::mailbox() const
{
    TRACE

    if(ManagerProxy::instance()->voicemail()->isConnected())
    {
        return ManagerProxy::instance()->voicemail()->mailbox();
    }

    // Fall back to GConf voicemail setting.
    return MGConfItem(CONFIG_KEY_VOICEMAIL_NUMBER).value().toString();
}

bool QMLDialer::modemOnline()
{
    if(ManagerProxy::instance()->modem() &&
       ManagerProxy::instance()->modem()->isValid())
    {
        return ManagerProxy::instance()->modem()->powered();
    }

    return false;
}

void QMLDialer::setMailbox(const QString &number)
{
    TRACE

    if(ManagerProxy::instance()->voicemail()->isConnected())
    {
        ManagerProxy::instance()->voicemail()->setMailbox(number);
        return;
    }

    // Fall back to GConf voicemail setting.
    MGConfItem(CONFIG_KEY_VOICEMAIL_NUMBER).set(number);
}

void QMLDialer::setModemOnline(bool online)
{
    if(ManagerProxy::instance()->modem()->isValid())
    {
        ManagerProxy::instance()->modem()->setPowered(online);
        return;
    }
}

QString QMLDialer::speedDial(int index) const
{
    TRACE
    return MGConfItem(QString("/apps/dialer/speeddial/%1").arg(index)).value().toString();
}

void QMLDialer::setSpeedDial(int index, const QString &number)
{
    TRACE
    MGConfItem(QString("/apps/dialer/speeddial/%1").arg(index)).set(number);
}

QMLCallItem* QMLDialer::currentCall() const
{
    TRACE
    return d->currentCall;
}

int QMLDialer::callCount() const
{
    TRACE
    return ManagerProxy::instance()->callManager()->callCount();
}

int QMLDialer::multipartyCallCount() const
{
    TRACE
    return ManagerProxy::instance()->callManager()->multipartyCallCount();
}

void QMLDialer::dial(const QString &msisdn)
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
    {
        cm->dial(msisdn);
    }
}

void QMLDialer::hangupDialingCall()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    CallItem *c = NULL;

    if (cm->dialingCall())
    {
        c = cm->dialingCall();
    }
    else if(cm->alertingCall())
    {
        c = cm->alertingCall();
    }

    if (c) {
        c->callProxy()->hangup();
    }
}

void QMLDialer::hangupAll()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->hangupAll();
}

void QMLDialer::mutePressed(bool checked)
{
    TRACE
    if(ManagerProxy::instance()->volumeManager()->isConnected())
    {
        qDebug() << "Attempting to mute call with volume manager.";
        ManagerProxy::instance()->volumeManager()->setMuted(checked);
    }
    else
    {
        qDebug() << "Couldn't decide how to mute call!";
    }
}

void QMLDialer::silenceRingtone()
{
    TRACE
    if(d->currentCall)
    {
        qDebug() << "Attempting to silence ringtone with call item signaller.";
        d->currentCall->proxy()->silenceRingtone();
    }
    else
    {
        qDebug() << "Couldn't decide how to mute ringtone!";
    }
}

void QMLDialer::multiPressed()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm->multipartyCallCount() == 0) {
        if (cm->callCount() == 2)
            cm->createMultiparty();
    } else {
        if ((cm->callCount() - cm->multipartyCallCount()) >= 1)
            cm->createMultiparty();
    }
}

void QMLDialer::sendTones(const QString &tones)
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->sendTones(tones);
}

void QMLDialer::transfer()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->transfer();
}

void QMLDialer::swapCalls()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->swapCalls();
}

void QMLDialer::releaseAndAnswer()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->releaseAndAnswer();
}

void QMLDialer::holdAndAnswer()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->holdAndAnswer();
}

void QMLDialer::createMultiparty()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->createMultiparty();
}

void QMLDialer::hangupMultiparty()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->hangupMultiparty();
}

void QMLDialer::privateChat(const QMLCallItem &call)
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();
    if (cm && cm->isValid())
        cm->privateChat(*call.proxy());
}

void QMLDialer::onCallsChanged()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();

    if(cm->activeCall())
    {
        this->onIncomingCall(cm->activeCall());
    }
    else if (cm->dialingCall())
    {
        // for dial out
        d->currentCall = new QMLCallItem(cm->dialingCall());
    }
    else if (cm->alertingCall())
    {
        // for dial out.
        d->currentCall = new QMLCallItem(cm->alertingCall());
    }
    else
    {
        d->currentCall = NULL;
    }
}

void QMLDialer::onIncomingCall(CallItem *callitem)
{
    TRACE
    DialerApplication::instance()->setPrestarted(false);

    d->currentCall = new QMLCallItem(callitem, this);
    emit this->incomingCall();
}

void QMLDialer::connectAll()
{
    TRACE
    CallManager *cm = ManagerProxy::instance()->callManager();

    if (cm && cm->isValid())
    {
        disconnect(cm, SIGNAL(callsChanged()));
        disconnect(cm, SIGNAL(incomingCall(CallItem*)));
        connect(cm, SIGNAL(callsChanged()), this, SLOT(onCallsChanged()));
        connect(cm, SIGNAL(callsChanged()), this, SIGNAL(callsChanged()));
        connect(cm, SIGNAL(incomingCall(CallItem*)), SLOT(onIncomingCall(CallItem*)));
        qDebug() << QString("CallMgr is connected");
    }
    else if (cm)
    {
        disconnect(cm, SIGNAL(connected()));
        connect(cm, SIGNAL(connected()), this, SLOT(connectAll()));
        qDebug() << QString("CallMgr is not yet valid");
    }
}

void QMLDialer::onCallManagerChanged()
{
    TRACE

    CallManager *cm = ManagerProxy::instance()->callManager();
    this->connectAll();
    if(cm && cm->activeCall()) d->currentCall = new QMLCallItem(cm->activeCall(), this);
}
