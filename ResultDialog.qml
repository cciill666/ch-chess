import QtQuick 2.15
import QtQuick.Controls 2.15

 Popup {
     id: root
     width: 340
     height: 210
     modal: true
     visible: false
     anchors.centerIn: parent
     property string showText: ""
     Rectangle {
         anchors.fill: parent
         radius: 10
         color: "#fff"
         border.color: "#bbb"
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
                     onClicked: root.visible = false
                 }
                 Button {
                     text: "返回首页"
                     width: 110
                     onClicked: {
                         root.visible = false
                         parent.parent.parent.currentPage = 0
                     }
                 }
             }
         }
     }
 }