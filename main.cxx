/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include <QQuickView>
#include <QUrl>
#include <QApplication>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QQuickView view;

  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.setSource(QUrl::fromLocalFile("main.qml"));
  view.show();
  return app.exec();
}
