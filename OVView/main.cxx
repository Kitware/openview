#include <QtDeclarative>
#include <QApplication>
#include <QGraphicsView>
#include "ovGLContext.h"
#include "ovView.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QDeclarativeView view;

  view.setViewport(new QGLWidget(ovGLContext::instance()));
  view.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  view.setCacheMode(QGraphicsView::CacheNone);
  view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
  view.setSource(QUrl::fromLocalFile("test.qml"));
  view.show();
  return app.exec();
}
