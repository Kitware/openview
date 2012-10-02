
import QtQuick 2.0

Rectangle {
  id: comboBox
  property variant items: ListModel{ ListElement{name: "hi"} }
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
    color: "#ddd"
    smooth: true
    Column {
      anchors.fill: parent
      Text {
        height: 20
        width: parent.width
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: name
        font.family: "Helvetica"
        font.bold: true
        font.pointSize: 12
        smooth:true
      }
      Text {
        height: 15
        width: parent.width
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        id: chosenItemText
        text: selectedIndex >= 0 ? comboBox.items.get(selectedIndex).name : "";
        font.family: "Helvetica"
        font.pointSize: 12
        smooth:true
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
    clip: true
    anchors.top: chosenItem.bottom
    color: "#ddd"

    ListView {
      id:listView
      height: 500
      model: comboBox.items
      currentIndex: 0
      delegate: Item {
        width: comboBox.width
        height: comboBox.height

        Text {
          height: 40
          width: parent.width
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
          font.family: "Helvetica"
          font.pointSize: 12
          text: name
        }
        MouseArea {
          anchors.fill: parent;
          onClicked: {
            comboBox.state = ""
            var prevSelection = chosenItemText.text
            chosenItemText.text = name
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
    PropertyChanges { target: dropDown; height:40*comboBox.items.count }
  }

  transitions: Transition {
    NumberAnimation { target: dropDown; properties: "height"; duration: 250 }
  }
}
