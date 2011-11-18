/*
 * dialer - QML User Interface Component
 *
 * Copyright (c) 2011, Tom Swindell.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

import Qt 4.7
import 'base'

AbstractDialog {
    id: root

    property string mesg: ''

    MeeGoTouchStyle {id: style}

    Rectangle {
        id: dialog
        width: parent.width * 0.8; height: 300;
        anchors.centerIn: parent
        color: style.background
        radius: 15
        smooth: true

        Text {
            id: mesgText
            width: parent.width * 0.9
            anchors.centerIn: parent
            wrapMode: Text.WordWrap
            color: style.foreground
            font.pixelSize: 32
            text: "<center>" + root.mesg + "</center>"
        }
    }
}
