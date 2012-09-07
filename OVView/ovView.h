
#ifndef ovView_h
#define ovView_h

#include "QVTKQuickItem.h"
#include "vtkGraphItem.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <QStringList>
#include <QTimer>
#include <QUrl>

#include <vector>
#include <map>

class vtkContextView;
class vtkTable;

class ovGraphItem : public vtkGraphItem
{
public:
  static ovGraphItem *New();
  vtkTypeMacro(ovGraphItem, vtkGraphItem);

protected:
  ovGraphItem() {}
  ~ovGraphItem() {}

  virtual vtkStdString VertexTooltip(vtkIdType vertex);
  virtual vtkColor4ub VertexColor(vtkIdType vertex);

private:
  ovGraphItem(const ovGraphItem&); // Not implemented
  void operator=(const ovGraphItem&); // Not implemented
};

class ovView : public QVTKQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl)
  Q_PROPERTY(QString viewType READ viewType WRITE setViewType)
  Q_PROPERTY(QStringList viewAttributes READ viewAttributes)
public:
  ovView();
  ~ovView();

  QUrl url() { return this->Url; }
  void setUrl(QUrl &url);

  QString viewType() { return this->ViewType; }
  void setViewType(QString &viewType);

  QStringList viewAttributes();

public slots:
  QStringList dataFields(QString attribute);
  void setAttribute(QString attribute, QString value);
  QString getAttribute(QString attribute);
  void animateGraph();

protected:
  virtual void init();
  virtual void prepareForRender();

  void setTable(vtkTable *data);
  void setupView();
  void setupGraph();
  void setupScatter();
  int basicType(int type);

  QTimer AnimationTimer;
  QUrl Url;
  QString ViewType;
  vtkNew<vtkContextView> View;
  vtkNew<ovGraphItem> graphItem;
  vtkNew<vtkTable> Table;
  std::vector<int> Types;
  std::vector<std::vector<int> > Relationships;
  std::map<QString, std::map<QString, QString> > Attributes;
};

#endif
