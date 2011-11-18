/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "common.h"
#include "callmanager.h"
#include "resourceproxy.h"

class CallManagerPrivate
{
public:
    ResourceProxy *resource;
    QHash<QString, CallItem *>  callItems;
};

CallManager::CallManager(const QString &modemPath, QObject *parent)
    : OfonoVoiceCallManager(OfonoModem::AutomaticSelect, modemPath, parent),
      d(new CallManagerPrivate)
{
    TRACE

    d->resource = ResourceProxy::instance();

    // Resource proxy binding
    if (d->resource) {
        connect(d->resource, SIGNAL(incomingResourceAcquired(CallItem *)),
                SLOT(proceedIncomingCall(CallItem *)));
        connect(d->resource, SIGNAL(incomingResourceDenied(CallItem *)),
                SLOT(deniedIncomingCall(CallItem *)));
        connect(d->resource, SIGNAL(incomingResourceLost(CallItem *)),
                SLOT(lostIncomingCall(CallItem *)));

        connect(d->resource, SIGNAL(dialResourceAcquired(const QString)),
                SLOT(proceedCallDial(const QString)));
        connect(d->resource, SIGNAL(dialResourceDenied()),
                SLOT(deniedCallDial()));
        connect(d->resource, SIGNAL(callResourceLost()),
                SLOT(lostCallDial()));
    }

    // Transform existing calls list, into list of CallItems
    updateCallItems();

    // Begin tracking calls
    connect(this, SIGNAL(callAdded(const QString)),
            this, SLOT(addCall(const QString)));
    connect(this, SIGNAL(callRemoved(const QString)),
            this, SLOT(removeCall(const QString)));

    // Hook into parent class signals
    connect(this, SIGNAL(dialComplete(const bool)),
            this, SLOT(dialFinished(const bool)));
    connect(this, SIGNAL(swapCallsComplete(const bool)),
            this, SLOT(swapFinished(const bool)));
    connect(this, SIGNAL(hangupAllComplete(const bool)),
            this, SLOT(hangupAllFinished(const bool)));
    connect(this, SIGNAL(sendTonesComplete(const bool)),
            this, SLOT(sendTonesFinished(const bool)));
    connect(this, SIGNAL(holdAndAnswerComplete(const bool)),
            this, SLOT(holdAndAnswerFinished(const bool)));
    connect(this, SIGNAL(transferComplete(const bool)),
            this, SLOT(transferFinished(const bool)));
    connect(this, SIGNAL(releaseAndAnswerComplete(const bool)),
            this, SLOT(releaseAndAnswerFinished(const bool)));
    connect(this, SIGNAL(privateChatComplete(const bool)),
            this, SLOT(privateChatFinished(const bool)));
    connect(this, SIGNAL(createMultipartyComplete(const bool)),
            this, SLOT(createMultipartyFinished(const bool)));
    connect(this, SIGNAL(hangupMultipartyComplete(const bool)),
            this, SLOT(hangupMultipartyFinished(const bool)));

    if (isValid())
        emit connected();
}

CallManager::~CallManager()
{
    TRACE
    // FIXME: Do something here!!!
    qDebug() << QString("Destroying VoiceCallManager");
    qDebug() << QString("Purging all CallItems");
    foreach (CallItem *item, d->callItems) {
        disconnect(item, SIGNAL(stateChanged()));
        disconnect(item, SIGNAL(dataChanged()));
        delete item;
    }
    d->callItems.clear();
}

QList<CallItem *> CallManager::getCallItems() const
{
    TRACE
    return d->callItems.values();
}

int CallManager::callCount() const
{
    TRACE
    return getCalls().length();
}

int CallManager::multipartyCallCount() const
{
    TRACE
    int call_count = 0;
    foreach (CallItem *c, d->callItems) {
        if(c->multiparty()) {
            call_count++;
        }
    }
    return call_count;
}

CallItem *CallManager::activeCall() const
{
    TRACE
    if (d->callItems.size())
    foreach (CallItem *c, d->callItems)
        if (c->state() == CallItemModel::STATE_ACTIVE)
            return c;
    return NULL;
}

CallItem *CallManager::heldCall() const
{
    TRACE
    if (d->callItems.size())
    foreach (CallItem *c, d->callItems)
        if (c->state() == CallItemModel::STATE_HELD)
            return c;
    return NULL;
}

CallItem *CallManager::dialingCall() const
{
    TRACE
    if (d->callItems.size())
    foreach (CallItem *c, d->callItems)
        if (c->state() == CallItemModel::STATE_DIALING)
            return c;
    return NULL;
}

CallItem *CallManager::incomingCall() const
{
    TRACE
    if (d->callItems.size())
    foreach (CallItem *c, d->callItems)
        if (c->state() == CallItemModel::STATE_INCOMING)
            return c;
    return NULL;
}

CallItem *CallManager::waitingCall() const
{
    TRACE
    if (d->callItems.size())
    foreach (CallItem *c, d->callItems)
        if (c->state() == CallItemModel::STATE_WAITING)
            return c;
    return NULL;
}

CallItem *CallManager::alertingCall() const
{
    TRACE
    if (d->callItems.size())
    foreach (CallItem *c, d->callItems)
        if (c->state() == CallItemModel::STATE_ALERTING)
            return c;
    return NULL;
}

void CallManager::setActiveCall(const CallItem &call)
{
    TRACE
    if (!call.isActive())
        swapCalls();
}

void CallManager::dial(const PeopleItem *person)
{
    TRACE
    dial(person->phone());
}

void CallManager::dial(const QString &number)
{
    TRACE

    // Nothing to do if the modem is not powered up
    if(!modem()->powered()) {
        emit callsChanged();
        return;
    }

    // If not online (flight mode?), check if the requested number is
    // one of the allowed EmergencyNumbers, in which case, continue.
    // Otherwise, notify that only Emergency calls are permitted.
    if(!modem()->online()) {
        if(modem()->powered() && !emergencyNumbers().contains(number)) {
            emit callsChanged();
            emit onlyEmergencyCalls();
            return;
        }
    }

    if (d->resource)
        d->resource->acquireDialResource(number);
}

void CallManager::privateChat(const CallItem &call)
{
    TRACE
    OfonoVoiceCallManager::privateChat(call.path());
}

/*
 * Resource Policy Manager Handler slots
 */
void CallManager::deniedCallDial()
{
    TRACE
    qCritical() << QString("Denied: Dial resource");
}

void CallManager::lostCallDial()
{
    TRACE
    qCritical() << QString("Lost: Dial resource");
    hangupAll();
}

void CallManager::proceedCallDial(const QString number)
{
    TRACE
    OfonoVoiceCallManager::dial(stripLineID(number), QString());
}

void CallManager::deniedCallAnswer()
{
    TRACE
    qCritical() << QString("Denied: Call resource");
    hangupAll();
}

void CallManager::deniedIncomingCall(CallItem *call)
{
    TRACE

    qCritical() << QString("Denied: Incoming Call resource");
    qDebug() << QString("Insert new CallItem %1").arg(call->path());
    emit callsChanged();
    emit incomingCall(call);
}

void CallManager::lostIncomingCall(CallItem *call)
{
    TRACE
    Q_UNUSED(call)
    qCritical() << QString("Lost: Incoming Call resource");
}

void CallManager::proceedIncomingCall(CallItem *call)
{
    TRACE
    qDebug() << QString("Acquired: Incoming Call resource");
    qDebug() << QString("Insert new CallItem %1").arg(call->path());
    emit callsChanged();
    emit incomingCall(call);
}

/*
 * Private slots for async replies
 */

void CallManager::updateCallItems()
{
    TRACE
    bool changed = false;

    /*
    // If ofono call list is empty (no calls), empty our CallItem list too.
    if (getCalls().isEmpty() && !d->callItems.isEmpty()) {
        qDebug() << QString("*** Purging all CallItems");

        foreach (CallItem *item, d->callItems)
        {
            delete item;
        }

        TRACE

        d->callItems.clear();

        TRACE

        if (d->resource) d->resource->releaseResources();

        TRACE

        emit callsChanged();
        return;
    }
    */

    // Remove CallItems that are not in the ofono "calls" list
    QMutableHashIterator<QString, CallItem*> iter(d->callItems);
    while (iter.hasNext()) {
        CallItem *item = iter.next().value();
        // This item is not in the ofono list, remove it
        if (!getCalls().contains(item->path())) {
            qDebug() << QString("Removing old CallItem %1").arg(item->path());
            delete item;
            iter.remove();
            changed = true;
        }
    }

    // Insert new CallItems for paths in the ofono "calls" list we are missing
    foreach (QString path, getCalls()) {
        // Insert a new CallItem
	if (!d->callItems.contains(path)) {
            qDebug() << QString("Inserting new CallItem %1").arg(path);
            CallItem *call = new CallItem(path);
            connect (call, SIGNAL(stateChanged()),
                     this, SLOT(callStateChanged()));
            connect (call, SIGNAL(multipartyChanged()),
                     this, SLOT(callMultipartyChanged()));
            d->callItems.insert(path, call);

            // NOTE: Must explicity bubble this up since incoming and waiting
            //       calls do not "changeState" unless they are handled or
            //       timeout
            if ((call->state() == CallItemModel::STATE_INCOMING) ||
                (call->state() == CallItemModel::STATE_WAITING)) {
                if (d->resource)
                    d->resource->acquireIncomingResource(call);
            } else {
                changed = true;
            }
        }
    }

    if(d->callItems.empty())
    {
        if (d->resource) d->resource->releaseResources();
    }

    if (changed)
        emit callsChanged();
}

void CallManager::addCall(const QString &path)
{
    TRACE
    qDebug() << QString("CallAdded: \"%1\"").arg(path);
    updateCallItems();
    emit callCountChanged(callCount());
}

void CallManager::removeCall(const QString &path)
{
    TRACE
    qDebug() << QString("CallRemoved: \"%1\"").arg(path);
    updateCallItems();
    emit callCountChanged(callCount());
}

void CallManager::dialFinished(const bool status)
{
    TRACE
    if (!status) {
        qCritical() << QString("dial() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
        // Fix BMC#10848:
        // Notify that state of the call has changed when the dialing fails
        emit callsChanged();
    }
}

void CallManager::hangupAllFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("hangupAll() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::swapFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("swapCalls() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::holdAndAnswerFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("HoldAndAnswer() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::transferFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("Transfer() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::releaseAndAnswerFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("ReleaseAndAnswer() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::privateChatFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("PrivateChat() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::createMultipartyFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("CreateMultiparty() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::hangupMultipartyFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("HangupMultiparty() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::sendTonesFinished(const bool status)
{
    TRACE
    if (!status)
        qCritical() << QString("SendTones() Failed: %1 - %2")
                       .arg(errorName())
                       .arg(errorMessage());
}

void CallManager::callStateChanged()
{
    CallItem *call = dynamic_cast<CallItem *>(sender());
    qDebug() << QString("%1 (%2) state has changed to %3")
                .arg(call->path())
                .arg(call->lineID())
                .arg(call->state());
    emit callsChanged();
}

void CallManager::callMultipartyChanged()
{
    TRACE
    emit callsChanged();
    emit multipartyCallCountChanged(multipartyCallCount());
}
