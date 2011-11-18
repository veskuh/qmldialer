/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include <ofonovoicecallmanager.h>

#include "callitem.h"
#include <QtDBus>

class CallManager: public OfonoVoiceCallManager
{
    Q_OBJECT

    Q_PROPERTY(int multipartyCallCount READ multipartyCallCount NOTIFY multipartyCallCountChanged)
    Q_PROPERTY(int callCount READ callCount NOTIFY callCountChanged)

public:
    explicit CallManager(const QString &modemPath="", QObject *parent=0);
    virtual ~CallManager();

    /* Properties */
    int multipartyCallCount() const;
    int callCount() const;

    // Extended version of OfonoVoiceCallManager::getCalls() that
    // returns QList of CallItems rather than strings
    Q_INVOKABLE QList<CallItem *> getCallItems() const;

    Q_INVOKABLE CallItem *activeCall() const;
    Q_INVOKABLE CallItem *heldCall() const;
    Q_INVOKABLE CallItem *dialingCall() const;
    Q_INVOKABLE CallItem *incomingCall() const;
    Q_INVOKABLE CallItem *waitingCall() const;
    Q_INVOKABLE CallItem *alertingCall() const;

public Q_SLOTS:
    void setActiveCall(const CallItem &call);

    // Overloaded version of OfonoVoiceCallManager::dial() that
    // takes a PeopleItem rather than number string and uses default
    // as CLIR value
    void dial(const PeopleItem *person);

    // Overloaded version of OfonoVoiceCallManager::dial() that
    // assumes CLIR == default
    void dial(const QString &number);

    // Overloaded version of OfonoVoiceCallManager::privateChat() that
    // takes a CallItem rather than path string
    void privateChat(const CallItem &call);

    // Push denied answer signal to upper layers from call proxy
    void deniedCallAnswer();

Q_SIGNALS:
    // Abstracts both callAdded() and callRemoved() into a single event
    void callsChanged();

    void incomingCall(CallItem *call);
    void incomingCall(QString path);
    void callDisconnected(const CallItem &call);
    void onlyEmergencyCalls();
    void connected();
    void multipartyCallCountChanged(const int &count);
    void callCountChanged(const int &count);

private Q_SLOTS:
    void updateCallItems();
    void addCall(const QString &path);
    void removeCall(const QString &path);
    void dialFinished(const bool status);
    void hangupAllFinished(const bool status);
    void swapFinished(const bool status);
    void holdAndAnswerFinished(const bool status);
    void transferFinished(const bool status);
    void releaseAndAnswerFinished(const bool status);
    void privateChatFinished(const bool status);
    void createMultipartyFinished(const bool status);
    void hangupMultipartyFinished(const bool status);
    void sendTonesFinished(const bool status);
    void callStateChanged();
    void callMultipartyChanged();

    // Handlers for Resource Policy Manager states and conditions
    void proceedCallDial(const QString number);
    void deniedCallDial();
    void lostCallDial();
    void proceedIncomingCall(CallItem *call);
    void deniedIncomingCall(CallItem *call);
    void lostIncomingCall(CallItem *call);

private:
    class CallManagerPrivate *d;

    Q_DISABLE_COPY(CallManager)
};

#endif // CALLMANAGER_H
