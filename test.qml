import QtQuick 1.0
import OVView 1.0

Item {
  width: 500;
  height: 500
  Rectangle {
    anchors.fill: parent;
    color: "#000";
  }
  OVView {
    anchors.fill: parent;
  }
}
