/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovViewQuickItem_h
#define ovViewQuickItem_h

#include "QVTKQuickItem.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <QStringList>
#include <QUrl>

#include "ovView.h"

#include <vector>
#include <map>
#include <set>

class vtkContextView;
class vtkTable;

enum {
  CONTINUOUS,
  INTEGER_CATEGORY,
  INTEGER_DATA,
  STRING_CATEGORY,
  STRING_DATA
};

enum {
  UNRELATED,
  SHARED_DOMAIN
};

class ovViewQuickItem : public QVTKQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl)
  Q_PROPERTY(QString viewType READ viewType WRITE setViewType)
  Q_PROPERTY(QStringList attributes READ attributes)
public:
  ovViewQuickItem();
  ~ovViewQuickItem();

  static std::vector<std::set<std::string> > columnDomains(vtkTable *table);
  static std::vector<int> columnTypes(vtkTable *table, const std::vector<std::set<std::string> > &domains);
  static void convertTableColumns(vtkTable *table, const std::vector<int> &types);
  static std::vector<std::vector<int> > columnRelations(vtkTable *table, const std::vector<std::set<std::string> > &domains, const std::vector<int> &types);
  static int basicType(int type);

  QUrl url() { return this->m_url; }
  void setUrl(QUrl &url);

  QString viewType() { return this->m_viewType; }
  void setViewType(QString &viewType);

public slots:
  QStringList viewTypes();
  QStringList attributes();
  QStringList attributeOptions(QString attribute);
  void setAttribute(QString attribute, QString value);
  QString getAttribute(QString attribute);
  int tableRows();
  int tableColumns();
  QString tableColumnName(int col);
  QString tableData(int row, int col);
  void animate();

protected:
  virtual void init();
  virtual void prepareForRender();

  void setTable(vtkTable *data);
  void setupView();

  QUrl m_url;
  QString m_viewType;
  vtkNew<vtkContextView> m_view;
  vtkSmartPointer<vtkTable> m_table;
  std::map<QString, ovView*> m_views;
};

#endif
