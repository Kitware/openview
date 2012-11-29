/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovTreeView_h
#define ovTreeView_h

#include "ovView.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkTooltipItem.h"

class vtkContextView;
class vtkTable;
class vtkTree;
class vtkTreeHeatmapItem;

class ovTreeView : public ovView
{
  Q_OBJECT
public:
  ovTreeView(QObject *parent);
  ~ovTreeView();

  virtual bool acceptsType(const QString &type);
  virtual void setData(vtkDataObject *data, vtkContextView *view);
  virtual void setTable(vtkTable *data, vtkContextView *view);
  virtual void setTree(vtkTree *data, vtkContextView *view);
  virtual QString name();

  virtual QStringList attributes();
  virtual QStringList attributeOptions(QString attribute);
  virtual void setAttribute(QString attribute, QString value);
  virtual QString getAttribute(QString attribute);

protected:
  void generateTree();
  void generateView();

  QString m_level1;
  QString m_level2;
  QString m_label;
  QString m_depth;
  vtkNew<vtkTreeHeatmapItem> m_item;
  vtkSmartPointer<vtkTable> m_table;
  vtkSmartPointer<vtkTree> m_tree;
};

#endif
