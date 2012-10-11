/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include <QQuickView>
#include <QUrl>
#include <QApplication>

#include "ovViewQuickItem.h"

#include <Cocoa/Cocoa.h>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QQuickView view;

  qmlRegisterType<ovViewQuickItem>("OVView", 1, 0, "OVView");

  view.setResizeMode(QQuickView::SizeRootObjectToView);
  //view.setSource(QUrl::fromLocalFile("main.qml"));
  view.setSource(QUrl("qrc:/main.qml"));
  view.show();

  NSView *nsview = (NSView *) view.winId(); 
  NSWindow *nswindow = [nsview window];
  [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

  return app.exec();
}
