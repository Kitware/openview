/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovGraphView.h"

#include "ovGraphItem.h"
#include "ovViewQuickItem.h"

#include "vtkAbstractArray.h"
#include "vtkBoostDividedEdgeBundling.h"
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
#include "vtkTable.h"
#include "vtkTableToGraph.h"
#include "vtkVertexDegree.h"

ovGraphView::ovGraphView(QObject *parent) : ovView(parent)
{
  m_animate = true;
  m_sharedDomain = false;
  m_table = vtkSmartPointer<vtkTable>::New();
  m_filter = false;
  m_bundle = false;
}

ovGraphView::~ovGraphView()
{
}

void ovGraphView::setTable(vtkTable *table, vtkContextView *view)
{
  if (table != this->m_table.GetPointer())
    {
    std::vector<std::set<std::string> > domains = ovViewQuickItem::columnDomains(table);
    std::vector<int> types = ovViewQuickItem::columnTypes(table, domains);
    std::vector<std::vector<int> > relations = ovViewQuickItem::columnRelations(table, domains, types);

    vtkIdType numCol = table->GetNumberOfColumns();

    QString source = "";
    QString target = "";

    double bestScore = 0;

    // Find best pair of columns for source/target
    m_sharedDomain = false;
    for (vtkIdType col1 = 0; col1 < numCol; ++col1)
      {
      for (vtkIdType col2 = col1+1; col2 < numCol; ++col2)
        {
        int type1 = types[col1];
        int type2 = types[col2];
        int rel = relations[col1][col2];
        bool category1 = (type1 == STRING_CATEGORY || type1 == INTEGER_CATEGORY);
        bool category2 = (type2 == STRING_CATEGORY || type2 == INTEGER_CATEGORY);
        if (rel == SHARED_DOMAIN)
          {
          if (bestScore < 10 && type1 == STRING_CATEGORY)
            {
            bestScore = 10;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            m_sharedDomain = true;
            }
          else if (bestScore < 8)
            {
            bestScore = 8;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            m_sharedDomain = true;
            }
          }
        else
          {
          if (bestScore < 7
              && type1 == STRING_CATEGORY
              && type2 == STRING_CATEGORY)
            {
            bestScore = 7;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          else if (bestScore < 6
              && type1 == INTEGER_CATEGORY
              && type2 == INTEGER_CATEGORY)
            {
            bestScore = 6;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          else if (bestScore < 5 && (category1 && category2))
            {
            bestScore = 5;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          else if (bestScore < 4 && (type1 == STRING_CATEGORY || type2 == STRING_CATEGORY))
            {
            bestScore = 4;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          else if (bestScore < 3 && (type1 == INTEGER_CATEGORY || type2 == INTEGER_CATEGORY))
            {
            bestScore = 3;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          else if (bestScore < 2 && (type1 != CONTINUOUS || type2 != CONTINUOUS))
            {
            bestScore = 2;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          else if (bestScore < 1)
            {
            bestScore = 1;
            source = table->GetColumn(col1)->GetName();
            target = table->GetColumn(col2)->GetName();
            }
          }
        }
      }

    // No source/target fields
    if (source == "" && target == "")
      {
      return;
      }

    //std::cerr << "GRAPH chose " << source << ", " << target << " with score " << bestScore << std::endl;

    this->m_source = source;
    this->m_target = target;
    this->m_item->SetColorArray("domain");
    this->m_item->SetTooltipArray("label");
    this->m_item->SetLabelArray("label");

    this->m_table = table;

    this->generateGraph();
    }

  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  view->GetScene()->AddItem(trans.GetPointer());

  trans->AddItem(this->m_item.GetPointer());
}

void ovGraphView::generateGraph()
{
  vtkNew<vtkTableToGraph> ttg;
  ttg->SetInputData(m_table);
  if (m_sharedDomain)
    {
    QString combined = m_source + " / " + m_target;
    ttg->AddLinkVertex(m_source.toAscii(), combined.toAscii());
    ttg->AddLinkVertex(m_target.toAscii(), combined.toAscii());
    }
  else
    {
    ttg->AddLinkVertex(m_source.toAscii(), m_source.toAscii());
    ttg->AddLinkVertex(m_target.toAscii(), m_target.toAscii());
    }
  ttg->AddLinkEdge(m_source.toAscii(), m_target.toAscii());
  ttg->SetDirected(true);

  vtkNew<vtkVertexDegree> degree;
  degree->SetInputConnection(ttg->GetOutputPort());
  degree->SetOutputArrayName("connections");

  degree->Update();
  vtkSmartPointer<vtkGraph> graph = degree->GetOutput();

  if (m_filter)
    {
    for (int i = 0; i < 2; ++i)
      {
      vtkNew<vtkSelection> sel;
      vtkNew<vtkSelectionNode> selNode;
      selNode->SetContentType(vtkSelectionNode::THRESHOLDS);
      selNode->SetFieldType(vtkSelectionNode::VERTEX);
      vtkNew<vtkDoubleArray> threshold;
      threshold->SetNumberOfComponents(2);
      threshold->SetName("connections");
      threshold->InsertNextValue(1.5);
      threshold->InsertNextValue(VTK_DOUBLE_MAX);
      selNode->SetSelectionList(threshold.GetPointer());
      sel->AddNode(selNode.GetPointer());
      vtkNew<vtkExtractSelectedGraph> extract;
      extract->SetInputData(0, graph);
      extract->SetInputData(1, sel.GetPointer());

      vtkNew<vtkVertexDegree> deg;
      deg->SetInputConnection(extract->GetOutputPort());
      deg->SetOutputArrayName("connections");

      deg->Update();
      graph->ShallowCopy(deg->GetOutput());
      }
    }

  vtkNew<vtkPoints> points;
  vtkIdType numVert = graph->GetNumberOfVertices();
  for (vtkIdType i = 0; i < numVert; ++i)
    {
    double angle = vtkMath::RadiansFromDegrees(360.0*i/numVert);
    points->InsertNextPoint(200*cos(angle) + 200, 200*sin(angle) + 200, 0.0);
    }
  graph->SetPoints(points.GetPointer());
  m_item->SetGraph(graph);
  //m_item->GetLayout()->SetStrength(1.0f);
  //m_item->GetLayout()->SetDistance(50.0f);
  //m_item->GetLayout()->SetCharge(-5.0f);
  m_item->GetLayout()->SetAlpha(0.1f);

  if (m_bundle)
    {
    m_item->GetLayout()->SetAlpha(0.3f);
    for (int i = 0; i < 100; ++i)
      {
      m_item->UpdateLayout();
      }
    vtkNew<vtkBoostDividedEdgeBundling> bundle;
    bundle->SetInputData(graph);
    bundle->Update();
    m_item->SetGraph(bundle->GetOutput());
    }
}

QString ovGraphView::name()
{
  return "GRAPH";
}

QStringList ovGraphView::attributes()
{
  return QStringList() << "Source" << "Target" << "Color" << "Label" << "Hover" << "Animate" << "Filter" << "Bundle";
}

QStringList ovGraphView::attributeOptions(QString attribute)
{
  if (attribute == "Source" || attribute == "Target")
    {
    QStringList fields;
    for (vtkIdType col = 0; col < this->m_table->GetNumberOfColumns(); ++col)
      {
      fields << this->m_table->GetColumn(col)->GetName();
      }
    return fields;
    }
  if (attribute == "Color" || attribute == "Label" || attribute == "Hover")
    {
    return QStringList() << "(none)" << "domain" << "label" << "connections";
    }
  if (attribute == "Animate" || attribute == "Bundle")
    {
    return QStringList() << "on" << "off";
    }
  if (attribute == "Filter")
    {
    return QStringList() << "all" << "2+ connections";
    }
  return QStringList();
}

void ovGraphView::setAttribute(QString attribute, QString value)
{
  if (attribute == "Source")
    {
    m_source = value;
    generateGraph();
    return;
    }
  if (attribute == "Target")
    {
    m_target = value;
    generateGraph();
    return;
    }
  if (attribute == "Color")
    {
    m_item->SetColorArray(value.toStdString());
    return;
    }
  if (attribute == "Label")
    {
    m_item->SetLabelArray(value.toStdString());
    return;
    }
  if (attribute == "Hover")
    {
    m_item->SetTooltipArray(value.toStdString());
    return;
    }
  if (attribute == "Animate")
    {
    m_animate = (value == "on");
    return;
    }
  if (attribute == "Bundle")
    {
    m_bundle = (value == "on");
    generateGraph();
    return;
    }
  if (attribute == "Filter")
    {
    m_filter = (value != "all");
    generateGraph();
    return;
    }
}

QString ovGraphView::getAttribute(QString attribute)
{
  if (attribute == "Source")
    {
    return this->m_source;
    }
  if (attribute == "Target")
    {
    return this->m_target;
    }
  if (attribute == "Color")
    {
    return QString::fromStdString(this->m_item->GetColorArray());
    }
  if (attribute == "Label")
    {
    return QString::fromStdString(this->m_item->GetLabelArray());
    }
  if (attribute == "Hover")
    {
    return QString::fromStdString(this->m_item->GetTooltipArray());
    }
  if (attribute == "Animate")
    {
    return this->m_animate ? "on" : "off";
    }
  if (attribute == "Bundle")
    {
    return this->m_bundle ? "on" : "off";
    }
  if (attribute == "Filter")
    {
    return this->m_filter ? "2+ connections" : "all";
    }
  return QString();
}

void ovGraphView::prepareForRender()
{
  if (this->m_animate && !this->m_bundle)
    {
    this->m_item->UpdateLayout();
    }
}
