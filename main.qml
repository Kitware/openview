import QtQuick 1.0
import OVView 1.0

Row {
  width: 1000;
  height: 1000;

  FileDialog {
    id: openDataDialog
    width: 0
    height: parent.height
    Behavior on width {
      NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
    }
    onFileSelected: {
      dataListView.model.append({name: fileName, path: filePath})
      width = 0
    }
  }

  Rectangle {
    width: parent.width;
    height: parent.height;
    color: "#ffa";

    Grid {
      anchors.fill: parent
      columns: 2
      spacing: 0

      Rectangle {
        width: 200;
        height: 40;
        gradient: Gradient {
          GradientStop { position: 0.0; color: "#fdd" }
          GradientStop { position: 1.0; color: "#faa" }
        }

        Text {
          anchors.horizontalCenter: parent.horizontalCenter;
          anchors.verticalCenter: parent.verticalCenter;
          text: "< ADD DATA";
          font.family: "Helvetica";
          font.pointSize: 12;
          font.weight: Font.Bold
        }

        MouseArea {
          anchors.fill: parent
          onClicked: {
            if (openDataDialog.width > 0) {
              openDataDialog.width = 0
            } else {
              openDataDialog.width = 200
            }
          }
        }
      }


      Rectangle {
        width: parent.width - 200;
        height: 40;

        Component {
          id: viewItemDelegate
          Item {
            width: 80
            height: parent.height
            Text {
              text: name;
              anchors.fill: parent
              verticalAlignment: Text.AlignVCenter
              horizontalAlignment: Text.AlignHCenter
              font.family: "Helvetica"
              font.pointSize: 12
              font.weight: Font.Bold
            }
          }
        }

        ListView {
          anchors.fill: parent;
          model: ViewModel {}
          orientation: ListView.Horizontal;
          delegate: viewItemDelegate
          highlight: Rectangle {
            height: parent.height;
            width: 80
            gradient: Gradient {
              GradientStop { position: 0.0; color: "#eee" }
              GradientStop { position: 1.0; color: "#ddd" }
            }
          }
          MouseArea {
            anchors.fill: parent
            onClicked: {
              parent.focus = true
              var indexedItem = parent.indexAt(mouseX, mouseY)
              if (indexedItem !== -1)
                parent.currentIndex = indexedItem
            }
          }
        }
      }

      Rectangle {
        width: 200
        height: parent.height - 40
        id: dataListRect
        color: "#eee"

        Component {
          id: dataItemDelegate
          Rectangle {
            width: parent.width
            height: 40
            gradient: Gradient {
              GradientStop { position: 0.0; color: "#eee" }
              GradientStop { position: 1.0; color: "#ddd" }
            }
            Text {
              text: name;
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

        ListView {
          anchors.fill: parent
          model: ListModel {}
          id: dataListView
          delegate: dataItemDelegate
          highlight: Rectangle {
            width: parent.width;
            color: "#bbb";
          }
          MouseArea {
            anchors.fill: parent
            onClicked: {
              parent.focus = true;
              var indexedItem = parent.indexAt(mouseX, mouseY);
              if (indexedItem !== -1) {
                parent.currentIndex = indexedItem;
                view.url = parent.model.get(indexedItem).path;
              }
            }
          }
        }
      }

      OVView {
        id: view;
        width: parent.width - 200;
        height: parent.height - 40;
        anchors.fill: parent;
      }
    }
  }
}

