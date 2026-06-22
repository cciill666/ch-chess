import QtQuick 2.15
import QtQuick.Controls 2.15
import chinesechess 1.0

 Rectangle {
     anchors.fill: parent
     color: "#f5f5f5"
     Column {
         anchors.centerIn: parent
         spacing: 35
         Text {
             text: "中国象棋"
             font.pixelSize: 42
             font.bold: true
         }
         Button {
             text: "开始对局"
             width: 220
             height: 50
             onClicked: ChessManager.initChess()
         }
         Row {
             spacing: 25
             Button {
                 text: "创建房间"
                 width: 180
                 height: 45
                 onClicked: ChessManager.createServer(8888)
             }
             Button {
                 text: "加入房间"
                 width: 180
                 height: 45
                 onClicked: ChessManager.connectServer("127.0.0.1", 8888)
             }
         }
     }
 }