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
#include "dialercontext.h"

#define AUTOSELECT OfonoModem::AutomaticSelect

#include <MGConfItem>
#ifndef CONFIG_KEY_TARGET_MODE
#define CONFIG_KEY_TARGET_MODE "/apps/dialer/mode"
#endif

#include <QDebug>

DialerContext *DialerContext::gContext = 0;

class DialerContextPrivate
{
public:
    DialerContextPrivate()
        : modemManager(0),
          volumeManager(0),
          voicemailManager(0),
          callManager(0),
          policyManager(0),
          modesKey(CONFIG_KEY_TARGET_MODE)
    { TRACE }

    ~DialerContextPrivate() {
        // NOTE: Not deleting policyManager, it's a Singleton
        delete volumeManager;
        volumeManager = 0;
        delete voicemailManager;
        voicemailManager = 0;
        delete callManager;
        callManager = 0;
        delete modemManager;
        modemManager = 0;
    }

    OfonoModemManager   *modemManager;
    OfonoCallVolume     *volumeManager;
    OfonoMessageWaiting *voicemailManager;
    CallManager         *callManager;
    ResourceProxy       *policyManager;
    QStringList          modes;
    MGConfItem           modesKey;
};


DialerContext::DialerContext(QObject *parent)
    : QObject(parent),
      d(new DialerContextPrivate)
{
    if (gContext)
        qFatal(__func__, ": There can be only one!");

    // Read and configure default runtime modes
    setModes(d->modesKey.value().toStringList());

    // Create misc services
    // TODO: We may not actually need this, since OfonoModem class
    //       allows for "auto selection" and switching of modems
    d->modemManager = new OfonoModemManager(parent);
    d->volumeManager = new OfonoCallVolume(AUTOSELECT,"");
    d->voicemailManager = new OfonoMessageWaiting(AUTOSELECT,"");
    d->callManager = new CallManager();

    // OfonoModemManager signals to monitor
    connect(d->modemManager, SIGNAL(modemAdded(const QString&)),
                             SLOT(onModemAdded(const QString&)));
    connect(d->modemManager, SIGNAL(modemRemoved(const QString&)),
                             SLOT(onModemRemoved(const QString&)));

    // CallManager signals to monitor
    connect(d->callManager, SIGNAL(validityChanged(bool)),
                            SLOT(onCallManagerValidityChanged(bool)));
    connect(d->callManager, SIGNAL(callsChanged()), SLOT(onCallsChanged()));

    // OfonoCallVolume signals to monitor
    connect(d->volumeManager, SIGNAL(validityChanged(bool)),
                              SLOT(onCallVolumeValidityChanged(bool)));

    // OfonoMessageWaiting signals to monitor
    connect(d->voicemailManager, SIGNAL(validityChanged(bool)),
                                 SLOT(onVoicemailValidityChanged(bool)));
    connect(d->voicemailManager, SIGNAL(voicemailWaitingChanged(bool)),
                                 SLOT(onVoicemailWaitingChanged(bool)));
    connect(d->voicemailManager, SIGNAL(voicemailMessageCountChanged(int)),
                                 SLOT(onVoicemailCountChanged(int)));

    // GConf Key change signals to monitor
    connect(&d->modesKey, SIGNAL(valueChanged()), SLOT(onModesChanged()));

    // ResourceProxy is a singleton, probably don't need to store this...
    d->policyManager = ResourceProxy::instance();

    if (d->callManager)
        qDebug() << __func__ << ": Using modem - "
                 << d->callManager->modem()->path();

    gContext = this;
}

/*
 * Public methods
 */
DialerContext::~DialerContext()
{
    delete d;
    d = 0;
    gContext=0;
}

DialerContext *DialerContext::instance()
{
    if (!gContext)
        gContext = new DialerContext();
    return gContext;
}

OfonoModemManager* DialerContext::modemManager() const
{
    return d->modemManager;
}

OfonoCallVolume* DialerContext::volumeManager() const
{
    return d->volumeManager;
}

OfonoMessageWaiting* DialerContext::voicemailManager() const
{
    return d->voicemailManager;
}

CallManager* DialerContext::callManager() const
{
    return d->callManager;
}

ResourceProxy* DialerContext::policyManager() const
{
    return d->policyManager;
}

QStringList DialerContext::modes() const
{
    return d->modes;
}

void DialerContext::setModes(const QStringList &modelist)
{
    d->modes = modelist;
    d->modes.removeDuplicates();
    emit modesChanged();
}

/*
OfonoModem* DialerContext::modem() const
{
    if (d->callManager)
        return d->callManager->modem();
    return NULL;
}
*/

/*
 * Private methods/slots
 */

void DialerContext::onModemAdded(const QString &path)
{
    TRACE
    // TODO: Handle modem additions, maybe...
    qWarning() << __func__ << ": Unhandled ModemAdded - " << path;
}

void DialerContext::onModemRemoved(const QString &path)
{
    TRACE
    // TODO: Handle modem removals, currently active for sure, others, maybe...
    qWarning() << __func__ << ": Unhandled ModemAdded - " << path;
}

void DialerContext::onCallVolumeValidityChanged(bool valid)
{
    TRACE
    // TODO: Reset the volumeManager service reference
    qWarning() << __func__ << ": valid? " << ((valid)?"true":"false");
}

void DialerContext::onVoicemailValidityChanged(bool valid)
{
    TRACE
    // TODO: Reset the voicemailManager service reference
    qWarning() << __func__ << ": valid? " << ((valid)?"true":"false");
}

void DialerContext::onVoicemailWaitingChanged(bool waiting)
{
    TRACE
    // TODO: Send notifications (or bubble this up for UI to handle?)
    qDebug() << __func__ << ": Messages? " << ((waiting)?"true":"false");
}

void DialerContext::onVoicemailCountChanged(int count)
{
    TRACE
    // TODO: Send notifications (or bubble this up for UI to handle?)
    qDebug() << __func__ << ": Message count == " << count;
}

void DialerContext::onCallManagerValidityChanged(bool valid)
{
    TRACE
    // TODO: Reset the callManager service reference
    qWarning() << __func__ << ": valid? " << ((valid)?"true":"false");
}

void DialerContext::onCallsChanged()
{
    TRACE
    // TODO: Send notifications (or bubble this up for UI to handle?)
    qDebug() << __func__ << ": Calls count == "
             << d->callManager->getCalls().count();
}

void DialerContext::onModesChanged()
{
    TRACE
    setModes(d->modesKey.value().toStringList());
    // Send notification of change
    emit modesChanged();
    qDebug() << __func__ << ": New modes == " << d->modes.join(", ");
}
