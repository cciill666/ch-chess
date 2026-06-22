import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15
import chinesechess 1.0

 Rectangle {
     anchors.fill: parent
     color: "#e9e9e9"
     property bool skipBoardTap: false
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
                     console.log("【落子】col=", col, "row=", row)
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