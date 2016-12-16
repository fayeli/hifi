import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
    id: tabletButton
    property string text: "EDIT"
    property string icon: "icons/edit-icon.svg"
    width: 129
    height: 129

    signal clicked()

    Rectangle {
        id: buttonBg
        color: "#2b2b2b"
        opacity: 0.2
        radius: 8
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
    }

    Rectangle {
        id: buttonOutline
        color: "#00000000"
        opacity: 0.2
        radius: 8
        z: 1
        border.width: 2
        border.color: "#ffffff"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
    }


    Image {
        id: icon
        width: 60
        height: 60
        anchors.bottom: text.top
        anchors.bottomMargin: 5
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.Stretch
        source: "../../../" + tabletButton.icon
    }

    Text {
        id: text
        color: "#ffffff"
        text: tabletButton.text
        font.bold: true
        font.pixelSize: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: tabletButton.clicked();
        onEntered: {
            console.log("Tablet Button Hovered!");
            tabletButton.state = "hover state";
        }
        onExited: {
            console.log("Tablet Button Unhovered!");
            tabletButton.state = "base state";
        }
    }

    states: [
        State {
            name: "hover state"

            PropertyChanges {
                target: buttonOutline
                border.color: "#1fc6a6"
                opacity: 1
            }
        }
    ]
}


