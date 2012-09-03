
#include "QVTKQuickItem.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QQuickCanvas>
#include <QSGEngine>
#include <QSGSimpleRectNode>
#include <QThread>

#include "QVTKInteractor.h"
#include "QVTKInteractorAdapter.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkgl.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkBlockItem.h"


#include <iostream>

QVTKQuickItem::QVTKQuickItem() : m_program(0)
{
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);

  mFBO = NULL;
}

QVTKQuickItem::~QVTKQuickItem()
{
  if(mFBO)
    delete mFBO;
}

void QVTKQuickItem::SetRenderWindow(vtkGenericOpenGLRenderWindow* win)
{
  if(mWin)
  {
    mWin->SetMapped(0);
    //mConnect->Disconnect(mWin, vtkCommand::StartEvent, this, SLOT(Start()));
    //mConnect->Disconnect(mWin, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    //mConnect->Disconnect(mWin, vtkCommand::EndEvent, this, SLOT(End()));
    //mConnect->Disconnect(mWin, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    mConnect->Disconnect(mWin, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    mConnect->Disconnect(mWin, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    mConnect->Disconnect(mWin, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
  }

  mIren->SetRenderWindow(win);
  mWin = win;
  mIren->Initialize();

  if(mWin)
  {
    mWin->SetMapped(1);
    mWin->SetDoubleBuffer(0);
    mWin->SetFrontBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    mWin->SetFrontLeftBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    mWin->SetBackBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    mWin->SetBackLeftBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);

    //mConnect->Connect(mWin, vtkCommand::StartEvent, this, SLOT(Start()));
    //mConnect->Connect(mWin, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    //mConnect->Connect(mWin, vtkCommand::EndEvent, this, SLOT(End()));
    //mConnect->Connect(mWin, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    mConnect->Connect(mWin, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    mConnect->Connect(mWin, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    mConnect->Connect(mWin, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
  }
}

vtkGenericOpenGLRenderWindow* QVTKQuickItem::GetRenderWindow() const
{
  return mWin;
}

QVTKInteractor* QVTKQuickItem::GetInteractor() const
{
  return mIren;
}

void QVTKQuickItem::itemChange(ItemChange change, const ItemChangeData &)
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
    connect(c, SIGNAL(beforeRendering()), this, SLOT(vtkpaint()), Qt::DirectConnection);

    // If we allow QML to do the clearing, they would clear what we paint
    // and nothing would show.
    c->setClearBeforeRendering(false);
  }
}

void QVTKQuickItem::Update()
{
  if(this->mWin && this->mFBO)
    {
    this->update();
    //QRectF bf = boundingRect();
    //QRect bounds(bf.x(), bf.y(), bf.width(), bf.height());
    //this->update(bounds);
    }
}

void QVTKQuickItem::MakeCurrent()
{
  if (!this->canvas())
    {
    mWin->SetAbortRender(1);
    cerr << "Could not make current since there is no canvas!" << endl;
    return;
    }
  if (QThread::currentThread() != this->canvas()->openglContext()->thread())
    {
    mWin->SetAbortRender(1);
    cerr << "Could not make current since we are on the wrong thread!" << endl;
    return;
    }
  cerr << "Making current" << endl;
  this->canvas()->openglContext()->makeCurrent(this->canvas());

  QSize sz(this->width(), this->height());
  if(!mFBO || sz != mFBO->size())
  {
    if(mFBO)
      delete mFBO;

    if(!sz.isEmpty())
      mFBO = new QOpenGLFramebufferObject(sz, QOpenGLFramebufferObject::Depth);
  }

  if(mFBO)
    mFBO->bind();
}

void QVTKQuickItem::Start()
{
  MakeCurrent();

  if(!mFBO)
  {
    mWin->SetAbortRender(1);
    return;
  }

  if (!mWin->GetAbortRender())
    {
    mWin->PushState();
    mWin->OpenGLInitState();
    }
}

void QVTKQuickItem::End()
{
  if(!mFBO)
    return;

  if (!mWin->GetAbortRender())
    {
    mWin->PopState();
    mFBO->release();
    }

}

void QVTKQuickItem::IsCurrent(vtkObject*, unsigned long, void*, void* call_data)
{
  if(mFBO)
  {
    bool* ptr = reinterpret_cast<bool*>(call_data);
    *ptr = QOpenGLContext::currentContext() == this->canvas()->openglContext() && mFBO->isBound();
  }
}

void QVTKQuickItem::IsDirect(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = 1;
}

void QVTKQuickItem::SupportsOpenGL(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = true;
  //*ptr = QGLFormat::hasOpenGL();
}

void QVTKQuickItem::paint()
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
        "    lowp float i = 1. - (pow(abs(coords.x), 4.) + pow(abs(coords.y), 4.));"
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

  std::cerr << " x: " << x();
  std::cerr << " y: " << y();
  std::cerr << " width: " << width();
  std::cerr << " height: " << height();
  std::cerr << std::endl;
  QPointF origin = mapToScene(QPointF(x(), y()));
  std::cerr << " origin.x: " << origin.x();
  std::cerr << " origin.y: " << origin.y();
  std::cerr << std::endl;

  glViewport(origin.x(), canvas()->height() - origin.y() - height(), width(), height());

  glDisable(GL_DEPTH_TEST);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  m_program->disableAttributeArray(0);
  m_program->release();
}

void QVTKQuickItem::vtkpaint()
{
  std::cerr << "vtkpaint" << std::endl;
  if (!mWin.GetPointer())
    {
    std::cerr << "initializing" << std::endl;
    mIren = vtkSmartPointer<QVTKInteractor>::New();
    mIrenAdapter = new QVTKInteractorAdapter(this);
    mConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    mConnect->Connect(mIren, vtkCommand::RenderEvent, this, SLOT(Update()));
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    this->SetRenderWindow(win);
    mView->SetRenderWindow(win);
    vtkNew<vtkBlockItem> block;
    mView->GetScene()->AddItem(block.GetPointer());
    }

  if(!mWin.GetPointer())
    {
    std::cerr << "no render window" << std::endl;
    return;
    }


  //if(!mFBO)
  //  return;

  //QPointF origin = mapToScene(QPointF(x(), y()));
  //glViewport(origin.x(), canvas()->height() - origin.y() - height(), width(), height());
  //glDisable(GL_DEPTH_TEST);
  //glClearColor(1, 1, 1, 1);
  //glClear(GL_COLOR_BUFFER_BIT);

  this->Start();
  mIren->Render();
  this->End();

  // simply draw the already existing texture to the scene
  // modifications to the texture is done using the VTK api (e.g. vtkRenderWindow::Render())
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, mFBO->texture());

  QRectF r(this->x(), this->y(), this->width(), this->height());

  //QColor c = this->palette().color(QPalette::Window);
  //glColor4ub(c.red(),c.green(),c.blue(),c.alpha());

  //if(c.alpha() < 255)
  //  {
  //  glEnable(GL_BLEND);
  //  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  //  }
  //else
  //  {
    glDisable(GL_BLEND);
  //  }

  glBegin(GL_QUADS);
  glTexCoord2i(0,1);
  glVertex2f(r.left(),r.top());
  glTexCoord2i(1,1);
  glVertex2f(r.right(),r.top());
  glTexCoord2i(1,0);
  glVertex2f(r.right(),r.bottom());
  glTexCoord2i(0,0);
  glVertex2f(r.left(),r.bottom());
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);
}
