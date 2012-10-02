#include "ovGraphView.h"

#include "ovGraphItem.h"
#include "ovViewQuickItem.h"

#include "vtkAbstractArray.h"
#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkGraph.h"
#include "vtkIncrementalForceLayout.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkTable.h"
#include "vtkTableToGraph.h"

ovGraphView::ovGraphView(QObject *parent) : ovView(parent)
{
  m_animate = true;
  m_sharedDomain = false;
  m_table = vtkSmartPointer<vtkTable>::New();
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
    ttg->AddLinkVertex(m_source.toAscii(), "domain");
    ttg->AddLinkVertex(m_target.toAscii(), "domain");
    }
  else
    {
    ttg->AddLinkVertex(m_source.toAscii(), "source");
    ttg->AddLinkVertex(m_target.toAscii(), "target");
    }
  ttg->AddLinkEdge(m_source.toAscii(), m_target.toAscii());
  ttg->Update();

  vtkNew<vtkPoints> points;
  vtkIdType numVert = ttg->GetOutput()->GetNumberOfVertices();
  for (vtkIdType i = 0; i < numVert; ++i)
    {
    double angle = vtkMath::RadiansFromDegrees(360.0*i/numVert);
    points->InsertNextPoint(200*cos(angle) + 200, 200*sin(angle) + 200, 0.0);
    }
  ttg->GetOutput()->SetPoints(points.GetPointer());
  m_item->SetGraph(ttg->GetOutput());
  m_item->GetLayout()->SetStrength(1);
  m_item->GetLayout()->SetAlpha(0.1f);
}

QString ovGraphView::name()
{
  return "GRAPH";
}

QStringList ovGraphView::attributes()
{
  return QStringList() << "Source" << "Target" << "Color" << "Label" << "Animate";
}

QStringList ovGraphView::attributeOptions(QString attribute)
{
  if (attribute == "Source" || attribute == "Target" || attribute == "Color" || attribute == "Label")
    {
    QStringList fields;
    for (vtkIdType col = 0; col < this->m_table->GetNumberOfColumns(); ++col)
      {
      fields << this->m_table->GetColumn(col)->GetName();
      }
    if (attribute == "Color" || attribute == "Label")
      {
      fields << "domain" << "label";
      }
    return fields;
    }
  if (attribute == "Animate")
    {
    return QStringList() << "on" << "off";
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
    }
  if (attribute == "Color")
    {
    m_color = value;
    return;
    }
  if (attribute == "Label")
    {
    m_label = value;
    return;
    }
  if (attribute == "Animate")
    {
    m_animate = (value == "on");
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
    return this->m_color;
    }
  if (attribute == "Label")
    {
    return this->m_label;
    }
  if (attribute == "Animate")
    {
    return this->m_animate ? "on" : "off";
    }
}

void ovGraphView::prepareForRender()
{
  if (this->m_animate)
    {
    this->m_item->UpdateLayout();
    }
}
