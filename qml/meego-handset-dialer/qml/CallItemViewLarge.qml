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
import 'javascripts/framework.js' as Support

Item {
    id: root

    width: parent.width
    height: childrenRect.height

    anchors.centerIn:parent

    property variant call: null
    property string callDuration: "00:00:00"
    property string callerLabelText: ""
    property string callMsisdn: ""
    property string callName: ""

    state: "disconnected"

    onCallChanged: {
        if (call) {
            root.callName = call.name
            root.callMsisdn = call.msisdn
        }
        else {
            root.reset()
        }
    }

    onStateChanged: {
        if (state == "held") {
            holdButton.hold = true
        }
        else {
            holdButton.hold = false
        }
    }

    function reset() {
        root.callDuration = "00:00:00"
        root.callName = ""
        root.callMsisdn = ""
    }

    onCallMsisdnChanged: {
        var contact = main.getContactByPhoneNumber(callMsisdn);

        if(contact) {
            root.callerLabelText = contact.displayLabel;
        } else if (callName) {
            root.callerLabelText = callName;
        } else {
            root.callerLabelText = callMsisdn;
        }
    }

    Timer {
        interval: 1000; repeat: true; running: true;

        onTriggered: {
            if(call) {
                if(call.duration && call.duration > 0) {
                    callDuration = Support.friendlyInterval(call.duration);
                }
            }
        }
    }

    states {
        State {
            name: 'active'
            PropertyChanges {target: activeTools; visible: true}
            PropertyChanges {target: answerButton; visible: false}
            PropertyChanges {target: hangupButton; visible: false}
        }

        State {
            name: 'held'
            PropertyChanges {target: activeTools; visible: true}
            PropertyChanges {target: answerButton; visible: false}
            PropertyChanges {target: hangupButton; visible: false}
        }

        State {
            name: 'dialing'
            PropertyChanges {target: activeTools; visible: false}
            PropertyChanges {target: answerButton; visible: false}
            PropertyChanges {target: hangupButton; visible: true}
        }

        State {
            name: 'alerting'
            PropertyChanges {target: activeTools; visible: false}
            PropertyChanges {target: answerButton; visible: false}
            PropertyChanges {target: hangupButton; visible: true}
        }

        State {
            name: 'incoming'
            PropertyChanges {target: activeTools; visible: false}
            PropertyChanges {target: answerButton; visible: true}
            PropertyChanges {target: hangupButton; visible: true}
        }

        State {
            name: 'waiting'
            PropertyChanges {target: activeTools; visible: false}
            PropertyChanges {target: answerButton; visible: false}
            PropertyChanges {target: hangupButton; visible: true}
        }

        State {
            name: 'disconnected'
            PropertyChanges {target: activeTools; visible: false}
            PropertyChanges {target: answerButton; visible: false}
            PropertyChanges {target: hangupButton; visible: false}
        }
    }

    Text {
        id:callerIdText
        anchors {top:parent.top;horizontalCenter:parent.horizontalCenter;margins:40}
        color:'white'
        font.pixelSize:32
        text:root.callerLabelText
    }

    Text {
        id:durationText
        anchors {top:callerIdText.bottom;horizontalCenter:parent.horizontalCenter;margins:10}
        color:'#dfdfdf'
        font.pixelSize:28
        text:root.callDuration
    }

    Item {
        id: activeTools
        height: holdButton.height
        anchors {bottom: hangupButton.top; left: parent.left; right: parent.right; margins: 10; leftMargin: 24; rightMargin: 24}

        CallButton {
            id: holdButton

            property bool hold: false

            width: parent.width / 4
            anchors {left: parent.left}
            icon: hold ? style.icon('icon-m-common-play') : style.icon('icon-m-telephony-pause')

            onClicked: {
                adapter.swapCalls()
            }
        }

        CallButton {
            id: muteButton

            width: parent.width / 4 - 7.5
            anchors {left: holdButton.right; leftMargin: 5}
            icon: main.mute ? style.icon('icon-m-telephony-volume') : style.icon('icon-m-telephony-volume-off')

            onClicked: {
                main.mute = !main.mute
                adapter.mutePressed(main.mute);
            }
        }

        CallButton {
            id: speakerButton

            width: parent.width / 4 - 7.5
            anchors {left: muteButton.right; leftMargin: 5}
            text: main.speaker ? qsTr("Handset") : qsTr("Speaker")

            onClicked: {
                main.speaker = !main.speaker
            }

        }

        CallButton {
            id: combineButton

            anchors {left: holdButton.right; right: parent.right; leftMargin: 12}
            icon: style.icon('icon-m-telephony-call-combine')

            onClicked: {
                adapter.multiPressed()
            }
        }
    }

    CallButton {
        id: answerButton

        anchors {bottom: hangupButton.top; left: parent.left; right: parent.right; margins: 24}
        icon: style.icon('icon-m-telephony-call-answer')

        gradient: Gradient {
            GradientStop {position: 0.0; color: '#dfffdf'}
            GradientStop {position: 1.0; color: '#4f8f4f'}
        }

        onClicked: {
            adapter.currentCall.answer();
        }
    }

    Loader {
        sourceComponent: hangupButtonComponent
        id: hangupButton
        anchors {bottom: parent.bottom; left: parent.left; right: parent.right; margins: 24}
    }

    Component {
        id: hangupButtonComponent

        CallButton {
            icon: style.icon('icon-m-telephony-call-end')

            gradient: Gradient {
                GradientStop {position: 0.0; color: '#ffdfdf'}
                GradientStop {position: 1.0; color: '#8f4f4f'}
            }
            onClicked: {
                if (call) {
                    if (call.isMultiparty) {
                        adapter.hangupMultiparty();
                    }
                    else {
                        call.hangup();
                    }
                }
                else if (root.state == "dialing") {
                    adapter.hangupDialingCall();
                }
                root.parent.reset();
            }
        }
    }
}

