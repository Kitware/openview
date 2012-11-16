/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovViewQuickItem.h"

#include "ovContextInteractorStyle.h"
#include "ovGraphView.h"
#include "ovScatterPlotView.h"
#include "ovScatterPlot3DView.h"
#include "ovTreemapView.h"
#include "ovTreeringView.h"

#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDataSetAttributes.h"
#include "vtkDelimitedTextReader.h"
#include "vtkDoubleArray.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTableReader.h"

#include <QOpenGLContext>
#include <QQuickCanvas>
#include <QThread>

#include <set>
#include <algorithm>

ovViewQuickItem::ovViewQuickItem()
{
  this->m_views["GRAPH"] = new ovGraphView(this);
  this->m_views["SCATTER"] = new ovScatterPlotView(this);
  this->m_views["3D SCATTER"] = new ovScatterPlot3DView(this);
  this->m_views["TREEMAP"] = new ovTreemapView(this);
  this->m_views["TREERING"] = new ovTreeringView(this);
  this->m_table = vtkSmartPointer<vtkTable>::New();
}

ovViewQuickItem::~ovViewQuickItem()
{
}

QStringList ovViewQuickItem::viewTypes()
{
  QStringList types;
  std::map<QString, ovView*>::iterator it, itEnd;
  itEnd = this->m_views.end();
  for (it = this->m_views.begin(); it != itEnd; ++it)
    {
    types << it->first;
    }
  return types;
}

void ovViewQuickItem::init()
{
  this->GetRenderWindow()->SetPolygonSmoothing(true);
  this->m_view->SetRenderWindow(this->GetRenderWindow());
  vtkNew<ovContextInteractorStyle> style;
  style->SetScene(this->m_view->GetScene());
  this->m_view->GetInteractor()->SetInteractorStyle(style.GetPointer());
  this->m_viewType = "GRAPH";
  //QUrl url("file:///code/opendemo/data/domains.csv");
  //QUrl url("file:///code/opendemo/data/classes.csv");
  //QUrl url("file:///code/opendemo/data/kcore_edges.csv");
  //this->setUrl(url);
}

void ovViewQuickItem::prepareForRender()
{
  this->m_views[this->m_viewType]->prepareForRender();
}

void ovViewQuickItem::setUrl(QUrl &url)
{
  this->m_viewLock.lock();
  this->m_url = url;
  QString fileName = url.toLocalFile();
  vtkSmartPointer<vtkTable> table;
  if (fileName.endsWith(".vtk"))
    {
    vtkNew<vtkTableReader> reader;
    reader->SetFileName(fileName.toAscii());
    reader->Update();
    table = reader->GetOutput();
    }
  else // delimited text
    {
    vtkNew<vtkDelimitedTextReader> reader;
    reader->SetFileName(fileName.toAscii());
    reader->SetHaveHeaders(true);
    if (fileName.endsWith(".tab") || fileName.endsWith(".tsv"))
      {
      reader->SetFieldDelimiterCharacters("\t");
      }
    reader->Update();
    table = reader->GetOutput();

    // Figure out if it really has headers
    // Are the column names contained in their own columns?
    int matchCount = 0;
    for (vtkIdType col = 0; col < table->GetNumberOfColumns(); ++col)
      {
      vtkAbstractArray *column = table->GetColumn(col);
      vtkVariant name(column->GetName());
      if (column->LookupValue(name) >= 0)
        {
        ++matchCount;
        }
      }
    if (matchCount > 0)
      {
      reader->SetHaveHeaders(false);
      reader->Update();
      table = reader->GetOutput();
      }
    }
  this->setTable(table);
  this->m_viewLock.unlock();
}

int ovViewQuickItem::tableRows()
{
  return m_table->GetNumberOfRows();
}

int ovViewQuickItem::tableColumns()
{
  return m_table->GetNumberOfColumns();
}

QString ovViewQuickItem::tableColumnName(int col)
{
  if (col >= 0 && col < m_table->GetNumberOfColumns())
    {
    return QString(m_table->GetColumnName(col));
    }
  return QString();
}

QString ovViewQuickItem::tableData(int row, int col)
{
  if (row >= 0 && row < m_table->GetNumberOfRows() && col >= 0 && col < m_table->GetNumberOfColumns())
    {
    return QString::fromStdString(m_table->GetValue(row, col).ToString());
    }
  return QString();
}

QStringList ovViewQuickItem::attributeOptions(QString attribute)
{
  this->m_viewLock.lock();
  QStringList result = this->m_views[this->m_viewType]->attributeOptions(attribute);
  this->m_viewLock.unlock();
  return result;
}

QStringList ovViewQuickItem::attributes()
{
  this->m_viewLock.lock();
  QStringList result = this->m_views[this->m_viewType]->attributes();
  this->m_viewLock.unlock();
  return result;
}

void ovViewQuickItem::setAttribute(QString attribute, QString value)
{
  this->m_viewLock.lock();
  this->m_views[this->m_viewType]->setAttribute(attribute, value);
  this->m_viewLock.unlock();
}

QString ovViewQuickItem::getAttribute(QString attribute)
{
  this->m_viewLock.lock();
  QString result = this->m_views[this->m_viewType]->getAttribute(attribute);
  this->m_viewLock.unlock();
  return result;
}

int ovViewQuickItem::basicType(int type)
{
  if (type == INTEGER_DATA || type == INTEGER_CATEGORY)
    {
    return 0;
    }
  if (type == STRING_DATA || type == STRING_CATEGORY)
    {
    return 1;
    }
  return 2;
}

std::vector<int> ovViewQuickItem::columnTypes(vtkTable *table, const std::vector<std::set<std::string> > &domains)
{
  vtkIdType numRow = table->GetNumberOfRows();
  vtkIdType numCol = table->GetNumberOfColumns();
  std::vector<int> types = std::vector<int>(numCol);
  for (vtkIdType col = 0; col < numCol; ++col)
    {
    vtkIdType numFractional = 0;
    vtkIdType numNumeric = 0;
    for (vtkIdType row = 0; row < numRow; ++row)
      {
      std::string stringVal = table->GetValue(row, col).ToString();
      vtkVariant value(stringVal);
      bool ok;
      double doubleVal = value.ToDouble(&ok);
      if (ok)
        {
        numNumeric++;
        double dummy;
        if (std::modf(doubleVal, &dummy) != 0.0)
          {
          numFractional++;
          }
        }
      }
    int numDistinct = domains[col].size();
    if (numNumeric > 0.95*numRow)
      {
      if (numFractional > 0.01*numRow)
        {
        types[col] = CONTINUOUS;
        }
      else if (numDistinct < 0.9*numRow)
        {
        types[col] = INTEGER_CATEGORY;
        }
      else
        {
        types[col] = INTEGER_DATA;
        }
      }
    else
      {
      if (numDistinct < 0.9*numRow)
        {
        types[col] = STRING_CATEGORY;
        }
      else
        {
        types[col] = STRING_DATA;
        }
      }
    }
  return types;
}

std::vector<std::set<std::string> > ovViewQuickItem::columnDomains(vtkTable *table)
{
  vtkIdType numRow = table->GetNumberOfRows();
  vtkIdType numCol = table->GetNumberOfColumns();
  std::vector<std::set<std::string> > domains(numCol);
  for (vtkIdType col = 0; col < numCol; ++col)
    {
    std::set<std::string> distinct;
    for (vtkIdType row = 0; row < numRow; ++row)
      {
      std::string stringVal = table->GetValue(row, col).ToString();
      distinct.insert(stringVal);
      }
    domains[col] = distinct;
    }
  return domains;
}

void ovViewQuickItem::convertTableColumns(vtkTable *table, const std::vector<int> &types)
{
  // Convert columns using types
  vtkIdType numRow = table->GetNumberOfRows();
  vtkIdType numCol = table->GetNumberOfColumns();
  vtkNew<vtkTable> out;
  for (vtkIdType col = 0; col < numCol; ++col)
    {
    vtkAbstractArray *arr = table->GetColumn(col);
    int type = types[col];
    if (type == CONTINUOUS)
      {
      vtkNew<vtkDoubleArray> darr;
      darr->SetName(arr->GetName());
      darr->SetNumberOfTuples(numRow);
      for (vtkIdType row = 0; row < numRow; ++row)
        {
        darr->SetValue(row, arr->GetVariantValue(row).ToDouble());
        }
      out->AddColumn(darr.GetPointer());
      }
    else if (type == INTEGER_CATEGORY || type == INTEGER_DATA)
      {
      vtkNew<vtkIntArray> iarr;
      iarr->SetName(arr->GetName());
      iarr->SetNumberOfTuples(numRow);
      for (vtkIdType row = 0; row < numRow; ++row)
        {
        iarr->SetValue(row, arr->GetVariantValue(row).ToInt());
        }
      out->AddColumn(iarr.GetPointer());
      }
    else // if (type == STRING_CATEGORY || type == STRING_DATA)
      {
      out->AddColumn(arr);
      }
    }

  // Copy converted columns into original table
  table->ShallowCopy(out.GetPointer());
}

std::vector<std::vector<int> > ovViewQuickItem::columnRelations(vtkTable *table, const std::vector<std::set<std::string> > &domains, const std::vector<int> &types)
{
  vtkIdType numRow = table->GetNumberOfRows();
  vtkIdType numCol = table->GetNumberOfColumns();
  std::vector<std::vector<int> > relations(numCol, std::vector<int>(numCol));
  for (vtkIdType col1 = 0; col1 < numCol; ++col1)
    {
    for (vtkIdType col2 = col1+1; col2 < numCol; ++col2)
      {
      int col1BasicType = basicType(types[col1]);
      int col2BasicType = basicType(types[col2]);
      if (col1BasicType != col2BasicType
          || col1BasicType == 2)
        {
        relations[col1][col2] = UNRELATED;
        break;
        }
      std::set<std::string> isect;
      std::set_intersection(
        domains[col1].begin(), domains[col1].end(),
        domains[col2].begin(), domains[col2].end(),
        std::inserter(isect, isect.begin()));
      int numShared = isect.size();
      if (numShared > 0.01*numRow)
        {
        relations[col1][col2] = SHARED_DOMAIN;
        }
      else
        {
        relations[col1][col2] = UNRELATED;
        }
      }
    }
  return relations;
}

void ovViewQuickItem::setTable(vtkTable *table)
{
  // Look for relationships between columns
  std::vector<std::set<std::string> > domains = columnDomains(table);
  std::vector<int> types = columnTypes(table, domains);
  convertTableColumns(table, types);
  std::vector<std::vector<int> > relations = columnRelations(table, domains, types);

  this->m_table = table;
  this->setupView();
}

void ovViewQuickItem::setViewType(QString &viewType)
{
  this->m_viewLock.lock();
  if (this->m_viewType != viewType)
    {
    this->m_viewType = viewType;
    this->setupView();
    }
  this->m_viewLock.unlock();
}

void ovViewQuickItem::setupView()
{
  this->m_view->GetScene()->ClearItems();
  this->m_views[this->m_viewType]->setTable(this->m_table.GetPointer(), this->m_view.GetPointer());
}

void ovViewQuickItem::animate()
{
  update();
}
