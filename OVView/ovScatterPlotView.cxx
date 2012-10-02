#include "ovScatterPlotView.h"

#include "ovGraphItem.h"
#include "ovViewQuickItem.h"

#include "vtkAbstractArray.h"
#include "vtkAxis.h"
#include "vtkChartXY.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDataArray.h"
#include "vtkPlot.h"
#include "vtkPlotPoints.h"
#include "vtkTable.h"

ovScatterPlotView::ovScatterPlotView(QObject *parent) : ovView(parent)
{
  m_table = vtkSmartPointer<vtkTable>::New();
  m_x = 0;
  m_y = 0;
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
  vtkPlot *points = m_chart->AddPlot(vtkChart::POINTS);
  points->SetInputData(m_table, m_x, m_y);
  points->SetColor(0, 0, 0, 255);
  points->SetWidth(1.0);
  //points->SetIndexedLabels(labels.GetPointer());
  points->SetTooltipLabelFormat("(%x, %y)");
  m_chart->GetAxis(vtkAxis::LEFT)->SetTitle(m_table->GetColumnName(this->m_y));
  m_chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(m_table->GetColumnName(this->m_x));
  vtkPlotPoints::SafeDownCast(points)->SetMarkerStyle(vtkPlotPoints::CIRCLE);
  vtkPlotPoints::SafeDownCast(points)->SetMarkerSize(10);
  points->SetColor(128, 128, 128, 255);
}

QString ovScatterPlotView::name()
{
  return "SCATTER";
}

QStringList ovScatterPlotView::attributes()
{
  return QStringList() << "X" << "Y";
}

QStringList ovScatterPlotView::attributeOptions(QString attribute)
{
  if (attribute == "X" || attribute == "Y")
    {
    QStringList fields;
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
}
