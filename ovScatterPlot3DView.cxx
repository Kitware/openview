/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovScatterPlot3DView.h"

#include "ovGraphItem.h"
#include "ovViewQuickItem.h"

#include "vtkAbstractArray.h"
#include "vtkAxis.h"
#include "vtkChartXYZ.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDataArray.h"
#include "vtkLookupTable.h"
#include "vtkPlotPoints3D.h"
#include "vtkTable.h"

ovScatterPlot3DView::ovScatterPlot3DView(QObject *parent) : ovView(parent)
{
  m_table = vtkSmartPointer<vtkTable>::New();
  m_color = "(none)";
  m_lookup->SetNumberOfTableValues(6);
  unsigned char colors[] = {
      //247,252,253,
      //229,245,249,
      //204,236,230,
      153,216,201,
      102,194,164,
      65,174,118,
      35,139,69,
      0,109,44,
      0,68,27};
  for (int i = 0; i < 6; ++i)
    {
    m_lookup->SetTableValue(
          i, colors[i*3] / 255., colors[i*3+1] / 255., colors[i*3+2] / 255., 1.);
    }
  m_lookup->IndexedLookupOff();
}

ovScatterPlot3DView::~ovScatterPlot3DView()
{
}

void ovScatterPlot3DView::setTable(vtkTable *table, vtkContextView *view)
{
  if (table != this->m_table.GetPointer())
    {
    // Find best pair of columns for x/y
    vtkIdType numCol = table->GetNumberOfColumns();
    vtkIdType x = -1;
    vtkIdType y = -1;
    vtkIdType z = -1;
    vtkIdType color = -1;
    for (vtkIdType i = 0; i < numCol; ++i)
      {
      if (vtkDataArray::SafeDownCast(table->GetColumn(i)))
        {
        if (x == -1)
          {
          x = i;
          }
        else if (y == -1)
          {
          y = i;
          }
        else if (z == -1)
          {
          z = i;
          }
        else if (color == -1)
          {
          color = i;
          }
        }
      }
    if (x == -1)
      {
      return;
      }
    if (y == -1)
      {
      y = x;
      }
    if (z == -1)
      {
      z = y;
      }
    if (color == -1)
      {
      color = z;
      }

    this->m_x = table->GetColumnName(x);
    this->m_y = table->GetColumnName(y);
    this->m_z = table->GetColumnName(z);
    this->m_color = table->GetColumnName(color);
    }

  view->GetScene()->AddItem(m_chart.GetPointer());
  this->m_table = table;

  this->generatePlot();
}

void ovScatterPlot3DView::generatePlot()
{
  m_chart->SetGeometry(vtkRectf(0.0, 0.0, 1000, 1000));

  if (m_table->GetColumnByName(m_color.toAscii()))
    {
    m_plot->SetInputData(m_table.GetPointer(), m_x.toStdString(), m_y.toStdString(), m_z.toStdString(), m_color.toStdString());
    }
  else
    {
    m_plot->SetInputData(m_table.GetPointer(), m_x.toStdString(), m_y.toStdString(), m_z.toStdString());
    }
  m_chart->RecalculateBounds();
  m_chart->RecalculateTransform();

  vtkDataArray *arr = vtkDataArray::SafeDownCast(m_table->GetColumnByName(m_color.toAscii()));
  if (arr)
    {
    m_lookup->SetRange(arr->GetRange());
    }
  //m_plot->SetColor(0, 0, 0, 255);
  //m_plot->SetWidth(1.0);
  //m_plot->SetIndexedLabels(labels.GetPointer());
  //m_plot->SetTooltipLabelFormat("(%x, %y)");
  //m_chart->GetAxis(vtkAxis::LEFT)->SetTitle(m_table->GetColumnName(this->m_y));
  //m_chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(m_table->GetColumnName(this->m_x));
  //m_plot->SetMarkerStyle(vtkPlotPoints::CIRCLE);
  //m_plot->SetMarkerSize(10);
  //m_plot->SetColor(128, 128, 128, 255);
  //m_plot->SetLookupTable(m_lookup.GetPointer());
  //this->setAttribute("Color", m_color);
}

QString ovScatterPlot3DView::name()
{
  return "3D SCATTER";
}

QStringList ovScatterPlot3DView::attributes()
{
  return QStringList() << "X" << "Y" << "Z" << "Color";
}

QStringList ovScatterPlot3DView::attributeOptions(QString attribute)
{
  if (attribute == "X" || attribute == "Y" || attribute == "Z" || attribute == "Color")
    {
    QStringList fields;
    if (attribute == "Color")
      {
      fields << "(none)";
      }
    for (vtkIdType col = 0; col < this->m_table->GetNumberOfColumns(); ++col)
      {
      if (vtkDataArray::SafeDownCast(m_table->GetColumn(col)))
        {
        fields << this->m_table->GetColumn(col)->GetName();
        }
      }
    return fields;
    }
  return QStringList();
}

void ovScatterPlot3DView::setAttribute(QString attribute, QString value)
{
  if (attribute == "X")
    {
    m_x = value;
    this->generatePlot();
    return;
    }
  if (attribute == "Y")
    {
    m_y = value;
    this->generatePlot();
    return;
    }
  if (attribute == "Z")
    {
    m_z = value;
    this->generatePlot();
    return;
    }
  if (attribute == "Color")
    {
    m_color = value;
    this->generatePlot();
    return;
    }
}

QString ovScatterPlot3DView::getAttribute(QString attribute)
{
  if (attribute == "X")
    {
    return m_x;
    }
  if (attribute == "Y")
    {
    return m_y;
    }
  if (attribute == "Z")
    {
    return m_z;
    }
  if (attribute == "Color")
    {
    return m_color;
    }
  return QString();
}
