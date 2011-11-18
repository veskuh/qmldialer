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
import com.meego.dialer 1.0

import 'base'
import 'javascripts/framework.js' as Support

Item
{
    id: main
    width: 760

    property bool mute: false
    property bool speaker: false

    property variant call

    Dialer { id: adapter } // TODO: Refactor

    ContactModel {
        id: contactModel

        autoUpdate: true

        filter: DetailFilter {
            detail: ContactDetail.PhoneNumber
            field: PhoneNumber.Number
        }
    }

    //model used by the callView on the DialPage
    ListModel {
        id: callModel
    }

    //have to use this timer when dialing so that the active call actually gets put on hold properly
    Timer {
        id: dialTimer
        interval: 500; repeat: false; running: false;

        property string msisdn: ""

        onTriggered: {
            if (msisdn.trim().length > 0) {
                dialingCall.sourceComponent = blockingCall
                dialingCall.item.view.state = 'dialing'
                dialingCall.item.view.callMsisdn = msisdn

                adapter.dial(msisdn);
            }
        }
    }

    Component.onCompleted: {
        adapter.callsChanged.connect(handleCallsChanged)
        adapter.incomingCall.connect(handleIncomingCall)
        controller.setAdapter(adapter)
        handleIncomingCall()
    }

    function handleCallsChanged() {
        if(!adapter.currentCall) {

            return;
        }

        if(adapter.currentCall.state == "dialing" || adapter.currentCall.state == "alerting") {
            dialingCall.sourceComponent = blockingCall;
            dialingCall.item.view.call = adapter.currentCall;
        }
    }

    function handleIncomingCall() {
        main.jumpTo(0);
        controller.show();

        if (adapter.currentCall) {
            var call = adapter.currentCall
            if (call.state == "incoming") {
                incomingCall.sourceComponent = blockingCall
                incomingCall.item.view.call = call
                main.call = call
            }
            else {
                //check if we already have this call
                var tmpCall
                for(var i=0; i < callModel.count; i++) {
                    tmpCall = callModel.get(i).newCall
                    if (tmpCall) {
                        if (tmpCall.path == call.path) {
                            return
                        }
                    }
                }
                //add the call to our model
                callModel.insert(0, {"newCall": call})
                main.call = call;

                //destroy the incomingCall blocking component
                if (incomingCall.item) {
                    if (incomingCall.item.view.call) {
                        if (incomingCall.item.view.call.path == call.path) {
                            incomingCall.sourceComponent = undefined
                            main.call = null;
                        }
                    }
                }
                //destroy the dialingCall blocking component
                if (dialingCall.item) {
                    if (dialingCall.item.view.callMsisdn == call.msisdn) {
                        dialingCall.sourceComponent = undefined
                        main.call = null;
                    }
                }
            }
        }
    }

    function showErrorMessage(mesg) {
        mesgDialog.mesg = mesg;
        mesgDialog.state = 'shown';
    }

    function getContactByPhoneNumber(number) {
        var result = null;

        for (var i = 0; i < contactModel.contacts.length; i++) {
            var contact = contactModel.contacts[i];

            for(var j = 0; j < contact.phoneNumbers.length; j++)
            {
                //stip out all non-numeric charaters from each before compare
                //only compare last six digits
                if(contact.phoneNumbers[j].number.replace(/[^\d.]/g, '').substr(-6) == number.replace(/[^\d.]/g, '').substr(-6))
                {
                    result = contact;
                    break;
                }
            }

            if(result) break;
        }

        return result;
    }

    function dial(msisdn) {
        if (msisdn.trim().length == 0) {
            showErrorMessage(qsTr("No number specified!"));
            return false;
        }
        //check that we don't already have 2 active calls
        if ((adapter.multipartyCallCount > 0 && (adapter.callCount - adapter.multipartyCallCount) >= 1) || (adapter.multipartyCallCount == 0 && adapter.callCount >= 2)) {
            showErrorMessage(qsTr("There are already two active calls."));
            return false;
        }
        if (!adapter.modemOnline)
        {
          showErrorMessage(qsTr("modem is not available or powered down!"));
          return false;
        }
        //hold the active call before dialing
        if (adapter.currentCall) {
            adapter.swapCalls()
        }

        dialTimer.stop()
        dialTimer.msisdn = msisdn
        dialTimer.start()

        return true;
    }

    function dialMailbox() {
        if (!adapter.currentCall) {
            if (adapter.mailbox) {
                console.log("*** QML *** :: Attempting to call mailbox number: " + adapter.mailbox);
                main.dial(adapter.mailbox);
            }
            else {
                console.log("*** QML *** :: No mailbox number defined!");
                showErrorMessage(qsTr("No mailbox number defined."));
            }
        }
    }

    function dialSpeedDial(index) {
        if (!adapter.currentCall) {
            if (adapter.speedDial(index)) {
                console.log("*** QML *** :: Calling speed dial " + index + ": " + adapter.speedDial(index));
                main.dial(adapter.speedDial(index));
            }
            else {
                console.log("*** QML *** :: No speed dial number defined for: " + index);
                showErrorMessage(qsTr("No speed dial for " + (index + 1)));
            }
        }
    }

    function jumpTo(idx) {
        switcher.jumpTo(idx);
        for(var i = 0; i < menubar.items.length; i++) {
            menubar.items[i].selected = (i == idx);
        }
    }

    function switchTo(idx) {
        switcher.switchTo(idx);
        for(var i = 0; i < menubar.items.length; i++) {
            menubar.items[i].selected = (i == idx);
        }
    }

    MeeGoTouchStyle {id: style}

    Rectangle {anchors.fill: parent; color: style.background}

    Menubar
    {
        id: menubar

        width: parent.width; height: 72

        items
        {
            MenubarButton {
                width: 70;
                icon: style.icon('icon-m-common-phone')
                onClicked: {main.switchTo(0)}
            }
            MenubarButton {
                width: 70;
                icon: style.icon('icon-m-common-clock')
                onClicked: {main.switchTo(1)}
            }
            /*MenubarButton {
                width: 70;
                icon: style.icon('icon-m-telephony-addressbook')
                onClicked: {main.switchTo(2)}
            }
            MenubarButton {
                width: 70;
                icon: style.icon('icon-m-content-favourites')
                onClicked: {main.switchTo(3)}
            }*/
        }
    }

    PageSwitcher
    {
        id: switcher

        width: parent.width
        anchors {top: menubar.bottom; bottom: parent.bottom}

        pages
        {
            DialPage {id: dialpage}
            CallHistoryPage {id: historypage}
            //ContactsPage {id: contactspage}
            //FavouritesPage {id: favouritespage}
        }
    }

    CallContextDialog {
        id: callContextDialog
        detail: "!! NAME"
        number: "!! NUMBER"
        state: 'hidden'
    }

    MessageDialog {
        id: mesgDialog
        state: 'hidden'
    }

    Loader {
        id: dialingCall
        anchors.fill: parent
    }

    Loader {
        id: incomingCall
        anchors.fill: parent
    }

    Component {
        id: blockingCall

        AbstractDialog {
            property alias view: callView
            anchors.fill: parent
            hideOnPress: false

            CallItemView {
                id: callView
                anchors.centerIn: parent

                onVisibleChanged: {
                    parent.state = visible ? "shown" : "hidden"
                }

                onStateChanged: {
                    if (state == "disconnected") {
                        visible = false
                    }
                    else {
                        visible = true
                    }
                }
            }
        }
    }
}

