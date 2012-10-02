
#include "QVTKQuickItem.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QQuickCanvas>
#include <QThread>

#include "QVTKInteractor.h"
#include "QVTKInteractorAdapter.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkgl.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkBlockItem.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"

#include "vtkCubeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"


#include <iostream>

QVTKQuickItem::QVTKQuickItem()
{
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
}

QVTKQuickItem::~QVTKQuickItem()
{
  this->SetRenderWindow(0);
}

void QVTKQuickItem::SetRenderWindow(vtkGenericOpenGLRenderWindow* win)
{
  if(m_win)
    {
    m_win->SetMapped(0);
    //m_connect->Disconnect(m_win, vtkCommand::StartEvent, this, SLOT(Start()));
    //m_connect->Disconnect(m_win, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    //m_connect->Disconnect(m_win, vtkCommand::EndEvent, this, SLOT(End()));
    //m_connect->Disconnect(m_win, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    m_connect->Disconnect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    m_connect->Disconnect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    m_connect->Disconnect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
    }

  m_interactor->SetRenderWindow(win);
  m_win = win;
  m_interactor->Initialize();

  if(m_win)
    {
    m_win->SetMapped(1);
    m_win->SetDoubleBuffer(0);
    m_win->SetFrontBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    m_win->SetFrontLeftBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    m_win->SetBackBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    m_win->SetBackLeftBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);

    //m_connect->Connect(m_win, vtkCommand::StartEvent, this, SLOT(Start()));
    //m_connect->Connect(m_win, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    //m_connect->Connect(m_win, vtkCommand::EndEvent, this, SLOT(End()));
    //m_connect->Connect(m_win, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    m_connect->Connect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    m_connect->Connect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    m_connect->Connect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
    }
}

vtkGenericOpenGLRenderWindow* QVTKQuickItem::GetRenderWindow() const
{
  return m_win;
}

QVTKInteractor* QVTKQuickItem::GetInteractor() const
{
  return m_interactor;
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
    connect(c, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);

    // If we allow QML to do the clearing, they would clear what we paint
    // and nothing would show.
    c->setClearBeforeRendering(false);
  }
}

void QVTKQuickItem::MakeCurrent()
{
  if (!this->canvas())
    {
    m_win->SetAbortRender(1);
    cerr << "Could not make current since there is no canvas!" << endl;
    return;
    }
  if (QThread::currentThread() != this->canvas()->openglContext()->thread())
    {
    m_win->SetAbortRender(1);
    cerr << "Could not make current since we are on the wrong thread!" << endl;
    return;
    }
  cerr << "Making current" << endl;
  this->canvas()->openglContext()->makeCurrent(this->canvas());
}

void QVTKQuickItem::Start()
{
  MakeCurrent();

  if (!m_win->GetAbortRender())
    {
    m_win->PushState();
    m_win->OpenGLInitState();
    }
}

void QVTKQuickItem::End()
{
  if (!m_win->GetAbortRender())
    {
    m_win->PopState();
    }

}

void QVTKQuickItem::IsCurrent(vtkObject*, unsigned long, void*, void* call_data)
{
  bool* ptr = reinterpret_cast<bool*>(call_data);
  *ptr = QOpenGLContext::currentContext() == this->canvas()->openglContext();
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

void QVTKQuickItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
  QQuickItem::geometryChanged(newGeometry, oldGeometry);
  QSize oldSize(oldGeometry.width(), oldGeometry.height());
  QSize newSize(newGeometry.width(), newGeometry.height());
  QResizeEvent e(newSize, oldSize);
  if (m_interactorAdapter)
    {
    m_interactorAdapter->ProcessEvent(&e, m_interactor);
    }
  if(m_win.GetPointer())
    {
    m_win->SetSize(canvas()->width(), canvas()->height());
    QPointF origin = mapToScene(QPointF(0, 0));
    QPointF minPt(origin.x()/canvas()->width(), (canvas()->height() - origin.y() - height())/canvas()->height());
    QPointF maxPt(minPt.x() + width()/canvas()->width(), minPt.y() + height()/canvas()->height());
    if (m_win->GetRenderers()->GetFirstRenderer())
      {
      m_win->GetRenderers()->GetFirstRenderer()->SetViewport(minPt.x(), minPt.y(), maxPt.x(), maxPt.y());
      }
    update();
    }
}

void QVTKQuickItem::keyPressEvent(QKeyEvent* e)
{
  e->accept();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  update();
}

void QVTKQuickItem::keyReleaseEvent(QKeyEvent* e)
{
  e->accept();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  update();
}

void QVTKQuickItem::mousePressEvent(QMouseEvent* e)
{
  e->accept();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  update();
}

void QVTKQuickItem::mouseReleaseEvent(QMouseEvent* e)
{
  e->accept();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  update();
}

void QVTKQuickItem::mouseMoveEvent(QMouseEvent* e)
{
  e->accept();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  update();
}

void QVTKQuickItem::wheelEvent(QWheelEvent* e)
{
  e->accept();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  update();
}

void QVTKQuickItem::hoverEnterEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Enter);
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  update();
}

void QVTKQuickItem::hoverLeaveEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Leave);
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  update();
}

void QVTKQuickItem::hoverMoveEvent(QHoverEvent* e)
{
  e->accept();
  QMouseEvent e2(QEvent::MouseMove, e->pos(), Qt::NoButton, Qt::NoButton, e->modifiers());
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  update();
}

void QVTKQuickItem::init()
{
  vtkNew<vtkContextView> view;
  view->SetRenderWindow(m_win);
  vtkNew<vtkBlockItem> block;
  block->SetDimensions(0, 0, 100, 100);
  view->GetScene()->AddItem(block.GetPointer());
}

void QVTKQuickItem::prepareForRender()
{
}

void QVTKQuickItem::paint()
{
  this->m_viewLock.lock();

  if (!m_win.GetPointer()) {
    m_interactor = vtkSmartPointer<QVTKInteractor>::New();
    m_interactorAdapter = new QVTKInteractorAdapter(this);
    m_connect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    m_connect->Connect(m_interactor, vtkCommand::RenderEvent, this, SLOT(Update()));
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    this->geometryChanged(QRectF(x(), y(), width(), height()), QRectF(0, 0, 100, 100));
    this->SetRenderWindow(win);

    // Let subclasses do something on initialization
    init();
  }

  // Let subclasses do something each render
  prepareForRender();

  // Make sure viewport is up to date.
  // This is needed because geometryChanged() is not called when parent geometry changes, so we miss when widths/heights
  // of surrounding elements change.
  m_win->SetSize(canvas()->width(), canvas()->height());
  QPointF origin = mapToScene(QPointF(0, 0));
  QPointF minPt(origin.x()/canvas()->width(), (canvas()->height() - origin.y() - height())/canvas()->height());
  QPointF maxPt(minPt.x() + width()/canvas()->width(), minPt.y() + height()/canvas()->height());
  if (m_win->GetRenderers()->GetFirstRenderer())
    {
    m_win->GetRenderers()->GetFirstRenderer()->SetViewport(minPt.x(), minPt.y(), maxPt.x(), maxPt.y());
    }

  // Turn off any QML shader program
  glUseProgram(0);

  // Set blending correctly
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  m_win->Render();

  // Disable alpha test for QML
  glDisable(GL_ALPHA_TEST);

  this->m_viewLock.unlock();
}
