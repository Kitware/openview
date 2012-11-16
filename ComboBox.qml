/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/

import QtQuick 2.0

Rectangle {
  id: comboBox
  property variant items: ListModel{ }
  property alias selectedItem: chosenItemText.text
  property alias selectedIndex: listView.currentIndex
  property string label: "Name"
  signal comboClicked
  width: 80
  height: parent.height
  smooth: true

  Rectangle {
    id: chosenItem
    width: parent.width
    height: comboBox.height
    color: "#444"
    smooth: true
    Column {
      anchors.fill: parent
      UIText {
        height: 20
        width: parent.width
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: name
        color: "#ccc"
      }
      UIText {
        height: 15
        width: parent.width
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        id: chosenItemText
        text: selectedIndex >= 0 ? comboBox.items.get(selectedIndex).name : "";
        color: "#aaa"
      }
    }

    MouseArea {
      anchors.fill: parent;
      onClicked: {
        comboBox.state = comboBox.state==="dropDown"?"":"dropDown"
      }
    }
  }

  Rectangle {
    id: dropDown
    width: comboBox.width
    height: 0
    visible: false
    anchors.top: chosenItem.bottom;
    anchors.topMargin: 5;

    ListView {
      id:listView
      height: 500
      model: comboBox.items
      currentIndex: 0
      delegate: Rectangle {
        width: comboBox.width
        height: 30
        color: comboItemArea.containsMouse ? "#0088cc" : "white";
        border.color: "#aaa";

        UIText {
          height: 30
          width: parent.width
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
          text: name
          color: comboItemArea.containsMouse ? "white" : "#333";
        }
        MouseArea {
          id: comboItemArea;
          anchors.fill: parent;
          hoverEnabled: true;

          onClicked: {
            comboBox.state = ""
            var prevSelection = chosenItemText.text
            chosenItemText.text = name;
            if(chosenItemText.text != prevSelection){
              comboBox.comboClicked();
            }
            listView.currentIndex = index;
          }
        }
      }
    }
  }

  Component {
    id: highlight
    Rectangle {
      width: comboBox.width
      height: comboBox.height
      color: "red"
    }
  }

  states: State {
    name: "dropDown";
    PropertyChanges { target: dropDown; visible: true }
  }
}
