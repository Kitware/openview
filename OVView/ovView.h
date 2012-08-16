
#ifndef ovView_h
#define ovView_h

#include "QVTKGraphicsItem.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <QStringList>
#include <QUrl>

#include <vector>

class vtkContextView;
class vtkTable;

class ovView : public QVTKGraphicsItem
{
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl)
  Q_PROPERTY(QString viewType READ viewType WRITE setViewType)
  Q_PROPERTY(QStringList dataFields READ dataFields)
  Q_PROPERTY(QStringList viewAttributes READ viewAttributes)
public:
  ovView(QGraphicsItem *p=0);
  ~ovView();

  QUrl url() { return this->Url; }
  void setUrl(QUrl &url);

  QString viewType() { return this->ViewType; }
  void setViewType(QString &viewType);

  QStringList dataFields();
  QStringList viewAttributes();

protected:
  void setTable(vtkTable *data);
  void setupView();
  void setupGraph();
  void setupScatter();
  int basicType(int type);

  QUrl Url;
  QString ViewType;
  vtkNew<vtkContextView> View;
  vtkNew<vtkTable> Table;
  std::vector<int> Types;
  std::vector<std::vector<int> > Relationships;
};

#endif
