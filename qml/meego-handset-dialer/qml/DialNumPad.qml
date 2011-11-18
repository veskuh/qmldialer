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

Item
{
    id: root

    property bool numPadShown: true
    property DialNumberEntry entry

    height: 72 * 5 + 4

    function handleButtonClick(text)
    {
        insertText(text)
        if (adapter.currentCall) {
            if (adapter.currentCall.state == 'active') {
                adapter.sendTones(text)
            }
        }
    }

    function insertText(text)
    {
        entry.appendChar(text)
    }

    function show()
    {
        height = 72 * 5 + 4;
        numPadShown = true;
    }

    function hide()
    {
        height = 72;
        numPadShown = false;
    }

    function toggle()
    {
        if(numPadShown == true) {hide()} else {show()}
    }

    Behavior on height {PropertyAnimation {duration: 500; easing.type: Easing.OutBounce}}

    Item
    {
        id: numpad
        width: parent.width; height: 72 * 4

        Behavior on opacity {PropertyAnimation {duration: 500}}

        Column
        {
            anchors.fill: parent

            Row
            {
                width: parent.width - 10
                anchors.horizontalCenter: parent.horizontalCenter
                DialNumPadButton {
                    text: qsTr("1");
                    detail: "voicemail"; //TODO: Change to icon
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialMailbox();
                }
                DialNumPadButton {
                    text: qsTr("2");
                    detail: qsTr("abc");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(1);
                }
                DialNumPadButton {
                    text: qsTr("3");
                    detail: qsTr("def");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(2);
                }
            }
            Row
            {
                width: parent.width - 10
                anchors.horizontalCenter: parent.horizontalCenter
                DialNumPadButton {
                    text: qsTr("4");
                    detail: qsTr("ghi");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(3);
                }
                DialNumPadButton {
                    text: qsTr("5");
                    detail: qsTr("jkl");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(4);
                }
                DialNumPadButton {
                    text: qsTr("6");
                    detail: qsTr("mno");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(5);
                }
            }
            Row
            {
                width: parent.width - 10
                anchors.horizontalCenter: parent.horizontalCenter
                DialNumPadButton {
                    text: qsTr("7");
                    detail: qsTr("pqrs");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(6);
                }
                DialNumPadButton {
                    text: qsTr("8");
                    detail: qsTr("tuv");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(7);
                }
                DialNumPadButton {
                    text: qsTr("9");
                    detail: qsTr("wxyz");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: main.dialSpeedDial(8);
                }
            }
            Row
            {
                width: parent.width - 10
                anchors.horizontalCenter: parent.horizontalCenter
                DialNumPadButton {
                    text: qsTr("*");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: root.handleButtonClick("p");
                }
                DialNumPadButton {
                    text: qsTr("0");
                    detail: qsTr("+");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: root.handleButtonClick("+");
                }
                DialNumPadButton {
                    text: qsTr("#");
                    onClicked: root.handleButtonClick(text);
                    onPressAndHold: root.handleButtonClick("w");
                }
            }
        }
    }

    Item
    {
        id: actions

        width: parent.width; height: 74
        anchors {bottom: parent.bottom}

        Rectangle
        {
            anchors.fill: parent
            color: style.background
        }

        Item
        {
            anchors {fill: parent; margins: 4}

            ToolButton {
                id: bAddContact;
                anchors {left: parent.left}
                width: parent.width / 4; height: parent.height
                radius: 10
                background: style.windowToolsNormal
                icon: style.icon('icon-m-common-add-contact')
            }
            ToolButton {
                id: bCall;
                anchors {left: bAddContact.right; right: bViewToggle.left; margins: 5}
                height: parent.height
                radius: 10
                icon: style.icon('icon-m-telephony-call');

                onClicked: {
                    if(adapter.currentCall) {
                        if(adapter.currentCall.isMultiparty) {
                            adapter.hangupMultiparty();
                        } else {
                            adapter.currentCall.hangup();
                        }

                        bCall.background = '#8fef8f';
                        bCall.icon = style.icon('icon-m-telephony-call');
                    } else {
                        if (entry.isBlank()) {
                            console.log("*** QML *** :: You can not dial without a number!");
                            main.showErrorMessage(qsTr("You can't dial without a number!"));
                            return;
                        }

                        if (main.dial(entry.textInput.text)) {
                            entry.clear();
                        }
                    }
                }

                background: '#8fef8f'

                Component.onCompleted: {
                    adapter.callsChanged.connect(
                                function() {
                                    if(!adapter.currentCall) {
                                        bCall.background = '#8fef8f';
                                        bCall.icon = style.icon('icon-m-telephony-call');
                                    } else {
                                        bCall.background = '#ef4f4f';
                                        bCall.icon = style.icon('icon-m-telephony-call-end');
                                    }
                                });

                    main.callChanged.connect(function() {
                        if(adapter.currentCall) {
                            bCall.background = '#ef4f4f';
                            bCall.icon = style.icon('icon-m-telephony-call-end');
                        } else {
                            bCall.background = '#8fef8f';
                            bCall.icon = style.icon('icon-m-telephony-call');
                        }
                    });
                }
            }
            ToolButton {
                id: bViewToggle;
                anchors {right: parent.right}
                width: parent.width / 4; height: parent.height
                radius: 10
                background: style.windowToolsNormal
                icon: style.icon('icon-m-telephony-dialer');
                onClicked: root.toggle()
            }
        }
    }
}
