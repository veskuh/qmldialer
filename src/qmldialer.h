/*
 * dialer - Declarative Dialer Adapter.
 * Copyright (c) 2011, Tom Swindell.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef QMLDIALER_H
#define QMLDIALER_H

#include <QObject>

#include "qmlcallitem.h"

class QMLDialer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString mailbox READ mailbox WRITE setMailbox)
    Q_PROPERTY(QMLCallItem* currentCall READ currentCall)
    Q_PROPERTY(bool modemOnline READ modemOnline WRITE setModemOnline)
    Q_PROPERTY(int callCount READ callCount)
    Q_PROPERTY(int multipartyCallCount READ multipartyCallCount)

public:
    explicit QMLDialer(QObject *parent = 0);
            ~QMLDialer();

    QString      mailbox        () const;
    bool         modemOnline    ();
    QMLCallItem* currentCall         () const;
    int          callCount           () const;
    int          multipartyCallCount () const;

Q_SIGNALS:
    void callsChanged();
    void incomingCall();

public Q_SLOTS:
    void setMailbox(const QString &number);
    void setModemOnline(bool online);

    QString speedDial(int index) const;
    void setSpeedDial(int index, const QString &number);

    void dial(const QString &msisdn);
    void hangupDialingCall();
    void hangupAll();

    void mutePressed(bool checked);
    void silenceRingtone();
    void multiPressed();

    void sendTones(const QString &tones);

    void transfer();
    void swapCalls();
    void releaseAndAnswer();
    void holdAndAnswer();

    void createMultiparty();
    void hangupMultiparty();
    void privateChat(const QMLCallItem &callitem);

protected Q_SLOTS:
    void connectAll();

    void onCallsChanged();
    void onIncomingCall(CallItem *callitem);

    void onCallManagerChanged();

private:
    class QMLDialerPrivate *d;
};

#endif // QMLDIALER_H
