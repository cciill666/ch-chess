import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15
import chinesechess 1.0

Rectangle {
    anchors.fill: parent
    color: "#e9e9e9"
    Image {
        anchors.fill: parent
        source: "qrc:/images/background/game.jpg"
        fillMode: Image.PreserveAspectCrop
        z: 0
    }
    property bool skipBoardTap: false

    // 顶部状态栏
    Rectangle {
        id: statusBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        color: "#cc000000"
        z: 20

        Row {
            anchors.centerIn: parent
            spacing: 30

            Text {
                text: "当前回合: " + (ChessManager.currentTurn === ChessPiece.Camp.Red ? "红方" : "黑方")
                color: ChessManager.currentTurn === ChessPiece.Camp.Red ? "#e53935" : "#424242"
                font.pixelSize: 16
                font.bold: true
            }

            Text {
                text: ChessManager.onlineState
                      ? ("本方为: " + (ChessManager.myCamp === ChessPiece.Camp.Red ? "红方" : "黑方"))
                      : "本地对战"
                color: "#fff"
                font.pixelSize: 14
            }

            Text {
                text: ChessManager.connectionStatus
                color: ChessManager.onlineState ? "#81c784" : "#ef5350"
                font.pixelSize: 14
            }
        }

        Button {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            text: "返回首页"
            width: 90
            height: 30
            onClicked: parent.parent.parent.currentPage = 0
        }
    }

    Item {
        id: boardContainer
        anchors.centerIn: parent
        width: ChessManager.board.boardWidth
        height: ChessManager.board.boardHeight
        Image {
            id: boardBg
            anchors.fill: parent
            source: ChessManager.board.boardImg
            z: 0
            clip: true
            TapHandler {
                onTapped: function(eventPoint) {
                    if (skipBoardTap) {
                        skipBoardTap = false
                        return
                    }
                    const pos = eventPoint.position
                    let col = Math.round((pos.x - 25) / 50)
                    let row = Math.round((pos.y - 25) / 50)
                    col = Math.max(0, Math.min(8, col))
                    row = Math.max(0, Math.min(9, row))
                    console.log("【棋子】col=", col, "row=", row)
                    ChessManager.moveSelectedPiece(col, row)
                }
            }
        }
        Repeater {
            model: ChessManager.pieces
            delegate: Image {
                width: modelData.pieceW
                height: modelData.pieceH
                x: modelData.posX
                y: modelData.posY
                z: 10
                source: modelData.imgSource
                visible: modelData.isAlive
                TapHandler {
                    onTapped: {
                        skipBoardTap = true
                        console.log("【选中】ID =", modelData.pieceId)
                        ChessManager.selectPiece(modelData)
                    }
                }
            }
        }
    }
    ResultDialog { id: resDlg }
    Connections {
        target: ChessManager
        function onGameOver(txt){
            resDlg.showText = txt
            resDlg.visible = true
        }
    }
}
