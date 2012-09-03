
#include <QQuickItem>

#include <QOpenGLShaderProgram>

class ovRectItem : public QQuickItem
{
  Q_OBJECT
public:
  ovRectItem();
  void itemChange(ItemChange change, const ItemChangeData &);
public slots:
  void paint();
private:
  QOpenGLShaderProgram *m_program;
};

