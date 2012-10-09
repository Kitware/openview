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

void ovScatterPlotView::setTable(vtkTable *table, vtkContextView *view)
{
  if (table != this->m_table.GetPointer())
    {
    std::vector<std::set<std::string> > domains = ovViewQuickItem::columnDomains(table);
    std::vector<int> types = ovViewQuickItem::columnTypes(table, domains);

    // Find best pair of columns for x/y
    vtkIdType numCol = table->GetNumberOfColumns();
    vtkIdType x = -1;
    vtkIdType y = -1;
    double bestScore = 0;
    for (vtkIdType col1 = 0; col1 < numCol; ++col1)
      {
      for (vtkIdType col2 = col1+1; col2 < numCol; ++col2)
        {
        int type1 = types[col1];
        int type2 = types[col2];
        bool numeric1 = (type1 == CONTINUOUS || type1 == INTEGER_CATEGORY || type1 == INTEGER_DATA);
        bool numeric2 = (type1 == CONTINUOUS || type1 == INTEGER_CATEGORY || type1 == INTEGER_DATA);
        if (bestScore < 10 && type1 == CONTINUOUS && type2 == CONTINUOUS)
          {
          bestScore = 10;
          x = col1;
          y = col2;
          }
        else if (bestScore < 8 && (type1 == CONTINUOUS && numeric2 || numeric1 && type2 == CONTINUOUS))
          {
          bestScore = 8;
          x = col1;
          y = col2;
          }
        else if (bestScore < 6 && numeric1 && numeric2)
          {
          bestScore = 6;
          x = col1;
          y = col2;
          }
        else if (bestScore < 1)
          {
          bestScore = 1;
          x = col1;
          y = col2;
          }
        }
      }
    //std::cerr << "SCATTER chose " << x << ", " << y << " with score " << bestScore << std::endl;

    if (x == -1 || y == -1)
      {
      return;
      }

    this->m_x = x;
    this->m_y = y;
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
  this->setAttribute("Color", m_color);
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
    m_plot->SetScalarVisibility(value != "(none)");
    m_plot->SelectColorArray(value.toStdString());
    vtkDataArray *arr = vtkDataArray::SafeDownCast(m_table->GetColumnByName(value.toAscii()));
    if (arr)
      {
      m_lookup->SetRange(arr->GetRange());
      }
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
}
