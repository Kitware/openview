/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include <QQuickView>
#include <QUrl>
#include <QApplication>

#include "ovViewQuickItem.h"
#include "ovWorkflowQuickItem.h"

#include <Cocoa/Cocoa.h>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QQuickView view;

  qmlRegisterType<ovViewQuickItem>("OVView", 1, 0, "OVView");
  qmlRegisterType<ovWorkflowQuickItem>("OVWorkflow", 1, 0, "OVWorkflow");

  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.setSource(QUrl("qrc:/main.qml"));
  view.show();

  NSView *nsview = (NSView *) view.winId(); 
  NSWindow *nswindow = [nsview window];
  [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

  return app.exec();
}
