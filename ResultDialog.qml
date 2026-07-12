import QtQuick
import QtQuick.Controls
import chinesechess 1.0

Rectangle {
    id: root
    width: 340
    height: 210
    radius: 10
    color: "#fff"
    border.color: "#bbb"
    visible: false
    anchors.centerIn: parent
    z: 100
    property string showText: ""

    Image {
        anchors.fill: parent
        source: "qrc:/images/background/result.jpg"
        fillMode: Image.PreserveAspectCrop
        z: 0
    }

    Column {
        anchors.centerIn: parent
        spacing: 30
        z: 1
        Text {
            text: root.showText
            font.pixelSize: 24
            font.bold: true
        }
        Row {
            spacing: 25
            Button {
                text: "关闭"
                width: 110
                onClicked: Qt.quit()
            }
            Button {
                text: "返回首页"
                width: 110
                onClicked: {
                    root.visible = false
                    ChessManager.requestGoStartPage()
                }
            }
        }
    }
}
