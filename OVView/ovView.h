
#ifndef ovView_h
#define ovView_h

#include "QVTKGraphicsItem.h"
#include "vtkNew.h"

class vtkContextView;
class vtkGraphLayoutView;

class ovView : public QVTKGraphicsItem
{
  Q_OBJECT
public:
  ovView(QGraphicsItem *p=0);
  ~ovView();
protected:
  //vtkNew<vtkGraphLayoutView> View;
  vtkNew<vtkContextView> View;
};

#endif
