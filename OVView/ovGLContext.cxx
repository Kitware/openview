#include "ovGLContext.h"

QGLContext *ovGLContext::_instance = NULL;

QGLContext *ovGLContext::instance()
{
  if(!_instance)
    {
    QGLFormat format;
    format.setVersion(2, 0);
    _instance = new QGLContext(format);
    }

  return _instance;
}
