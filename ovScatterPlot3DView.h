/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovScatterPlot3DView_h
#define ovScatterPlot3DView_h

#include "ovView.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkInteractiveChartXYZ;
class vtkContextView;
class vtkLookupTable;
class vtkTable;

class ovScatterPlot3DView : public ovView
{
  Q_OBJECT
public:
  ovScatterPlot3DView(QObject *parent);
  ~ovScatterPlot3DView();

  virtual void setTable(vtkTable *data, vtkContextView *view);
  virtual QString name();

  virtual QStringList attributes();
  virtual QStringList attributeOptions(QString attribute);
  virtual void setAttribute(QString attribute, QString value);
  virtual QString getAttribute(QString attribute);

protected:
  void generatePlot();

  vtkSmartPointer<vtkTable> m_table;
  vtkNew<vtkInteractiveChartXYZ> m_chart;
  vtkNew<vtkLookupTable> m_lookup;
  QString m_x;
  QString m_y;
  QString m_z;
  QString m_color;
};

#endif
