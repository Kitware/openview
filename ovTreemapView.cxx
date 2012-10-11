/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovTreemapView.h"

#include "ovTreemapItem.h"
#include "ovViewQuickItem.h"

#include "vtkAbstractArray.h"
#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkDoubleArray.h"
#include "vtkExtractSelectedGraph.h"
#include "vtkGraph.h"
#include "vtkIncrementalForceLayout.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTableToTreeFilter.h"

ovTreemapView::ovTreemapView(QObject *parent) : ovView(parent)
{
  m_table = vtkSmartPointer<vtkTable>::New();
}

ovTreemapView::~ovTreemapView()
{
}

void ovTreemapView::setTable(vtkTable *table, vtkContextView *view)
{
  cerr << "setTable" << endl;
  if (table != this->m_table.GetPointer())
    {
    std::vector<std::set<std::string> > domains = ovViewQuickItem::columnDomains(table);
    std::vector<int> types = ovViewQuickItem::columnTypes(table, domains);

    vtkIdType numCol = table->GetNumberOfColumns();

    QString level1 = "";
    QString level2 = "";

    for (vtkIdType col = 0; col < numCol; ++col)
      {
      if (level1 == "" && types[col] == INTEGER_CATEGORY || types[col] == STRING_CATEGORY)
        {
        level1 = table->GetColumnName(col);
        }
      else if (level2 == "" && types[col] == INTEGER_CATEGORY || types[col] == STRING_CATEGORY)
        {
        level2 = table->GetColumnName(col);
        }
      else if (level1 == "" && types[col] == STRING_DATA)
        {
        level1 = table->GetColumnName(col);
        }
      else if (level2 == "" && types[col] == STRING_DATA)
        {
        level2 = table->GetColumnName(col);
        }
      }

    cerr << "level1: " << level1.toStdString() << endl;
    cerr << "level2: " << level2.toStdString() << endl;

    this->m_level1 = level1;
    this->m_level2 = level2;
    this->m_color = "count";
    this->m_size = "count";

    this->m_table = table;
    this->m_table->Dump();

    this->generateTreemap();
    }

  view->GetScene()->AddItem(this->m_item.GetPointer());
}

void ovTreemapView::generateTreemap()
{
  vtkNew<vtkTableToTreeFilter> ttt;
  ttt->SetInputData(m_table);

  m_item->SetTree(ttt->GetOutput());
}

QString ovTreemapView::name()
{
  return "TREEMAP";
}

QStringList ovTreemapView::attributes()
{
  return QStringList() << "Level 1" << "Level 2" << "Color" << "Size";
}

QStringList ovTreemapView::attributeOptions(QString attribute)
{
  cerr << attribute.toStdString() << endl;
  if (attribute == "Level 1" || attribute == "Level 2")
    {
    QStringList fields;
    for (vtkIdType col = 0; col < this->m_table->GetNumberOfColumns(); ++col)
      {
      if (vtkStringArray::SafeDownCast(this->m_table->GetColumn(col)))
        {
        fields << this->m_table->GetColumn(col)->GetName();
        }
      }
    return fields;
    }
  if (attribute == "Color" || attribute == "Size")
    {
    QStringList fields;
    fields << "count";
    for (vtkIdType col = 0; col < this->m_table->GetNumberOfColumns(); ++col)
      {
      if (vtkDataArray::SafeDownCast(this->m_table->GetColumn(col)))
        {
        fields << this->m_table->GetColumn(col)->GetName();
        }
      }
    return fields;
    }
  return QStringList();
}

void ovTreemapView::setAttribute(QString attribute, QString value)
{
  if (attribute == "Level 1")
    {
    m_level1 = value;
    generateTreemap();
    return;
    }
  if (attribute == "Level 2")
    {
    m_level2 = value;
    generateTreemap();
    return;
    }
  if (attribute == "Color")
    {
    m_color = value;
    generateTreemap();
    return;
    }
  if (attribute == "Size")
    {
    m_size = value;
    generateTreemap();
    return;
    }
}

QString ovTreemapView::getAttribute(QString attribute)
{
  if (attribute == "Level 1")
    {
    return this->m_level1;
    }
  if (attribute == "Level 2")
    {
    return this->m_level2;
    }
  if (attribute == "Color")
    {
    return this->m_color;
    }
  if (attribute == "Size")
    {
    return this->m_size;
    }
  return QString();
}
