
#ifndef ovView_h
#define ovView_h

#include "QVTKQuickItem.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <QStringList>
#include <QUrl>

#include <vector>
#include <map>

class vtkContextView;
class vtkTable;

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
  std::map<QString, std::map<QString, QString> > Attributes;
};

#endif
