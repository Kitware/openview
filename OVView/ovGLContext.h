#ifndef ovGLContext_h
#define ovGLContext_h

#include <QGLWidget>

class ovGLContext
{
public:
  static QGLContext *instance();
private:
  ovGLContext(const QGLFormat&){}
  virtual ~ovGLContext();
private:
  static QGLContext *_instance;
};


#endif
