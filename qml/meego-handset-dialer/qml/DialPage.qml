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

Page
{
    id: root

    DialNumberEntry {
        id: numberEntry

        width: parent.width
        height: 72
        anchors {top: parent.top; margins: 10}
    }

    ListView {
        width: parent.width
        clip: true
        anchors {top: numberEntry.bottom; bottom: numPad.top; margins: 10}
        model: callModel
        delegate: callDelegate

        onCountChanged: {
            if (count == 0) {
                numPad.show();
            }
        }
    }

    DialNumPad {
        id: numPad

        width: parent.width
        anchors {bottom: parent.bottom}
        entry: numberEntry
    }

    Component {
        id: callDelegate

        CallItemView {
            call: newCall
            anchors.horizontalCenter: parent.horizontalCenter

            onStateChanged: {
                if (state == "disconnected") {
                    callModel.remove(index)
                }
            }
        }
    }
}
