import QtQuick 2.15
import chinesechess 1.0

Window {
    width: 800
    height: 600
    visible: true
    property int currentPage: 0

    StartPage { visible: currentPage === 0 }
    GamePage  { visible: currentPage === 1 }

    Connections {
        target: ChessManager
        function onGoGamePage() {
            currentPage = 1
        }
        function onGoStartPage() {
            currentPage = 0
        }
    }
}