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

Item {
    id: root

    anchors.fill: parent

    MeeGoTouchStyle {id: style}

    property bool hideOnPress: true

    states: [
        State {
            name: 'shown'
            PropertyChanges {target: root; opacity: 1.0}
        },
        State {
            name: 'hidden'
            PropertyChanges {target: root; opacity: 0.0}
        }
    ]

    Behavior on opacity {PropertyAnimation {duration: 250}}

    Rectangle {
        id: blind
        anchors.fill: parent
        color: 'black'
        opacity: 0.7
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            if (hideOnPress) {
                root.state = 'hidden'
            }
        }
    }
}
