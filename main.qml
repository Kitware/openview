import QtQuick 1.0
import OVView 1.0

Row {
  width: 1000;
  height: 540;

  FileDialog {
    id: openDataDialog
    width: 0
    height: parent.height
    Behavior on width {
      NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
    }
    onFileSelected: {
      dataListView.model.append({name: fileName})
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
      color: "#faa";

       MouseArea {
         anchors.fill: parent
         onClicked: {
           openDataDialog.width = 200
         }
       }
    }


    Rectangle {
      width: parent.width - 200;
      height: 40;

       Component {
           id: viewItemDelegate
           Item {
             width: 60; height: parent.height
             Column {
                 Text { text: name }
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
             color: "#bbb";
           }
           MouseArea {
             anchors.fill: parent
             onClicked: {
                parent.focus = true
                var indexedItem = parent.indexAt(mouseX, mouseY)
                if (indexedItem != -1)
                  parent.currentIndex = indexedItem
             }
           }
       }
    }

    Rectangle {
       width: 200;
       height: parent.height - 40;
       id: dataListRect;

       Component {
           id: dataItemDelegate
           Item {
             width: parent.width; height: 40
             Column {
                 Text { text: name }
             }
           }
       }

       ListView {
           anchors.fill: parent
           model: DataModel {}
           id: dataListView
           delegate: dataItemDelegate
           highlight: Rectangle {
             width: parent.width;
             color: "#bbb";
           }
           MouseArea {
             anchors.fill: parent
             onClicked: {
                parent.focus = true
                var indexedItem = parent.indexAt(mouseX, mouseY)
                if (indexedItem != -1)
                  parent.currentIndex = indexedItem
             }
           }
       }
    }

    OVView {
      width: parent.width - 200;
      height: parent.height - 40;
      anchors.fill: parent;
    }
  }
}
}

