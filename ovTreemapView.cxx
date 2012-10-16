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
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkExtractSelectedGraph.h"
#include "vtkGraph.h"
#include "vtkGroupLeafVertices.h"
#include "vtkIdTypeArray.h"
#include "vtkIncrementalForceLayout.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTableToTreeFilter.h"
#include "vtkTreeFieldAggregator.h"
#include "vtkTreeMapLayout.h"
#include "vtkSliceAndDiceLayoutStrategy.h"
#include "vtkSquarifyLayoutStrategy.h"

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
      if (level1 == "" && types[col] == STRING_CATEGORY)
        {
        level1 = table->GetColumnName(col);
        }
      else if (level2 == "" && types[col] == STRING_CATEGORY)
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
    this->m_color = "parent";
    this->m_size = "equal size";
    this->m_strategy = "squarify";

    this->m_table = table;
    //this->m_table->Dump();

    this->generateTreemap();
    }

  view->GetScene()->AddItem(this->m_item.GetPointer());
}

void ovTreemapView::generateTreemap()
{
  vtkNew<vtkTableToTreeFilter> ttt;
  ttt->SetInputData(m_table);
  ttt->Update();

  vtkNew<vtkIdTypeArray> pedigree;
  pedigree->SetName("pedigree");
  pedigree->SetNumberOfTuples(ttt->GetOutput()->GetNumberOfVertices());
  vtkNew<vtkStringArray> name;
  name->SetName("name");
  name->SetNumberOfTuples(ttt->GetOutput()->GetNumberOfVertices());
  for (vtkIdType i = 0; i < ttt->GetOutput()->GetNumberOfVertices(); ++i)
    {
    pedigree->SetValue(i, i);
    name->SetValue(i, "");
    }
  ttt->GetOutput()->GetVertexData()->SetPedigreeIds(pedigree.GetPointer());
  ttt->GetOutput()->GetVertexData()->AddArray(name.GetPointer());

  vtkNew<vtkGroupLeafVertices> group1;
  group1->SetInputConnection(ttt->GetOutputPort());
  group1->SetInputArrayToProcess(0, 0, 0, vtkDataObject::VERTEX, m_level1.toAscii().data());
  group1->SetInputArrayToProcess(1, 0, 0, vtkDataObject::VERTEX, "name");

  vtkNew<vtkGroupLeafVertices> group2;
  group2->SetInputConnection(group1->GetOutputPort());
  group2->SetInputArrayToProcess(0, 0, 0, vtkDataObject::VERTEX, m_level2.toAscii().data());
  group2->SetInputArrayToProcess(1, 0, 0, vtkDataObject::VERTEX, "name");

  vtkNew<vtkTreeFieldAggregator> agg;
  agg->SetInputConnection(group2->GetOutputPort());
  agg->SetLeafVertexUnitSize(false);
  agg->SetField(m_size.toAscii());

  vtkNew<vtkSquarifyLayoutStrategy> squarify;
  squarify->SetShrinkPercentage(0.0);

  vtkNew<vtkSliceAndDiceLayoutStrategy> slice;
  slice->SetShrinkPercentage(0.0);

  vtkNew<vtkTreeMapLayout> layout;
  layout->SetInputConnection(agg->GetOutputPort());
  if (m_strategy == "squarify")
    {
    layout->SetLayoutStrategy(squarify.GetPointer());
    }
  else
    {
    layout->SetLayoutStrategy(slice.GetPointer());
    }
  layout->SetSizeArrayName(m_size.toAscii());
  cerr << "layout begin" << endl;
  layout->Update();
  cerr << "layout end" << endl;

  cerr << "set tree begin" << endl;
  m_item->SetTree(layout->GetOutput());
  cerr << "set tree end" << endl;
  m_item->SetColorArray(m_color.toStdString());
  m_item->SetLabelArray("name");
}

QString ovTreemapView::name()
{
  return "TREEMAP";
}

QStringList ovTreemapView::attributes()
{
  return QStringList() << "Level 1" << "Level 2" << "Strategy" << "Hover" << "Color" << "Size";
}

QStringList ovTreemapView::attributeOptions(QString attribute)
{
  if (attribute == "Strategy")
    {
    return QStringList() << "squarify" << "slice and dice";
    }
  if (attribute == "Hover")
    {
    QStringList fields;
    for (vtkIdType col = 0; col < this->m_table->GetNumberOfColumns(); ++col)
      {
      fields << this->m_table->GetColumn(col)->GetName();
      }
    return fields;
    }
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
    if (attribute == "Color")
      {
      fields << "parent";
      }
    if (attribute == "Size")
      {
      fields << "equal size";
      }
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
  if (attribute == "Strategy")
    {
    m_strategy = value;
    generateTreemap();
    return;
    }
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
