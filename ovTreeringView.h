/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovTreeringView_h
#define ovTreeringView_h

#include "ovView.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkTooltipItem.h"

class ovTreeringItem;
class vtkContextView;
class vtkTable;

class ovTreeringView : public ovView
{
  Q_OBJECT
public:
  ovTreeringView(QObject *parent);
  ~ovTreeringView();

  virtual void setTable(vtkTable *data, vtkContextView *view);
  virtual QString name();

  virtual QStringList attributes();
  virtual QStringList attributeOptions(QString attribute);
  virtual void setAttribute(QString attribute, QString value);
  virtual QString getAttribute(QString attribute);

protected:
  void generateTreering();

  QString m_level1;
  QString m_level2;
  QString m_color;
  QString m_size;
  QString m_hover;
  QString m_label;
  vtkNew<ovTreeringItem> m_item;
  vtkSmartPointer<vtkTable> m_table;
};

#endif
