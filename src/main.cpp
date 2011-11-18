/*
 * dialer - MeeGo Voice Call Manager
 * Copyright (c) 2009, 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "dialercontext.h"
#include "dialerapplication.h"
#include "qmlmainwindow.h"

#include <QtGui>
#include <QApplication>
#include <MApplication>

#define CONFIG_KEY_TARGET_UX "/apps/dialer/ux"

#define CONFIG_VAL_TARGET_UX_MEEGOTOUCH             "meegotouch"
#define CONFIG_VAL_TARGET_UX_MEEGO_UX_COMPONENTS    "meego-ux-components"

#if !defined(CONFIG_DEFAULT_TARGET_UX)
#  define CONFIG_DEFAULT_TARGET_UX "meego-ux-components"
#endif

int main(int argc, char *argv[])
{
    MApplicationService *service = new MApplicationService(DBUS_SERVICE);
    DialerApplication app(argc, argv, service);

    qDebug() << "Initializing QML User eXperience!";
    QMLMainWindow::instance();

    // Command line '-mode' parameter takes priority over the previously
    // retrieved value from GConf or compile time default.
    if(app.arguments().contains("-mode"))
    {
        QStringList targetMode = DC->modes(); // Start with defaults
        int keyIdx = app.arguments().indexOf("-mode");

        if(keyIdx + 1 >= app.arguments().count())
        {
            qCritical() << "You must supply an option with -mode parameter";
            return EXIT_FAILURE;
        }

        // Add any additional runtime mode from -mode argument
        targetMode << app.arguments().at(keyIdx + 1).split(",");
        DC->setModes(targetMode);
    }

    qDebug() << "Modem path = " << DC->callManager()->modem()->path();

    return app.exec();
}

QString stripLineID(QString lineid)
{
    TRACE
    static QRegExp rx = QRegExp("([^0-9*#])");

    if (lineid.indexOf('+') == 0) {
        lineid.replace(rx, "");
        return lineid.insert(0,"+");
    }
    else
        return lineid.replace(rx, "");
}

// Returns a valid QDateTime if parsable as such, otherwise the result
// will be !isValid()
QDateTime qDateTimeFromOfono(const QString &val)
{
    TRACE
    QDateTime result;

    if (val.isEmpty())
        return result;

    // NOTE: Ofono formats time to string with the following format spec:
    //       %Y-%m-%dT%H:%M:%S%z (for example: "2001-10-19T10:32:30-05:00")

    // Start by trying to parse this as an ISODate "YYYY-MM-DDTHH:MM:SSTZD"
    result = QDateTime::fromString(val,Qt::ISODate);
#ifdef WANT_DEBUG
    qDebug() << QString("Converted %1 with Qt::ISODate: %2")
                       .arg(val)
                       .arg(result.toString());
#endif

    if (!result.isValid()) {
    // ISODate conversion has failed, fallback to manual method
    // NOTE: QDateTime::fromString(val, Qt::ISODate) Fails since the date
    //       format from Ofono is in RFC 822 form, but QDateTime can't parse it
        struct tm time_tm;
        QByteArray  bytes = val.toAscii();
        const char *str = bytes.constData();
        if (strptime(str, "%Y-%m-%dT%H:%M:%S%z", &time_tm) != NULL) {
            time_t t = mktime(&time_tm);
            if (t >= (time_t)(0)) {
                result.setTime_t(t);
#ifdef WANT_DEBUG
                qDebug() << QString("Converted %1 with strptime: %2")
                                   .arg(val)
                                   .arg(result.toString());
#endif
            }
        }

        if (!result.isValid())
            qCritical() << QString("Format error, unknown date/time: %1")
                                  .arg(str);
    }

    return result;
}
