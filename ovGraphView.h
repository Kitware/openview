/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovGraphView_h
#define ovGraphView_h

#include "ovView.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class ovGraphItem;
class vtkContextView;
class vtkTable;

class ovGraphView : public ovView
{
  Q_OBJECT
public:
  ovGraphView(QObject *parent);
  ~ovGraphView();

  virtual bool acceptsType(const QString &type);
  virtual void setData(vtkDataObject *data, vtkContextView *view);
  virtual QString name();
  virtual void prepareForRender();

  virtual QStringList attributes();
  virtual QStringList attributeOptions(QString attribute);
  virtual void setAttribute(QString attribute, QString value);
  virtual QString getAttribute(QString attribute);

protected:
  void generateGraph();

  QString m_source;
  QString m_target;
  bool m_animate;
  bool m_sharedDomain;
  vtkNew<ovGraphItem> m_item;
  vtkSmartPointer<vtkTable> m_table;
  bool m_bundle;
};

#endif
