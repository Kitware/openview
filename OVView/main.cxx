#include <QQuickView>
#include <QQmlEngine>
#include <QOpenGLContext>
#include <QApplication>
#include <QGraphicsView>
//#include "ovOpenGLContext.h"
#include "ovView.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QQuickView view;
  //QOpenGLContext *context = view.openglContext();
  //ovGLContext::setInstance(context);

  //view.setViewport(new QGLWidget(ovGLContext::instance()));
  //view.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  //view.setCacheMode(QGraphicsView::CacheNone);
  //view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
  view.setSource(QUrl::fromLocalFile("main.qml"));
  view.show();
  return app.exec();
}
