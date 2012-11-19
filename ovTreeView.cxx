/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovTreeView.h"

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
#include "vtkTreeHeatmapItem.h"
#include "vtkAreaLayout.h"
#include "vtkStackedTreeLayoutStrategy.h"

ovTreeView::ovTreeView(QObject *parent) : ovView(parent)
{
  m_table = vtkSmartPointer<vtkTable>::New();
  m_tree = vtkSmartPointer<vtkTree>::New();
}

ovTreeView::~ovTreeView()
{
}

void ovTreeView::setTable(vtkTable *table, vtkContextView *view)
{
  if (table != this->m_table.GetPointer())
    {
    std::vector<std::set<std::string> > domains = ovViewQuickItem::columnDomains(table);
    std::vector<int> types = ovViewQuickItem::columnTypes(table, domains);

    vtkIdType numCol = table->GetNumberOfColumns();

    QString level1 = "";
    QString level2 = "";
    QString depth = "tree level";
    QString label = "(none)";

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
      else if (label == "" && types[col] == STRING_CATEGORY)
        {
        label = table->GetColumnName(col);
        }
      else if (level1 == "" && types[col] == STRING_DATA)
        {
        level1 = table->GetColumnName(col);
        }
      else if (level2 == "" && types[col] == STRING_DATA)
        {
        level2 = table->GetColumnName(col);
        }
      else if (label == "" && types[col] == STRING_DATA)
        {
        label = table->GetColumnName(col);
        }
      else if (depth == "" && types[col] == CONTINUOUS)
        {
        depth = table->GetColumnName(col);
        }
      else if (depth == "" && types[col] == INTEGER_DATA)
        {
        depth = table->GetColumnName(col);
        }
      else if (depth == "" && types[col] == INTEGER_CATEGORY)
        {
        depth = table->GetColumnName(col);
        }
      }

    if (level2 == "")
      {
      level2 = level1;
      }
    if (label == "")
      {
      label = level2;
      }

    this->m_level1 = level1;
    this->m_level2 = level2;
    this->m_label = label;
    this->m_depth = depth;

    this->m_table = table;

    this->generateTree();
    }

  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  view->GetScene()->AddItem(trans.GetPointer());

  trans->AddItem(this->m_item.GetPointer());
}

void ovTreeView::setTree(vtkTree *tree, vtkContextView *view)
{
  if (tree != this->m_tree.GetPointer())
    {
    QString level1 = "";
    QString level2 = "";
    QString depth = "tree level";
    QString label = "(none)";

    this->m_level1 = level1;
    this->m_level2 = level2;
    this->m_label = label;
    this->m_depth = depth;

    this->m_tree = tree;

    this->generateView();
    }

  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  view->GetScene()->AddItem(trans.GetPointer());

  trans->AddItem(this->m_item.GetPointer());
}


void ovTreeView::generateTree()
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
  group2->Update();

  this->m_tree = group2->GetOutput();
}

void ovTreeView::generateView()
{
  vtkNew<vtkTreeFieldAggregator> agg;
  agg->SetInputData(this->m_tree);
  agg->SetLeafVertexUnitSize(false);
  agg->SetField(m_depth.toAscii());
  agg->Update();

  this->m_item->SetTree(agg->GetOutput());
}

QString ovTreeView::name()
{
  return "TREE";
}

QStringList ovTreeView::attributes()
{
  return QStringList() << "Level 1" << "Level 2" << "Label" << "Depth";
}

QStringList ovTreeView::attributeOptions(QString attribute)
{
  if (attribute == "Label")
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
  if (attribute == "Depth")
    {
    QStringList fields;
    fields << "tree level";
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

void ovTreeView::setAttribute(QString attribute, QString value)
{
  if (attribute == "Label")
    {
    m_label = value;
    generateView();
    return;
    }
  if (attribute == "Level 1")
    {
    m_level1 = value;
    generateTree();
    return;
    }
  if (attribute == "Level 2")
    {
    m_level2 = value;
    generateTree();
    return;
    }
  if (attribute == "Depth")
    {
    m_depth = value;
    generateView();
    return;
    }
}

QString ovTreeView::getAttribute(QString attribute)
{
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
  if (attribute == "Depth")
    {
    return this->m_depth;
    }
  return QString();
}
