#include "ovGLContext.h"

QGLContext *ovGLContext::_instance = NULL;

QGLContext *ovGLContext::instance()
{
  if(!_instance)
    _instance = new QGLContext(QGLFormat());

  return _instance;
}
