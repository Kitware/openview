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
