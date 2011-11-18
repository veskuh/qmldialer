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

Item
{
    property string placeHolderText: qsTr("Enter Number")
    property TextInput textInput: input

    id: root

    function clear()
    {
        input.color = style.foregroundLight;
        input.text = placeHolderText;
    }

    function isBlank()
    {
        return (input.text == placeHolderText);
    }

    function appendChar(character)
    {
        if(input.text == placeHolderText) {
            input.text = character;
        } else {
            var pos = input.cursorPosition;
            input.text = input.text.substring(0, pos) + character + input.text.substring(pos, input.text.length);
            input.cursorPosition = pos + 1;
        };
    }

    TextInput
    {
        id: input
        anchors {top: parent.top; left: parent.left; right: bksp.left; margins: 10}
        color: style.foregroundLight
        cursorVisible: false
        activeFocusOnPress: false
        inputMethodHints: Qt.ImhDialableCharactersOnly
        font {pixelSize: 42}
        text: placeHolderText

        Component.onCompleted: forceActiveFocus();

        onTextChanged: {
            if(text.length == 0) root.clear();

            if(text.length > placeHolderText.length && text.substr(0, placeHolderText.length) == placeHolderText)
            {
              text = text.substr(placeHolderText.length);
            }

            if(text.length < placeHolderText.length && placeHolderText.substr(0, text.length) == text)
            {
              text = placeHolderText;
            }

            if(text == placeHolderText)
            {
                color = style.foregroundLight;
            }
            else
            {
                color = style.foreground;
            }
        }

        onAccepted: {
          main.dial(text)
        }
    }

    Image
    {
        id: bksp
        anchors {top: parent.top; right: parent.right; margins: 24}
        source: style.icon('icon-m-common-backspace')
        MouseArea
        {
            anchors.fill: parent
            onClicked: {
                if(input.text == placeHolderText) return;
                var pos = input.cursorPosition;
                var text = input.text.substring(0, pos - 1);
                text = text + input.text.substring(pos, input.text.length);
                input.text = text;
                input.cursorPosition = pos - 1;
            }
            onPressAndHold: root.clear();
        }
    }
}
