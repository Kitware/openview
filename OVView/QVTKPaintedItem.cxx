/*=========================================================================

  Program:   Visualization Toolkit
  Module:    QVTKPaintedItem.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2010 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "QVTKPaintedItem.h"
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QQuickCanvas>
#include <QThread>

#include "QVTKInteractor.h"
#include "QVTKInteractorAdapter.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkgl.h"

QVTKPaintedItem::QVTKPaintedItem(QQuickItem* p)
  : QQuickPaintedItem(p)
{
  mFBO = NULL;
  mIren = vtkSmartPointer<QVTKInteractor>::New();
  mIrenAdapter = new QVTKInteractorAdapter(NULL);
  mConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  mConnect->Connect(mIren, vtkCommand::RenderEvent, this, SLOT(Update()));
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  this->SetRenderWindow(win);

  //this->setFlag(QGraphicsItem::ItemIsFocusable, true);
  //setFocusPolicy(Qt::ClickFocus);
  setAcceptHoverEvents(true);
  //this->setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding));
  //QPalette pal = this->palette();
  //pal.setColor(QPalette::Window, QColor(255,255,255,255));
  //this->setPalette(pal);
}

QVTKPaintedItem::~QVTKPaintedItem()
{
  if(mFBO)
    delete mFBO;
  if (mIrenAdapter)
    delete mIrenAdapter;
}

void QVTKPaintedItem::SetRenderWindow(vtkGenericOpenGLRenderWindow* win)
{
  if(mWin)
  {
    mWin->SetMapped(0);
    mConnect->Disconnect(mWin, vtkCommand::StartEvent, this, SLOT(Start()));
    mConnect->Disconnect(mWin, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    mConnect->Disconnect(mWin, vtkCommand::EndEvent, this, SLOT(End()));
    mConnect->Disconnect(mWin, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
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

    mConnect->Connect(mWin, vtkCommand::StartEvent, this, SLOT(Start()));
    mConnect->Connect(mWin, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    mConnect->Connect(mWin, vtkCommand::EndEvent, this, SLOT(End()));
    mConnect->Connect(mWin, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    mConnect->Connect(mWin, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    mConnect->Connect(mWin, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    mConnect->Connect(mWin, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
  }
}

vtkGenericOpenGLRenderWindow* QVTKPaintedItem::GetRenderWindow() const
{
  return mWin;
}

QVTKInteractor* QVTKPaintedItem::GetInteractor() const
{
  return mIren;
}

void QVTKPaintedItem::Update()
{
  if(this->mWin && this->mFBO)
    {
    QRectF bf = boundingRect();
    QRect bounds(bf.x(), bf.y(), bf.width(), bf.height());
    this->update(bounds);
    }
};

void QVTKPaintedItem::MakeCurrent()
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

void QVTKPaintedItem::Start()
{
  MakeCurrent();

  if(!mFBO)
  {
    mWin->SetAbortRender(1);
    return;
  }

  mWin->PushState();
  mWin->OpenGLInitState();
}

void QVTKPaintedItem::End()
{
  if(!mFBO)
    return;

  mWin->PopState();

  mFBO->release();
}

void QVTKPaintedItem::IsCurrent(vtkObject*, unsigned long, void*, void* call_data)
{
  if(mFBO)
  {
    bool* ptr = reinterpret_cast<bool*>(call_data);
    *ptr = QOpenGLContext::currentContext() == this->canvas()->openglContext() && mFBO->isBound();
  }
}

void QVTKPaintedItem::IsDirect(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = 1;
}

void QVTKPaintedItem::SupportsOpenGL(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = true;
  //*ptr = QGLFormat::hasOpenGL();
}


void QVTKPaintedItem::paint(QPainter *painter)
{
  cerr << "Painting!!" << endl;
  if(!mWin)
    return;

  // tell Qt we're doing our own GL calls
  // if necessary, it'll put us in an OpenGL 1.x compatible state.
  painter->beginNativePainting();

  if(!mFBO || QSize(this->width(), this->height()) != mFBO->size() || mWin->GetNeverRendered())
    {
    // first time or is enabled
    // if its not the first time and it is disabled, don't update the scene
    if(!mFBO || isEnabled())
      {
      cerr << "really rendering" << endl;
      mIren->Render();
      }
    }

  cerr << "really rendering 2" << endl;
  mIren->Render();

  if(!mFBO)
    return;

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

  painter->endNativePainting();
}

#if 0
void QVTKPaintedItem::keyPressEvent(QKeyEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
}

void QVTKPaintedItem::keyReleaseEvent(QKeyEvent* e)
{
  e->accept();
  mIrenAdapter->ProcessEvent(e, mIren);
}

void QVTKPaintedItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
  QPointF pf = e->pos();
  QPoint pi = pf.toPoint();

  e->accept();
  QMouseEvent e2(QEvent::MouseButtonPress, pi, e->button(),
      e->buttons(), e->modifiers());
  mIrenAdapter->ProcessEvent(&e2, mIren);
}

void QVTKPaintedItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
  QPointF pf = e->pos();
  QPoint pi = pf.toPoint();
  e->accept();
  QMouseEvent e2(QEvent::MouseButtonRelease, pi, e->button(),
      e->buttons(), e->modifiers());
  mIrenAdapter->ProcessEvent(&e2, mIren);
}

void QVTKPaintedItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
  QPointF pf = e->pos();
  QPoint pi = pf.toPoint();
  e->accept();
  QMouseEvent e2(QEvent::MouseMove, pi, e->button(),
      e->buttons(), e->modifiers());
  mIrenAdapter->ProcessEvent(&e2, mIren);
}

void QVTKPaintedItem::wheelEvent(QGraphicsSceneWheelEvent* e)
{
  e->accept();
  QWheelEvent e2(e->pos().toPoint(), e->scenePos().toPoint(), e->delta(),
      e->buttons(), e->modifiers(), e->orientation());
  mIrenAdapter->ProcessEvent(&e2, mIren);
}

void QVTKPaintedItem::resizeEvent(QGraphicsSceneResizeEvent* e)
{
  e->accept();
  QResizeEvent e2(e->newSize().toSize(), e->oldSize().toSize());
  mIrenAdapter->ProcessEvent(&e2, mIren);
  if(mWin)
    mWin->SetSize(e2.size().width(), e2.size().height());

}

void QVTKPaintedItem::moveEvent(QGraphicsSceneMoveEvent* e)
{
  e->accept();
  QMoveEvent e2(e->newPos().toPoint(), e->oldPos().toPoint());
  if(mWin)
    mWin->SetPosition(e2.pos().x(), e2.pos().y());
}

void QVTKPaintedItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Enter);
  mIrenAdapter->ProcessEvent(&e2, mIren);
}

void QVTKPaintedItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Leave);
  mIrenAdapter->ProcessEvent(&e2, mIren);
}

void QVTKPaintedItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
  e->accept();
  QPointF pf = e->pos();
  QPoint pi = pf.toPoint();
  QMouseEvent e2(QEvent::MouseMove, pi, Qt::NoButton, Qt::NoButton, e->modifiers());
  mIrenAdapter->ProcessEvent(&e2, mIren);
}
#endif

void QVTKPaintedItem::geometryChanged(const QRectF &old, const QRectF& current)
{
  QQuickItem::geometryChanged(old, current);
  cerr << "geometryChanged" << endl;

  QResizeEvent e2(QSize(current.size().width(), current.size().height()),
    QSize(old.size().width(), old.size().height()));
  mIrenAdapter->ProcessEvent(&e2, mIren);
  if(mWin)
    {
    mWin->SetSize(current.width(), current.height());
    }
}
