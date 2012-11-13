/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovTreeringView.h"

#include "ovTreeringItem.h"
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
#include "vtkAreaLayout.h"
#include "vtkStackedTreeLayoutStrategy.h"

ovTreeringView::ovTreeringView(QObject *parent) : ovView(parent)
{
  m_table = vtkSmartPointer<vtkTable>::New();
}

ovTreeringView::~ovTreeringView()
{
}

void ovTreeringView::setTable(vtkTable *table, vtkContextView *view)
{
  if (table != this->m_table.GetPointer())
    {
    std::vector<std::set<std::string> > domains = ovViewQuickItem::columnDomains(table);
    std::vector<int> types = ovViewQuickItem::columnTypes(table, domains);

    vtkIdType numCol = table->GetNumberOfColumns();

    QString level1 = "";
    QString level2 = "";
    QString hover = "";

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
      else if (hover == "" && types[col] == STRING_DATA)
        {
        hover = table->GetColumnName(col);
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

    if (level2 == "")
      {
      level2 = level1;
      }
    if (hover == "")
      {
      hover = level2;
      }

    this->m_level1 = level1;
    this->m_level2 = level2;
    this->m_hover = hover;
    this->m_color = "parent";
    this->m_size = "equal size";

    this->m_table = table;
    //this->m_table->Dump();

    this->generateTreering();
    }

  view->GetScene()->AddItem(this->m_item.GetPointer());
}

void ovTreeringView::generateTreering()
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

  vtkNew<vtkTreeFieldAggregator> agg2;
  agg2->SetInputConnection(agg->GetOutputPort());
  agg2->SetLeafVertexUnitSize(false);
  agg2->SetField(m_color.toAscii());

  vtkNew<vtkStackedTreeLayoutStrategy> stacked;
  stacked->SetShrinkPercentage(0.0);
  stacked->SetRootEndAngle(1.0);
  stacked->SetReverse(true);

  vtkNew<vtkAreaLayout> layout;
  layout->SetInputConnection(m_size != m_color ? agg2->GetOutputPort() : agg->GetOutputPort());
  layout->SetLayoutStrategy(stacked.GetPointer());
  layout->SetSizeArrayName(m_size.toAscii());
  layout->Update();

  m_item->SetTree(layout->GetOutput());
  m_item->SetColorArray(m_color.toStdString());
  m_item->SetGroupNameArray("name");
  m_item->SetLabelArray(m_label.toStdString());
  m_item->SetTooltipArray(m_hover.toStdString());
}

QString ovTreeringView::name()
{
  return "TREERING";
}

QStringList ovTreeringView::attributes()
{
  return QStringList() << "Level 1" << "Level 2" << "Hover" << "Label" << "Color" << "Size";
}

QStringList ovTreeringView::attributeOptions(QString attribute)
{
  if (attribute == "Hover" || attribute == "Label")
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

void ovTreeringView::setAttribute(QString attribute, QString value)
{
  if (attribute == "Hover")
    {
    m_hover = value;
    generateTreering();
    return;
    }
  if (attribute == "Hover")
    {
    m_label = value;
    generateTreering();
    return;
    }
  if (attribute == "Level 1")
    {
    m_level1 = value;
    generateTreering();
    return;
    }
  if (attribute == "Level 2")
    {
    m_level2 = value;
    generateTreering();
    return;
    }
  if (attribute == "Color")
    {
    m_color = value;
    generateTreering();
    return;
    }
  if (attribute == "Size")
    {
    m_size = value;
    generateTreering();
    return;
    }
}

QString ovTreeringView::getAttribute(QString attribute)
{
  if (attribute == "Hover")
    {
    return this->m_hover;
    }
  if (attribute == "Label")
    {
    return this->m_label;
    }
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
