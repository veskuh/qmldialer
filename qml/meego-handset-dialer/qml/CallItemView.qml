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
import QtMobility.contacts 1.1

import 'base'

Item {
    id: root
    width: parent.width * 0.9;
    height: childrenRect.height

    property variant call: null
    property alias callMsisdn: largeView.callMsisdn
    property alias callName: largeView.callName

    state: 'disconnected'

    onCallChanged: {
        if (call) {
            root.state = call.state;

            call.stateChanged.connect(handleStateChange)
        }
        else {
            root.reset()
        }
    }

    function handleStateChange(state) {
        root.state = state
    }

    function reset() {
        root.state = "disconnected"
    }

    onStateChanged: {
        if (root.state == "disconnected") {
            root.call = null
        }
    }

    Rectangle {
        id: background
        anchors {fill: parent; margins: 10}
        radius: 15
        smooth: true

        gradient: Gradient {
            GradientStop {position: 0.0; color: '#4f4f4f'}
            GradientStop {position: 1.0; color: '#000000'}
        }
    }

    CallItemViewLarge {
        id: largeView
        call: parent.call
        state: parent.state
    }
}
