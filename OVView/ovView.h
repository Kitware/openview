
#ifndef ovView_h
#define ovView_h

#include "QVTKGraphicsItem.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <QUrl>

#include <vector>

class vtkContextView;
class vtkGraphLayoutView;
class vtkTable;

class ovView : public QVTKGraphicsItem
{
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl)
public:
  ovView(QGraphicsItem *p=0);
  ~ovView();

  QUrl url() { return this->Url; }
  void setUrl(QUrl &url);

protected:
  void setTable(vtkTable *data);
  void setupGraph();

  QUrl Url;
  vtkNew<vtkContextView> View;
  vtkNew<vtkTable> Table;
  std::vector<int> Types;
  std::vector<std::vector<int> > Relationships;
};

#endif
