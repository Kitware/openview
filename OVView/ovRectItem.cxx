
#include "ovRectItem.h"

#include <QSGEngine>
#include <QQuickCanvas>
#include <QOpenGLShaderProgram>

#include <iostream>

ovRectItem::ovRectItem() : m_program(0)
{
  setFlag(ItemHasContents);
}

void ovRectItem::itemChange(ItemChange change, const ItemChangeData &)
{
  // The ItemSceneChange event is sent when we are first attached to a canvas.
  if (change == ItemSceneChange) {
    QQuickCanvas *c = canvas();
    if (!c)
      {
      return;
      }

    // Connect our the beforeRendering signal to our paint function.
    // Since this call is executed on the rendering thread it must be
    // a Qt::DirectConnection
    connect(c, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);

    // If we allow QML to do the clearing, they would clear what we paint
    // and nothing would show.
    c->setClearBeforeRendering(false);
  }
}

void ovRectItem::paint()
{
  if (!m_program) {
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertices;"
        "varying highp vec2 coords;"
        "void main() {"
        "    gl_Position = vertices;"
        "    coords = vertices.xy;"
        "}");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "uniform lowp float t;"
        "varying highp vec2 coords;"
        "void main() {"
        "    lowp float i = 1. - (pow(coords.x, 4.) + pow(coords.y, 4.));"
        "    i = smoothstep(t - 0.3, t + 0.3, i);"
        "    gl_FragColor = vec4(coords / 2. + .5, i, i);"
        "}");

    m_program->bindAttributeLocation("vertices", 0);
    m_program->link();
  }

  m_program->bind();

  m_program->enableAttributeArray(0);

  float values[] = {
    -1, -1,
    1, -1,
    -1, 1,
    1, 1
  };
  float m_t = 0.5;
  m_program->setAttributeArray(0, GL_FLOAT, values, 2);
  m_program->setUniformValue("t", (float) m_t);

  glViewport(0, 0, canvas()->width(), canvas()->height());

  glDisable(GL_DEPTH_TEST);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  m_program->disableAttributeArray(0);
  m_program->release();
}
