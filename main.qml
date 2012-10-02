import QtQuick 2.0
import OVView 1.0

Row {
  width: 1000;
  height: 1000;

  // ------------------------------------------------------------------
  // File open dialog
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

  Item {
    width: parent.width
    height: parent.height
    //color: "#ffa"
    //opacity: 1

    Grid {
      anchors.fill: parent
      columns: 2
      spacing: 0
      //opacity: 1

      // ------------------------------------------------------------------
      // Button to add data
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


      // ------------------------------------------------------------------
      // View list
      Rectangle {
        width: parent.width - 200;
        height: 40;
        color: "#f5f5f5"
        z: 5

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
          id: viewList;
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

          onCurrentIndexChanged: {
            view.viewType = model.get(currentIndex).name;
            updateViewAttributes();
          }

          function updateViewAttributes() {
            attributeListView.model.clear();
            for (var i in view.attributes) {
              var attribute = view.attributes[i];
              var dataFields = view.attributeOptions(attribute);
              var value = view.getAttribute(attribute);
              var valueIndex = -1;
              var dataFieldsArray = Qt.createQmlObject("import QtQuick 2.0; ListModel {}", attributeList);
              for (var j = 0; j < dataFields.length; ++j) {
                if (dataFields[j] === value) {
                  valueIndex = j;
                }
                dataFieldsArray.append({name: dataFields[j]});
              }
              attributeListView.model.append({name: attribute, fields: dataFieldsArray, valueIndex: valueIndex});
            }
          }

          MouseArea {
            anchors.fill: parent
            onClicked: {
              parent.focus = true
              var indexedItem = parent.indexAt(mouseX, mouseY)
              if (indexedItem === parent.currentIndex) {
                if (attributeList.height > 0) {
                  attributeList.height = 0;
                  attributeList.opacity = 0;
                } else {
                  attributeList.height = 40
                  attributeList.opacity = 1;
                }
              }
              else {
                parent.currentIndex = indexedItem
              }
            }
          }
        }
      }

      // ------------------------------------------------------------------
      // Data list
      Rectangle {
        width: 200
        height: parent.height - 40
        id: dataListRect
        color: "#f5f5f5"

        Component {
          id: dataItemDelegate
          Item {
            width: parent.width
            height: 40
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
            width: parent.width
            gradient: Gradient {
              GradientStop { position: 0.0; color: "#eee" }
              GradientStop { position: 1.0; color: "#ddd" }
            }
          }
          MouseArea {
            anchors.fill: parent
            onClicked: {
              parent.focus = true;
              var indexedItem = parent.indexAt(mouseX, mouseY);
              if (indexedItem !== -1) {
                parent.currentIndex = indexedItem;
                view.url = parent.model.get(indexedItem).path;
                viewList.updateViewAttributes();
              }
            }
          }
        }
      }

      Item {
        width: parent.width - 200;
        height: parent.height - 40;

        Column {
          anchors.fill: parent
          spacing: 0

          // ------------------------------------------------------------------
          // Attributes list
          Rectangle {
            width: parent.width
            height: 0
            opacity: 0
            color: "#ddd"
            z: 5
            id: attributeList
            Behavior on height {
              NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
            }
            Behavior on opacity {
              NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
            }

            Component {
              id: attributeItemDelegate
              ComboBox {
                id: attributeComboBox
                width: 80
                //height: parent.height
                items: fields
                selectedIndex: valueIndex
                label: name
                //items: [name, name, name];
                /*
                Text {
                  text: name;
                  anchors.fill: parent
                  verticalAlignment: Text.AlignVCenter
                  horizontalAlignment: Text.AlignHCenter
                  font.family: "Helvetica"
                  font.pointSize: 12
                  font.weight: Font.Bold
                }
                */
                onComboClicked: {
                  view.setAttribute(label, selectedItem);
                }
              }
            }

            ListView {
              id: attributeListView
              anchors.fill: parent;
              model: ListModel {}
              orientation: ListView.Horizontal;
              delegate: attributeItemDelegate
              /*
              highlight: Rectangle {
                height: parent.height;
                width: 80
                gradient: Gradient {
                  GradientStop { position: 0.0; color: "#eee" }
                  GradientStop { position: 1.0; color: "#ddd" }
                }
              }
              */
            }
          }

          // ------------------------------------------------------------------
          // VTK view
          OVView {
            id: view
            width: parent.width
            height: parent.height - attributeList.height
            //anchors.fill: parent
          }
        }
      }
    }
  }
}

