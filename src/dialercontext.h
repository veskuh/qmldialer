/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef DIALERCONTEXT_H
#define DIALERCONTEXT_H

// libofono-qt headers
#include <ofonomodemmanager.h>
#include <ofonocallvolume.h>
#include <ofonomessagewaiting.h>

// local headers
#include "callmanager.h"
#include "resourceproxy.h"

// Convienence Macro for access to Class instance
#define DC DialerContext::instance()

class DialerContext: public QObject
{
    Q_OBJECT

    Q_PROPERTY(OfonoModemManager* modemManager READ modemManager)
    Q_PROPERTY(OfonoCallVolume* volumeManager READ volumeManager)
    Q_PROPERTY(OfonoMessageWaiting* voicemailManager READ voicemailManager)
    Q_PROPERTY(CallManager* callManager READ callManager)
    Q_PROPERTY(ResourceProxy* policyManager READ policyManager)
    Q_PROPERTY(QStringList modes READ modes WRITE setModes)

public:
    virtual ~DialerContext();

    static DialerContext *instance();

    OfonoModemManager*   modemManager() const;
    OfonoCallVolume*     volumeManager() const;
    OfonoMessageWaiting* voicemailManager() const;
    CallManager*         callManager() const;
    ResourceProxy*       policyManager() const;
    QStringList          modes() const;

public slots:
    // Slot to set current mode at runtime
    void setModes(const QStringList &modeList);

Q_SIGNALS:
    void modesChanged();

private Q_SLOTS:

    // Slots to handle signals from Manager oFono service
    void onModemAdded(const QString &path);
    void onModemRemoved(const QString &path);

    // Slots to handle signals from CallVolume oFono service
    void onCallVolumeValidityChanged(bool valid);

    // Slots to handle signals from MessageWaiting oFono service
    void onVoicemailValidityChanged(bool valid);
    void onVoicemailWaitingChanged(bool waiting);
    void onVoicemailCountChanged(int count);

    // Slots to handle signals from CallManager oFono service
    void onCallManagerValidityChanged(bool valid);
    void onCallsChanged();

    // Slot to handle runtime mode changes in GConf key
    void onModesChanged();

protected:
    DialerContext(QObject *parent = 0);

private:
    DialerContext(const DialerContext&);
    DialerContext& operator= (DialerContext&);

    class DialerContextPrivate *d;

    static DialerContext *gContext;
};

#endif // DIALERCONTEXT_H
