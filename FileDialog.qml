import QtQuick 1.0
import Qt.labs.folderlistmodel 1.0

Rectangle {
  signal fileSelected (string fileName)

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
              height: 30
              Text {
                  text: fileName
                  anchors.fill: parent
                  MouseArea {
                      anchors.fill: parent
                      onClicked: {
                          if (foldermodel.isFolder(index)) {
                              foldermodel.folder = filePath
                          }
                          else {
                            fileSelected(filePath + '/' + fileName)
                          }
                      }
                  }
              }
          }
      }

      model: foldermodel
      delegate: filedelegate
      header: Rectangle {
        width: parent.width
        height: 30
        Text { text: foldermodel.folder; }
      }
  }
}

