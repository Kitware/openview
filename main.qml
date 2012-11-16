/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
import QtQuick 2.0
import OVView 1.0

Row {
  width: 1000;
  height: 800;

  // ------------------------------------------------------------------
  // File open dialog
  FileDialog {
    id: openDataDialog
    width: 0
    height: parent.height
    Behavior on width {
      NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
    }
    onFileSelected: {
      width = 0;
      dataListView.model.append({name: fileName, path: filePath});
      dataListView.currentIndex = dataListView.model.count - 1;
    }
  }

  Item {
    width: parent.width
    height: parent.height

    Grid {
      anchors.fill: parent
      columns: 2
      spacing: 0

      // ------------------------------------------------------------------
      // Button to add data
      Rectangle {
        id: addData;
        width: 200;
        height: 40;

        gradient: Gradient {
          GradientStop { id: addDataStop1; position: 0.0; color: Qt.rgba(0, 136/255, 204/255, 1) }
          GradientStop { id: addDataStop2; position: 1.0; color: Qt.rgba(0, 68/255, 204/255, 1) }
        }

        UIText {
          anchors.horizontalCenter: parent.horizontalCenter;
          anchors.verticalCenter: parent.verticalCenter;
          text: "\u2190 ADD DATA";
          color: "#fff";
        }

        MouseArea {
          anchors.fill: parent
          onClicked: {
            if (openDataDialog.width > 0) {
              addDataStop1.color = Qt.rgba(0, 136/255, 204/255, 1);
              openDataDialog.width = 0
            } else {
              addDataStop1.color = Qt.rgba(0, 68/255, 204/255, 1);
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
        color: "#555"

        Component {
          id: viewItemDelegate
          Item {
            width: 120
            height: parent.height
            id: viewItem;
            UIText {
              text: name;
              color: viewItem.ListView.isCurrentItem ? "white" : "#ccc";
              anchors.fill: parent;
              verticalAlignment: Text.AlignVCenter;
              horizontalAlignment: Text.AlignHCenter;
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
            width: 120

            color: "#444";

            Rectangle {
              height: 6;
              width: parent.height;
              x: height;
              transformOrigin: Item.TopLeft;
              rotation: 90;
              gradient: Gradient {
                GradientStop { position: 0.0; color: "#444" }
                GradientStop { position: 1.0; color: "#333" }
              }
            }

            Rectangle {
              height: 6;
              width: parent.height;
              x: parent.width - 1;
              transformOrigin: Item.TopLeft;
              rotation: 90;
              gradient: Gradient {
                GradientStop { position: 0.0; color: "#333" }
                GradientStop { position: 1.0; color: "#444" }
              }
            }

            Rectangle {
              width: 1;
              height: parent.height;
              x: parent.width - 1;
              color: "#888";
              opacity: 0.5;
            }
          }

          onCurrentIndexChanged: {
            var type = model.get(currentIndex).name;
            if (type === "TABLE") {
              view.visible = false;
              tableView.visible = true;
            } else {
              view.visible = true;
              tableView.visible = false;
              view.viewType = type;
            }
            updateViewAttributes();
          }

          function updateViewAttributes() {
            attributeListView.model.clear();
            if (model.get(currentIndex).name === "TABLE") {
              return;
            }

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
        z: 5;

        Component {
          id: dataItemDelegate
          Item {
            width: parent.width
            height: 40
            UIText {
              text: name;
              anchors.fill: parent
              anchors.verticalCenter: parent.verticalCenter
              anchors.leftMargin: 10
              verticalAlignment: Text.AlignVCenter
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

          onCurrentIndexChanged: {
            view.url = model.get(currentIndex).path;
            tableView.update();
            viewList.updateViewAttributes();
          }

          MouseArea {
            anchors.fill: parent
            onClicked: {
              parent.focus = true;
              var indexedItem = parent.indexAt(mouseX, mouseY);
              if (indexedItem !== -1) {
                parent.currentIndex = indexedItem;
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
            color: "#555"
            id: attributeList
            z: 5

            Component {
              id: attributeItemDelegate
              ComboBox {
                id: attributeComboBox
                width: 120
                items: fields
                selectedIndex: valueIndex
                label: name
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
            }
          }

          // ------------------------------------------------------------------
          // Main view area
          Column {
            width: parent.width
            height: parent.height - attributeList.height

            Timer {
              interval: 1000/60
              running: true
              repeat: true
              onTriggered: view.animate()
            }

            // VTK view
            OVView {
              id: view
              width: parent.width
              height: parent.height
              visible: true
            }

            // Table view
            Rectangle {
              id: tableView

              width: parent.width
              height: parent.height
              visible: false

              color: "#fff"
              Component {
                id: tableRowDelegate

                Rectangle {
                  width: parent.width
                  height: 40
                  property int rowIndex: index

                  Component {
                    id: tableItemDelegate
                    Rectangle {
                      width: 120
                      height: 40
                      clip: true
                      gradient: Gradient {
                        GradientStop {
                          position: 0.0
                          color: "#fff"
                        }
                        GradientStop {
                          position: 1.0
                          color: "#eee"
                        }
                      }

                      UIText {
                        text: rowIndex === 0 ? view.tableColumnName(index) : view.tableData(rowIndex-1, index)
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        font.weight: rowIndex === 0 ? Font.Bold : Font.Normal
                      }

                    }
                  }

                  ListView {
                    id: tableItemView
                    anchors.fill: parent;
                    interactive: false;
                    model: 10
                    orientation: ListView.Horizontal;
                    delegate: tableItemDelegate
                  }
                }
              }
              ListView {
                id: tableRowView
                maximumFlickVelocity: 5000
                anchors.fill: parent
                model: 0
                delegate: tableRowDelegate
              }

              function update() {
                tableRowView.model = view.tableRows() + 1;
              }
            }
          }
        }
      }
    }
  }
}
