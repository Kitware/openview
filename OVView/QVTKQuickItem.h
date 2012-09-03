#ifndef QVTKQuickItem_h
#define QVTKQuickItem_h

#include <QQuickItem>

#include <QOpenGLShaderProgram>

#include "vtkSmartPointer.h"
#include "vtkNew.h"

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QVTKInteractorAdapter;
class QVTKInteractor;
class vtkEventQtSlotConnect;
class vtkGenericOpenGLRenderWindow;
class vtkObject;
class vtkContextView;

class QVTKQuickItem : public QQuickItem
{
  Q_OBJECT
public:
  QVTKQuickItem();

  // Description:
  // destructor
  ~QVTKQuickItem();

  void itemChange(ItemChange change, const ItemChangeData &);

  // Description:
  // set the render window to use with this item
  void SetRenderWindow(vtkGenericOpenGLRenderWindow* win);

  // Description:
  // get the render window used with this item
  vtkGenericOpenGLRenderWindow* GetRenderWindow() const;

  // Description:
  // get the render window interactor used with this item
  // this item enforces its own interactor
  QVTKInteractor* GetInteractor() const;

public slots:
  // Description:
  // update this item in the view (this does not cause the vtk render window to draw)
  // it just causes the current contents in the window to draw to the QGraphicsScene
  virtual void Update();

  void paint();
  void vtkpaint();

protected slots:
  // slot to make this vtk render window current
  virtual void MakeCurrent();
  // slot called when vtk render window starts to draw
  virtual void Start();
  // slot called when vtk render window is done drawing
  virtual void End();
  // slot called when vtk wants to know if the context is current
  virtual void IsCurrent(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
  // slot called when vtk wants to know if a window is direct
  virtual void IsDirect(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
  // slot called when vtk wants to know if a window supports OpenGL
  virtual void SupportsOpenGL(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);

private:
  QOpenGLShaderProgram *m_program;
  QOpenGLContext* mContext;
  QOpenGLFramebufferObject* mFBO;
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> mWin;
  vtkSmartPointer<QVTKInteractor> mIren;
  QVTKInteractorAdapter* mIrenAdapter;
  vtkSmartPointer<vtkEventQtSlotConnect> mConnect;
  vtkNew<vtkContextView> mView;
};

#endif
