import QtQuick
import QtQuick.Controls
import chinesechess 1.0

Rectangle {
    id: gamePage
    anchors.fill: parent
    color: "#e9e9e9"

    // 背景图
    Image {
        anchors.fill: parent
        source: "qrc:/images/background/game.jpg"
        fillMode: Image.PreserveAspectCrop
        z: 0
    }

    property bool skipBoardTap: false

    // ========== 顶部状态栏 ==========
    Rectangle {
        id: statusBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 50
        color: "#cc000000"
        z: 20

        Row {
            id: statusRow
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            spacing: 25

            // 回合指示
            Text {
                text: "回合"
                color: "#aaaaaa"
                font.pixelSize: 13
            }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 10
                height: 10
                radius: 5
                color: ChessManager.currentTurn === ChessPiece.Camp.Red ? "#e53935" : "#333333"
            }
            Text {
                text: ChessManager.currentTurn === ChessPiece.Camp.Red ? "红方" : "黑方"
                color: ChessManager.currentTurn === ChessPiece.Camp.Red ? "#e53935" : "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }

            // 分隔线
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 1
                height: 24
                color: "#555555"
            }

            // 模式显示
            Text {
                text: ChessManager.onlineState
                      ? ("联机 - " + (ChessManager.myCamp === ChessPiece.Camp.Red ? "红方" : "黑方"))
                      : ChessManager.aiMode
                        ? "人机对战"
                        : "本地对战"
                color: "#ffffff"
                font.pixelSize: 13
            }

            // AI思考提示
            Text {
                text: ChessManager.aiThinking ? "AI 思考中..." : ""
                color: "#ffeb3b"
                font.pixelSize: 13
                visible: ChessManager.aiThinking
            }

            // 连接状态
            Text {
                text: ChessManager.connectionStatus !== "" && !ChessManager.aiMode
                      ? ChessManager.connectionStatus : ""
                color: ChessManager.onlineState ? "#81c784" : "#ef5350"
                font.pixelSize: 12
                visible: !ChessManager.aiMode && ChessManager.connectionStatus !== ""
            }
        }
    }

    // ========== 棋盘区域 ==========
    Item {
        id: boardArea
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 15
        width: ChessManager.board.boardWidth + 20
        height: ChessManager.board.boardHeight + 20

        // 棋盘边框装饰
        Rectangle {
            id: boardBorder
            anchors.fill: parent
            radius: 4
            color: "transparent"
            border.color: "#8B7355"
            border.width: 3
            z: 0
        }

        Item {
            id: boardContainer
            anchors.centerIn: parent
            width: ChessManager.board.boardWidth
            height: ChessManager.board.boardHeight

            // 棋盘底图
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
                        console.log("【落子】col=", col, "row=", row)
                        ChessManager.moveSelectedPiece(col, row)
                    }
                }
            }

            // 棋子渲染
            Repeater {
                model: ChessManager.pieces
                delegate: Image {
                    width: modelData.pieceW
                    height: modelData.pieceH
                    x: modelData.posX
                    y: modelData.posY
                    z: modelData.isSelected ? 15 : 10
                    source: modelData.imgSource
                    visible: modelData.isAlive

                    // 选中时的发光效果
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: -3
                        radius: width / 2
                        color: "transparent"
                        border.color: modelData.isSelected ? "#ffff00" : "transparent"
                        border.width: 2
                        z: -1
                    }

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
    }

    // ========== 左侧操作按钮栏 ==========
    Column {
        id: sideBar
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 15
        anchors.bottomMargin: 15
        spacing: 15
        z: 25

        // 返回首页
        Button {
            id: returnBtn
            text: "返回首页"
            width: 90
            height: 40
            onClicked: confirmDlg.open()
            background: Rectangle {
                radius: 6
                color: returnBtn.hovered ? "#555555" : "#333333"
                border.color: "#777777"
                border.width: 1
            }
            contentItem: Text {
                text: returnBtn.text
                color: "#ffffff"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        // 悔棋
        Button {
            id: undoBtn
            text: "悔  棋"
            width: 90
            height: 40
            enabled: ChessManager.canUndo
            opacity: ChessManager.canUndo ? 1.0 : 0.4
            onClicked: {
                if (ChessManager.onlineState) {
                    ChessManager.requestUndo()
                    waitingDlg.open()
                } else {
                    ChessManager.undoMove()
                }
            }
            background: Rectangle {
                radius: 6
                color: undoBtn.enabled
                       ? (undoBtn.hovered ? "#c0392b" : "#e74c3c")
                       : "#555555"
                border.color: undoBtn.enabled ? "#ff6b6b" : "#666666"
                border.width: 1
            }
            contentItem: Text {
                text: undoBtn.text
                color: "#ffffff"
                font.pixelSize: 15
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    // ========== 确认返回弹窗 ==========
    Dialog {
        id: confirmDlg
        anchors.centerIn: parent
        title: "确认返回"
        modal: true
        z: 50
        width: 300
        height: 150

        Column {
            anchors.centerIn: parent
            spacing: 20
            Text {
                text: "游戏正在进行中，确定返回首页吗？"
                font.pixelSize: 14
                color: "#333333"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Row {
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter
                Button {
                    text: "取消"
                    implicitWidth: 80
                    implicitHeight: 34
                    onClicked: confirmDlg.close()
                    background: Rectangle {
                        radius: 4
                        color: "#eeeeee"
                        border.color: "#aaaaaa"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#333333"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Button {
                    text: "确定返回"
                    implicitWidth: 80
                    implicitHeight: 34
                    onClicked: {
                        confirmDlg.close()
                        ChessManager.requestGoStartPage()
                    }
                    background: Rectangle {
                        radius: 4
                        color: "#e74c3c"
                        border.color: "#c0392b"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    // ========== 结果弹窗 ==========
    ResultDialog { id: resDlg }

    // ========== 等待对方响应弹窗 ==========
    Dialog {
        id: waitingDlg
        anchors.centerIn: parent
        title: "等待对方响应"
        modal: true
        z: 50
        width: 280
        height: 120
        standardButtons: Dialog.NoButton

        Text {
            anchors.centerIn: parent
            text: "已发送悔棋请求，等待对方同意…"
            font.pixelSize: 14
            color: "#333333"
        }
    }

    // ========== 对方请求悔棋弹窗 ==========
    Dialog {
        id: undoRequestDlg
        anchors.centerIn: parent
        title: "悔棋请求"
        modal: true
        z: 50
        width: 300
        height: 150

        Column {
            anchors.centerIn: parent
            spacing: 20
            Text {
                text: "对方请求悔棋，是否同意？"
                font.pixelSize: 14
                color: "#333333"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Row {
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter
                Button {
                    text: "同意"
                    implicitWidth: 80
                    implicitHeight: 34
                    onClicked: {
                        undoRequestDlg.close()
                        ChessManager.respondToUndo(true)
                    }
                    background: Rectangle {
                        radius: 4
                        color: "#4caf50"
                        border.color: "#388e3c"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Button {
                    text: "拒绝"
                    implicitWidth: 80
                    implicitHeight: 34
                    onClicked: {
                        undoRequestDlg.close()
                        ChessManager.respondToUndo(false)
                    }
                    background: Rectangle {
                        radius: 4
                        color: "#e74c3c"
                        border.color: "#c0392b"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    // ========== 对方拒绝悔棋提示 ==========
    Dialog {
        id: undoRejectDlg
        anchors.centerIn: parent
        title: "悔棋被拒绝"
        modal: true
        z: 50
        width: 260
        height: 120
        standardButtons: Dialog.Ok

        Text {
            anchors.centerIn: parent
            text: "对方拒绝了你的悔棋请求"
            font.pixelSize: 14
            color: "#333333"
        }
    }

    Connections {
        target: ChessManager
        function onGameOver(txt) {
            resDlg.showText = txt
            resDlg.visible = true
        }
        function onUndoRequested() {
            undoRequestDlg.open()
        }
        function onUndoResponded(agreed) {
            waitingDlg.close()
            if (agreed) {
                ChessManager.doOnlineUndo()
            } else {
                undoRejectDlg.open()
            }
        }
    }
}
