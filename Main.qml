import QtQuick
import QtQuick.Controls
import chinesechess

Item {
    width: 800
    height: 600
    property int currentPage: 0

    StartPage { visible: currentPage === 0 }
    GamePage  { visible: currentPage === 1 }

    Connections {
        target: ChessManager
        function onGoGamePage(){
            currentPage = 1
        }
    }
}