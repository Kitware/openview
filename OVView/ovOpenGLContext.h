#ifndef ovOpenGLContext_h
#define ovOpenGLContext_h

#include <QOpenGLContext>

class ovOpenGLContext
{
public:
  static QOpenGLContext *instance();
  static void setInstance(QOpenGLContext *instance);
private:
  ovOpenGLContext() {}
  virtual ~ovOpenGLContext() {};
private:
  static QOpenGLContext *_instance;
};


#endif
