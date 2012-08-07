#include "ovView.h"
#include "ovGLContext.h"

#include "QVTKInteractor.h"
#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkInteractorObserver.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkBlockItem.h"
#include "vtkTable.h"
#include "vtkStringArray.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkDelimitedTextReader.h"
#include "vtkMath.h"
#include "vtkIncrementalForceLayout.h"

#include "vtkRandomGraphSource.h"
#include "vtkGraphLayoutView.h"
#include "vtkTableToGraph.h"
#include "vtkGraphItem.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRendererCollection.h"

#include <set>
#include <algorithm>

ovView::ovView(QGraphicsItem *p)
  : QVTKGraphicsItem(ovGLContext::instance(), p)
{
  std::cerr << "set render window" << std::endl;
  this->View->SetRenderWindow(this->GetRenderWindow());
  vtkNew<vtkBlockItem> block;
  block->SetDimensions(200, 200, 100, 100);
  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  trans->AddItem(block.GetPointer());
  this->View->GetScene()->AddItem(trans.GetPointer());
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

void ovView::setUrl(QUrl &url)
{
  this->Url = url;
  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(url.toLocalFile().toLatin1().data());
  reader->SetHaveHeaders(true);
  reader->Update();
  this->setTable(reader->GetOutput());
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
      if (this->Types[col1] != this->Types[col2]
          || this->Types[col1] == INTEGER_DATA
          || this->Types[col1] == STRING_DATA
          || this->Types[col1] == CONTINUOUS)
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
      if (numShared > 0.1*numRow)
        {
        this->Relationships[col1][col2] = SHARED_DOMAIN;
        }
      else
        {
        this->Relationships[col1][col2] = UNRELATED;
        }
      }
    }
  this->setupGraph();
}

void ovView::setupGraph()
{
  vtkIdType numCol = this->Table->GetNumberOfColumns();
  std::string source = "";
  std::string target = "";

  double bestScore = 0;

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
          }
        else if (bestScore < 8)
          {
          bestScore = 8;
          source = this->Table->GetColumn(col1)->GetName();
          target = this->Table->GetColumn(col2)->GetName();
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

  std::cerr << "chose " << source << ", " << target << " with score " << bestScore << std::endl;

  vtkNew<vtkTableToGraph> ttg;
  ttg->SetInputData(this->Table.GetPointer());
  ttg->AddLinkVertex(source.c_str());
  ttg->AddLinkVertex(target.c_str());
  ttg->AddLinkEdge(source.c_str(), target.c_str());
  ttg->Update();
  //this->Table->Dump(10);
  //vtkNew<vtkTable> vertexData;
  //vertexData->SetRowData(ttg->GetOutput()->GetVertexData());
  //vertexData->Dump(10);

  vtkNew<vtkPoints> points;
  vtkIdType numVert = ttg->GetOutput()->GetNumberOfVertices();
  for (vtkIdType i = 0; i < numVert; ++i)
    {
    double angle = vtkMath::RadiansFromDegrees(360.0*i/numVert);
    points->InsertNextPoint(200*cos(angle) + 200, 200*sin(angle) + 200, 0.0);
    }
  ttg->GetOutput()->SetPoints(points.GetPointer());

  this->View->GetScene()->ClearItems();

  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  this->View->GetScene()->AddItem(trans.GetPointer());

  vtkNew<vtkGraphItem> graphItem;
  graphItem->SetGraph(ttg->GetOutput());
  trans->AddItem(graphItem.GetPointer());

  graphItem->GetLayout()->SetStrength(1);
  graphItem->StartLayoutAnimation(this->GetRenderWindow()->GetInteractor());
}
