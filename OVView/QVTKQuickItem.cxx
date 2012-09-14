
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
}

void QVTKQuickItem::Start()
{
  MakeCurrent();

  if (!mWin->GetAbortRender())
    {
    mWin->PushState();
    mWin->OpenGLInitState();
    }
}

void QVTKQuickItem::End()
{
  if (!mWin->GetAbortRender())
    {
    mWin->PopState();
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
  if (mIrenAdapter)
    {
    mIrenAdapter->ProcessEvent(&e, mIren);
    }
  if(mWin.GetPointer())
    {
    mWin->SetSize(canvas()->width(), canvas()->height());
    QPointF origin = mapToScene(QPointF(0, 0));
    QPointF minPt(origin.x()/canvas()->width(), (canvas()->height() - origin.y() - height())/canvas()->height());
    QPointF maxPt(minPt.x() + width()/canvas()->width(), minPt.y() + height()/canvas()->height());
    if (mWin->GetRenderers()->GetFirstRenderer())
      {
      mWin->GetRenderers()->GetFirstRenderer()->SetViewport(minPt.x(), minPt.y(), maxPt.x(), maxPt.y());
      }
    update();
    }
}

void QVTKQuickItem::keyPressEvent(QKeyEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
  update();
}

void QVTKQuickItem::keyReleaseEvent(QKeyEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
  update();
}

void QVTKQuickItem::mousePressEvent(QMouseEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
  update();
}

void QVTKQuickItem::mouseReleaseEvent(QMouseEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
  update();
}

void QVTKQuickItem::mouseMoveEvent(QMouseEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
  update();
}

void QVTKQuickItem::wheelEvent(QWheelEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
  update();
}

void QVTKQuickItem::hoverEnterEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Enter);
  mIrenAdapter->ProcessEvent(&e2, mIren);
  update();
}

void QVTKQuickItem::hoverLeaveEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Leave);
  mIrenAdapter->ProcessEvent(&e2, mIren);
  update();
}

void QVTKQuickItem::hoverMoveEvent(QHoverEvent* e)
{
  e->accept();
  QMouseEvent e2(QEvent::MouseMove, e->pos(), Qt::NoButton, Qt::NoButton, e->modifiers());
  mIrenAdapter->ProcessEvent(&e2, mIren);
  update();
}

void QVTKQuickItem::init()
{
  vtkNew<vtkContextView> view;
  view->SetRenderWindow(mWin);
  vtkNew<vtkBlockItem> block;
  block->SetDimensions(0, 0, 100, 100);
  view->GetScene()->AddItem(block.GetPointer());
}

void QVTKQuickItem::prepareForRender()
{
}

void QVTKQuickItem::paint()
{
  this->ViewLock.lock();

  if (!mWin.GetPointer()) {
    mIren = vtkSmartPointer<QVTKInteractor>::New();
    mIrenAdapter = new QVTKInteractorAdapter(this);
    mConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    mConnect->Connect(mIren, vtkCommand::RenderEvent, this, SLOT(Update()));
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
  mWin->SetSize(canvas()->width(), canvas()->height());
  QPointF origin = mapToScene(QPointF(0, 0));
  QPointF minPt(origin.x()/canvas()->width(), (canvas()->height() - origin.y() - height())/canvas()->height());
  QPointF maxPt(minPt.x() + width()/canvas()->width(), minPt.y() + height()/canvas()->height());
  if (mWin->GetRenderers()->GetFirstRenderer())
    {
    mWin->GetRenderers()->GetFirstRenderer()->SetViewport(minPt.x(), minPt.y(), maxPt.x(), maxPt.y());
    }

  // Turn off any QML shader program
  glUseProgram(0);

  // Set blending correctly
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  mWin->Render();

  // Disable alpha test for QML
  glDisable(GL_ALPHA_TEST);

  this->ViewLock.unlock();
}
