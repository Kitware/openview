#include "ovOpenGLContext.h"

QOpenGLContext *ovOpenGLContext::_instance = NULL;

QOpenGLContext *ovOpenGLContext::instance()
{
  if(!_instance)
    {
    _instance = new QOpenGLContext();
    }

  return _instance;
}

void ovOpenGLContext::setInstance(QOpenGLContext *instance)
{
  _instance = instance;
}
