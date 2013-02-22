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

bool ovTreemapView::acceptsType(const QString &type)
{
  return (type == "vtkTable");
}

void ovTreemapView::setData(vtkDataObject *data, vtkContextView *view)
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

    vtkIdType numCol = table->GetNumberOfColumns();

    QString group = "";
    QString hover = "";

    for (vtkIdType col = 0; col < numCol; ++col)
      {
      if (group == "" && types[col] == STRING_CATEGORY)
        {
        group = table->GetColumnName(col);
        }
      else if (hover == "" && types[col] == STRING_DATA)
        {
        hover = table->GetColumnName(col);
        }
      else if (group == "" && types[col] == STRING_DATA)
        {
        group = table->GetColumnName(col);
        }
      }

    if (hover == "")
      {
      hover = group;
      }

    this->m_group = group;
    this->m_hover = hover;
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
  vtkAbstractArray *groupArr = m_table->GetColumnByName(m_group.toUtf8().data());
  vtkAbstractArray *hoverArr = m_table->GetColumnByName(m_hover.toUtf8().data());
  for (vtkIdType i = 0; i < ttt->GetOutput()->GetNumberOfVertices(); ++i)
    {
    pedigree->SetValue(i, i);
    name->SetValue(i, hoverArr ? hoverArr->GetVariantValue(i).ToString() : "");
    }
  ttt->GetOutput()->GetVertexData()->SetPedigreeIds(pedigree.GetPointer());
  ttt->GetOutput()->GetVertexData()->AddArray(name.GetPointer());

  vtkNew<vtkGroupLeafVertices> group;
  group->SetInputConnection(ttt->GetOutputPort());
  group->SetInputArrayToProcess(0, 0, 0, vtkDataObject::VERTEX, m_group.toUtf8().data());
  group->SetInputArrayToProcess(1, 0, 0, vtkDataObject::VERTEX, "name");

  vtkNew<vtkTreeFieldAggregator> agg;
  if (!groupArr)
    {
    agg->SetInputConnection(ttt->GetOutputPort());
    }
  else
    {
    agg->SetInputConnection(group->GetOutputPort());
    }
  agg->SetLeafVertexUnitSize(false);
  agg->SetField(m_size.toUtf8());

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
  layout->SetSizeArrayName(m_size.toUtf8());
  layout->Update();

  m_item->SetTree(layout->GetOutput());
  m_item->SetColorArray(m_color.toStdString());
  m_item->SetLabelArray("name");
  m_item->SetTooltipArray(m_hover.toStdString());
}

QString ovTreemapView::name()
{
  return "TREEMAP";
}

QStringList ovTreemapView::attributes()
{
  return QStringList() << "Group" << "Strategy" << "Hover" << "Color" << "Size";
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
  if (attribute == "Group")
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
  if (attribute == "Hover")
    {
    m_hover = value;
    generateTreemap();
    return;
    }
  if (attribute == "Strategy")
    {
    m_strategy = value;
    generateTreemap();
    return;
    }
  if (attribute == "Group")
    {
    m_group = value;
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
  if (attribute == "Hover")
    {
    return this->m_hover;
    }
  if (attribute == "Strategy")
    {
    return this->m_strategy;
    }
  if (attribute == "Group")
    {
    return this->m_group;
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
