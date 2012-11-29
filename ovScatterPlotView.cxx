/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovScatterPlotView.h"

#include "ovGraphItem.h"
#include "ovViewQuickItem.h"

#include "vtkAbstractArray.h"
#include "vtkAxis.h"
#include "vtkChartXY.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDataArray.h"
#include "vtkLookupTable.h"
#include "vtkPlot.h"
#include "vtkPlotPoints.h"
#include "vtkTable.h"

ovScatterPlotView::ovScatterPlotView(QObject *parent) : ovView(parent)
{
  m_table = vtkSmartPointer<vtkTable>::New();
  m_plot = vtkSmartPointer<vtkPlotPoints>::New();
  m_color = "(none)";
  m_x = 0;
  m_y = 0;
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

ovScatterPlotView::~ovScatterPlotView()
{
}

bool ovScatterPlotView::acceptsType(const QString &type)
{
  return (type == "vtkTable");
}

void ovScatterPlotView::setData(vtkDataObject *data, vtkContextView *view)
{
  vtkTable *table = vtkTable::SafeDownCast(data);
  if (!table)
    {
    return;
    }
  if (table != this->m_table.GetPointer())
    {
    std::vector<std::set<std::string> > domains = ovViewQuickItem::columnDomains(table);
    std::vector<int> types = ovViewQuickItem::columnTypes(table, domains);

    // Find best pair of columns for x/y
    vtkIdType numCol = table->GetNumberOfColumns();
    vtkIdType x = -1;
    vtkIdType y = -1;
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
    if (color == -1)
      {
      color = y;
      }

    this->m_x = x;
    this->m_y = y;
    this->m_color = m_table->GetColumnName(color);
    }

  view->GetScene()->AddItem(m_chart.GetPointer());
  m_chart->SetShowLegend(false);
  this->m_table = table;

  this->generatePlot();
}

void ovScatterPlotView::generatePlot()
{
  m_chart->ClearPlots();
  m_plot = vtkPlotPoints::SafeDownCast(m_chart->AddPlot(vtkChart::POINTS));
  m_plot->SetInputData(m_table, m_x, m_y);
  m_plot->SetColor(0, 0, 0, 255);
  m_plot->SetWidth(1.0);
  //m_plot->SetIndexedLabels(labels.GetPointer());
  m_plot->SetTooltipLabelFormat("(%x, %y)");
  m_chart->GetAxis(vtkAxis::LEFT)->SetTitle(m_table->GetColumnName(this->m_y));
  m_chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(m_table->GetColumnName(this->m_x));
  m_plot->SetMarkerStyle(vtkPlotPoints::CIRCLE);
  m_plot->SetMarkerSize(10);
  m_plot->SetColor(128, 128, 128, 255);
  m_plot->SetLookupTable(m_lookup.GetPointer());
  m_plot->SetScalarVisibility(m_color != "(none)");
  m_plot->SelectColorArray(m_color.toStdString());
  vtkDataArray *arr = vtkDataArray::SafeDownCast(m_table->GetColumnByName(m_color.toAscii()));
  if (arr)
    {
    m_lookup->SetRange(arr->GetRange());
    }
}

QString ovScatterPlotView::name()
{
  return "SCATTER";
}

QStringList ovScatterPlotView::attributes()
{
  return QStringList() << "X" << "Y" << "Color";
}

QStringList ovScatterPlotView::attributeOptions(QString attribute)
{
  if (attribute == "X" || attribute == "Y" || attribute == "Color")
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

int ovScatterPlotView::tableColumnIndex(QString value)
{
  for (int i = 0; i < m_table->GetNumberOfColumns(); ++i)
    {
    if (m_table->GetColumnName(i) == value)
      {
      return i;
      }
    }
  return -1;
}

void ovScatterPlotView::setAttribute(QString attribute, QString value)
{
  if (attribute == "X")
    {
    m_x = this->tableColumnIndex(value);
    this->generatePlot();
    return;
    }
  if (attribute == "Y")
    {
    m_y = this->tableColumnIndex(value);
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

QString ovScatterPlotView::getAttribute(QString attribute)
{
  if (attribute == "X")
    {
    return m_table->GetColumnName(m_x);
    }
  if (attribute == "Y")
    {
    return m_table->GetColumnName(m_y);
    }
  if (attribute == "Color")
    {
    return m_color;
    }
  return QString();
}
