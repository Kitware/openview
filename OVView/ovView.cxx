#include "ovView.h"

#include "ovContextInteractorStyle.h"
#include "ovGraphItem.h"

#include "vtkAxis.h"
#include "vtkChartXY.h"
#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkDataSetAttributes.h"
#include "vtkDelimitedTextReader.h"
#include "vtkDoubleArray.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkGraphItem.h"
#include "vtkIncrementalForceLayout.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkPlot.h"
#include "vtkPlotPoints.h"
#include "vtkPoints.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTableToGraph.h"

#include <QOpenGLContext>
#include <QQuickCanvas>
#include <QThread>

#include <set>
#include <algorithm>

ovView::ovView()
{
}

ovView::~ovView()
{
}

enum {
  CONTINUOUS,
  INTEGER_CATEGORY,
  INTEGER_DATA,
  STRING_CATEGORY,
  STRING_DATA
};

enum {
  UNRELATED,
  SHARED_DOMAIN
};

void ovView::init()
{
  this->View->SetRenderWindow(this->GetRenderWindow());
  vtkNew<ovContextInteractorStyle> style;
  style->SetScene(this->View->GetScene());
  this->View->GetInteractor()->SetInteractorStyle(style.GetPointer());
  this->ViewType = "GRAPH";
  //QUrl url("file:///code/opendemo/data/domains.csv");
  //QUrl url("file:///code/opendemo/data/classes.csv");
  //QUrl url("file:///code/opendemo/data/kcore_edges.csv");
  //this->setUrl(url);
  connect(&AnimationTimer, SIGNAL(timeout()), this, SLOT(animateGraph()), Qt::DirectConnection);
  this->AnimationTimer.setInterval(1000/60);
  this->AnimationTimer.moveToThread(this->canvas()->openglContext()->thread());
  this->AnimationTimer.start();
}

void ovView::prepareForRender()
{
  if (this->ViewType == "GRAPH")
    {
    graphItem->UpdateLayout();
    }
}

void ovView::setUrl(QUrl &url)
{
  this->Url = url;
  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(url.toLocalFile().toLatin1().data());
  reader->SetHaveHeaders(true);
  //reader->SetFieldDelimiterCharacters("\t");
  reader->Update();
  vtkTable *table = reader->GetOutput();

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
  this->setTable(table);
}

QStringList ovView::dataFields(QString attribute)
{
  QStringList fields;
  for (vtkIdType col = 0; col < this->Table->GetNumberOfColumns(); ++col)
    {
    fields << this->Table->GetColumn(col)->GetName();
    }
  if (this->ViewType == "GRAPH")
    {
    fields << "domain" << "label";
    }
  return fields;
}

QStringList ovView::viewAttributes()
{
  QStringList attributes;
  if (this->ViewType == "GRAPH")
    {
    attributes << "source" << "target";
    }
  else if (this->ViewType == "SCATTER")
    {
    attributes << "x" << "y";
    }
  return attributes;
}

void ovView::setAttribute(QString attribute, QString value)
{
  this->Attributes[this->ViewType][attribute] = value;
}

QString ovView::getAttribute(QString attribute)
{
  return this->Attributes[this->ViewType][attribute];
}

int ovView::basicType(int type)
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

void ovView::setTable(vtkTable *table)
{
  vtkIdType numRow = table->GetNumberOfRows();
  vtkIdType numCol = table->GetNumberOfColumns();
  this->Types = std::vector<int>(numCol);
  std::vector<std::set<std::string> > domains(numCol);
  for (vtkIdType col = 0; col < numCol; ++col)
    {
    vtkIdType numFractional = 0;
    vtkIdType numNumeric = 0;
    std::set<std::string> distinct;
    for (vtkIdType row = 0; row < numRow; ++row)
      {
      std::string stringVal = table->GetValue(row, col).ToString();
      distinct.insert(stringVal);
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
    int numDistinct = distinct.size();
    if (numNumeric > 0.95*numRow)
      {
      if (numFractional > 0.01*numRow)
        {
        this->Types[col] = CONTINUOUS;
        }
      else if (numDistinct < 0.9*numRow)
        {
        this->Types[col] = INTEGER_CATEGORY;
        }
      else
        {
        this->Types[col] = INTEGER_DATA;
        }
      }
    else
      {
      if (numDistinct < 0.9*numRow)
        {
        this->Types[col] = STRING_CATEGORY;
        }
      else
        {
        this->Types[col] = STRING_DATA;
        }
      }
    domains[col] = distinct;
    }

  // Convert columns using types
  this->Table->Initialize();
  for (vtkIdType col = 0; col < numCol; ++col)
    {
    vtkAbstractArray *arr = table->GetColumn(col);
    int type = this->Types[col];
    if (type == CONTINUOUS)
      {
      vtkNew<vtkDoubleArray> darr;
      darr->SetName(arr->GetName());
      darr->SetNumberOfTuples(numRow);
      for (vtkIdType row = 0; row < numRow; ++row)
        {
        darr->SetValue(row, arr->GetVariantValue(row).ToDouble());
        }
      this->Table->AddColumn(darr.GetPointer());
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
      this->Table->AddColumn(iarr.GetPointer());
      }
    else // if (type == STRING_CATEGORY || type == STRING_DATA)
      {
      this->Table->AddColumn(arr);
      }
    }

  // Look for relationships between columns
  this->Relationships = std::vector<std::vector<int> >(
      numCol, std::vector<int>(numCol));
  for (vtkIdType col1 = 0; col1 < numCol; ++col1)
    {
    for (vtkIdType col2 = col1+1; col2 < numCol; ++col2)
      {
      int col1BasicType = this->basicType(this->Types[col1]);
      int col2BasicType = this->basicType(this->Types[col2]);
      if (col1BasicType != col2BasicType
          || col1BasicType == 2)
        {
        this->Relationships[col1][col2] = UNRELATED;
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
        this->Relationships[col1][col2] = SHARED_DOMAIN;
        }
      else
        {
        this->Relationships[col1][col2] = UNRELATED;
        }
      }
    }
  this->setupView();
}

void ovView::setViewType(QString &viewType)
{
  if (this->ViewType != viewType)
    {
    this->ViewType = viewType;
    this->setupView();
    }
}

void ovView::setupView()
{
  if (!canvas() || !canvas()->openglContext() || QThread::currentThread() != this->canvas()->openglContext()->thread())
    {
    this->ViewLock.lock();
    }
  this->View->GetScene()->ClearItems();
  this->AnimationTimer.stop();

  if (this->ViewType == "SCATTER")
    {
    setupScatter();
    }
  else if (this->ViewType == "GRAPH")
    {
    setupGraph();
    }
  if (!canvas() || !canvas()->openglContext() || QThread::currentThread() != this->canvas()->openglContext()->thread())
    {
    this->ViewLock.unlock();
    }
}

void ovView::setupScatter()
{
  // Find best pair of columns for x/y
  vtkIdType numCol = this->Table->GetNumberOfColumns();
  vtkIdType x = -1;
  vtkIdType y = -1;
  double bestScore = 0;
  for (vtkIdType col1 = 0; col1 < numCol; ++col1)
    {
    for (vtkIdType col2 = col1+1; col2 < numCol; ++col2)
      {
      int type1 = this->Types[col1];
      int type2 = this->Types[col2];
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
  std::cerr << "SCATTER chose " << x << ", " << y << " with score " << bestScore << std::endl;

  if (x == -1 || y == -1)
    {
    return;
    }

  this->Attributes[this->ViewType]["x"] = this->Table->GetColumnName(x);
  this->Attributes[this->ViewType]["y"] = this->Table->GetColumnName(y);

  vtkNew<vtkChartXY> chart;
  this->View->GetScene()->AddItem(chart.GetPointer());
  chart->SetShowLegend(false);

  vtkPlot *points = chart->AddPlot(vtkChart::POINTS);
  points->SetInputData(this->Table.GetPointer(), x, y);
  points->SetColor(0, 0, 0, 255);
  points->SetWidth(1.0);
  //points->SetIndexedLabels(labels.GetPointer());
  points->SetTooltipLabelFormat("(%x, %y)");
  chart->GetAxis(vtkAxis::LEFT)->SetTitle(this->Table->GetColumnName(x));
  chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(this->Table->GetColumnName(y));
  vtkPlotPoints::SafeDownCast(points)->SetMarkerStyle(vtkPlotPoints::CIRCLE);
  vtkPlotPoints::SafeDownCast(points)->SetMarkerSize(10);
  points->SetColor(128, 128, 128, 255);
}

void ovView::animateGraph()
{
  update();
}

void ovView::setupGraph()
{
  vtkIdType numCol = this->Table->GetNumberOfColumns();
  std::string source = "";
  std::string target = "";

  double bestScore = 0;
  bool sharedDomain = false;

  // Find best pair of columns for source/target
  for (vtkIdType col1 = 0; col1 < numCol; ++col1)
    {
    for (vtkIdType col2 = col1+1; col2 < numCol; ++col2)
      {
      int type1 = this->Types[col1];
      int type2 = this->Types[col2];
      int rel = this->Relationships[col1][col2];
      bool category1 = (type1 == STRING_CATEGORY || type1 == INTEGER_CATEGORY);
      bool category2 = (type2 == STRING_CATEGORY || type2 == INTEGER_CATEGORY);
      if (rel == SHARED_DOMAIN)
        {
        if (bestScore < 10 && type1 == STRING_CATEGORY)
          {
          bestScore = 10;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          sharedDomain = true;
          }
        else if (bestScore < 8)
          {
          bestScore = 8;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          sharedDomain = true;
          }
        }
      else
        {
        if (bestScore < 7
            && type1 == STRING_CATEGORY
            && type2 == STRING_CATEGORY)
          {
          bestScore = 7;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        else if (bestScore < 6
            && type1 == INTEGER_CATEGORY
            && type2 == INTEGER_CATEGORY)
          {
          bestScore = 6;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        else if (bestScore < 5 && (category1 && category2))
          {
          bestScore = 5;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        else if (bestScore < 4 && (type1 == STRING_CATEGORY || type2 == STRING_CATEGORY))
          {
          bestScore = 4;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        else if (bestScore < 3 && (type1 == INTEGER_CATEGORY || type2 == INTEGER_CATEGORY))
          {
          bestScore = 3;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        else if (bestScore < 2 && (type1 != CONTINUOUS || type2 != CONTINUOUS))
          {
          bestScore = 2;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        else if (bestScore < 1)
          {
          bestScore = 1;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
          }
        }
      }
    }

  // No source/target fields
  if (source == "" && target == "")
    {
    return;
    }

  std::cerr << "GRAPH chose " << source << ", " << target << " with score " << bestScore << std::endl;

  this->Attributes[this->ViewType]["source"] = source.c_str();
  this->Attributes[this->ViewType]["target"] = target.c_str();

  vtkNew<vtkTableToGraph> ttg;
  ttg->SetInputData(this->Table.GetPointer());
  if (sharedDomain)
    {
    ttg->AddLinkVertex(source.c_str(), "domain");
    ttg->AddLinkVertex(target.c_str(), "domain");
    }
  else
    {
    ttg->AddLinkVertex(source.c_str(), "source");
    ttg->AddLinkVertex(target.c_str(), "target");
    }
  ttg->AddLinkEdge(source.c_str(), target.c_str());
  ttg->Update();

  vtkNew<vtkPoints> points;
  vtkIdType numVert = ttg->GetOutput()->GetNumberOfVertices();
  for (vtkIdType i = 0; i < numVert; ++i)
    {
    double angle = vtkMath::RadiansFromDegrees(360.0*i/numVert);
    points->InsertNextPoint(200*cos(angle) + 200, 200*sin(angle) + 200, 0.0);
    }
  ttg->GetOutput()->SetPoints(points.GetPointer());

  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  this->View->GetScene()->AddItem(trans.GetPointer());

  graphItem->SetGraph(ttg->GetOutput());
  trans->AddItem(graphItem.GetPointer());

  graphItem->GetLayout()->SetStrength(1);
  graphItem->GetLayout()->SetAlpha(0.1f);

  this->AnimationTimer.start();
  //graphItem->StartLayoutAnimation(this->GetRenderWindow()->GetInteractor());
}
