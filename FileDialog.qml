import QtQuick 1.0
import Qt.labs.folderlistmodel 1.0

Rectangle {
  signal fileSelected (string filePath, string fileName)

  ListView {
    anchors.fill: parent

    FolderListModel {
      id: foldermodel
      folder: "file:///"
      nameFilters: ["*.*"]
      showDotAndDotDot: true
    }

    Component {
      id: filedelegate
      Rectangle {
        width: parent.width
        height: 40

        gradient: Gradient {
          GradientStop { position: 0.0; color: "#fff" }
          GradientStop { position: 1.0; color: "#eee" }
        }

        Text {
          text: fileName
          anchors.fill: parent
          anchors.verticalCenter: parent.verticalCenter
          anchors.leftMargin: 10
          verticalAlignment: Text.AlignVCenter
          font.family: "Helvetica"
          font.pointSize: 12
          MouseArea {
            anchors.fill: parent
            onClicked: {
              if (foldermodel.isFolder(index)) {
                foldermodel.folder = filePath
              }
              else {
                fileSelected(filePath, fileName)
              }
            }
          }
        }
      }
    }

    model: foldermodel
    delegate: filedelegate
    header: Rectangle {
      height: 40
      width: parent.width

      gradient: Gradient {
        GradientStop { position: 0.0; color: "#fff" }
        GradientStop { position: 1.0; color: "#eee" }
      }

      Text {
        text: foldermodel.folder.toString().substring(7)
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 10
        verticalAlignment: Text.AlignVCenter
        font.family: "Helvetica"
        font.pointSize: 12
        font.weight: Font.Bold
      }
    }
  }
}

