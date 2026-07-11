import QtQuick
import QtQuick.Controls
import chinesechess 1.0

Rectangle {
    anchors.fill: parent
    color: "#f5f5f5"
    Image {
        anchors.fill: parent
        source: "qrc:/images/background/start.webp"
        fillMode: Image.PreserveAspectCrop
        z: 0
    }
    Column {
        anchors.centerIn: parent
        spacing: 25
        z: 1

        Text {
            text: "中国象棋"
            font.pixelSize: 42
            font.bold: true
            color: "#333"
        }

        Button {
            text: "开始对局"
            width: 220
            height: 50
            onClicked: ChessManager.initChess()
        }

        Button {
            text: "人机对战"
            width: 220
            height: 50
            onClicked: ChessManager.initAiChess()
        }

        // 联机区域
        Rectangle {
            width: 420
            height: 200
            color: "#ccffffff"
            radius: 10
            border.color: "#aaa"
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 15

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "联机对战"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#444"
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        text: "IP地址:"
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#444"
                    }
                    TextField {
                        id: ipField
                        width: 140
                        height: 35
                        text: "127.0.0.1"
                        placeholderText: "输入IP"
                    }
                    Text {
                        text: "端口:"
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#444"
                    }
                    TextField {
                        id: portField
                        width: 70
                        height: 35
                        text: "8888"
                        placeholderText: "端口"
                        validator: IntValidator { bottom: 1; top: 65535 }
                    }
                }

                Row {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: "创建房间"
                        width: 120
                        height: 40
                        onClicked: ChessManager.createServer(parseInt(portField.text))
                    }
                    Button {
                        text: "加入房间"
                        width: 120
                        height: 40
                        onClicked: ChessManager.connectServer(ipField.text, parseInt(portField.text))
                    }
                    Button {
                        text: "断开连接"
                        width: 120
                        height: 40
                        enabled: ChessManager.onlineState
                        onClicked: ChessManager.disconnectNetwork()
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: ChessManager.connectionStatus
                    color: ChessManager.onlineState ? "#2e7d32" : "#c62828"
                    font.pixelSize: 14
                }
            }
        }
    }
}
