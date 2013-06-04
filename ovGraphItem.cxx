/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovGraphItem.h"

#include "vtkAbstractArray.h"
#include "vtkColorSeries.h"
#include "vtkContext2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextScene.h"
#include "vtkDataSetAttributes.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkIncrementalForceLayout.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkTextProperty.h"
#include "vtkTransform2D.h"

vtkStandardNewMacro(ovGraphItem);

ovGraphItem::ovGraphItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
  this->FocusedVertices = vtkSmartPointer<vtkIdTypeArray>::New();
}

void ovGraphItem::InitializeColorLookup()
{
  if (!this->GetGraph())
    {
    return;
    }
  this->ColorLookup->ResetAnnotations();
  vtkAbstractArray *arr = this->GetGraph()->GetVertexData()->GetAbstractArray(this->ColorArray.c_str());
  if (arr)
    {
    if (vtkDataArray::SafeDownCast(arr))
      {
      vtkDataArray *darr = vtkDataArray::SafeDownCast(arr);
      this->ColorLookup->SetNumberOfTableValues(6);
      this->ColorLookup->SetScaleToLog10();
      unsigned char colors[] = {
          //247,252,253,
          //229,245,249,
          //204,236,230,
          153,216,201,
          102,194,164,
          65,174,118,
          35,139,69,
          0,109,44,
          0,68,27};
      for (int i = 0; i < 6; ++i)
        {
        this->ColorLookup->SetTableValue(
              i, colors[i*3] / 255., colors[i*3+1] / 255., colors[i*3+2] / 255., 1.);
        }
      this->ColorLookup->IndexedLookupOff();
      this->ColorLookup->SetRange(darr->GetRange());
      }
    else
      {
      this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
      this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
      this->ColorLookup->IndexedLookupOn();
      vtkIdType c = 0;
      for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i)
        {
        vtkVariant value = arr->GetVariantValue(i);
        if (this->ColorIndexMap.count(value) == 0)
          {
          this->ColorIndexMap[value] = c;
          c = (c + 1) % this->ColorSeries->GetNumberOfColors();
          }
        }
      }
    }
}

void ovGraphItem::SetGraph(vtkGraph *graph)
{
  this->Superclass::SetGraph(graph);
  this->InitializeColorLookup();
}

void ovGraphItem::SetColorArray(const std::string &name)
{
  this->ColorArray = name;
  this->InitializeColorLookup();
}

vtkColor4ub ovGraphItem::EdgeColor(vtkIdType edgeIdx, vtkIdType vtkNotUsed(point))
{
  return vtkColor4ub(128, 128, 128, 128);
}

float ovGraphItem::EdgeWidth(vtkIdType edgeIdx, vtkIdType point)
{
  vtkGraph *g = this->GetGraph();
  int sdist = this->DistanceFromFocus(g->GetSourceVertex(edgeIdx));
  int tdist = this->DistanceFromFocus(g->GetTargetVertex(edgeIdx));
  if ((sdist == 0 && tdist <= 1) || (tdist == 0 && sdist <= 1))
    {
    return 3;
    }
  return 1;
}

vtkColor4ub ovGraphItem::VertexColor(vtkIdType vertex)
{
  int dist = this->DistanceFromFocus(vertex);
  if (dist == 0)
    {
    return vtkColor4ub(255, 0, 0, 255);
    }
  if (dist == 1)
    {
    return vtkColor4ub(255, 128, 128, 255);
    }
  // vtkScalarsToColors - add annotations, value (vtkVariant)
  // Boolean IndexedLookup - whether colors are discrete or continuous
  // GetAnnotatedValueIndex, then GetTableValue
  // Build lookup table - set color swatches
  // common/colors vtkColorSeries, creates lookup table for you
  vtkAbstractArray *arr = this->GetGraph()->GetVertexData()->GetAbstractArray(this->ColorArray.c_str());
  if (arr)
    {
    if (this->ColorLookup->GetIndexedLookup())
      {
      vtkIdType index = this->ColorIndexMap[arr->GetVariantValue(vertex)];
      double rgba[4];
      this->ColorLookup->GetTableValue(index, rgba);
      return vtkColor4ub(static_cast<unsigned char>(rgba[0]*255),
                         static_cast<unsigned char>(rgba[1]*255),
                         static_cast<unsigned char>(rgba[2]*255),
                         static_cast<unsigned char>(rgba[3]*255));
      }
    else
      {
      vtkDataArray *darr = vtkDataArray::SafeDownCast(arr);
      if (darr)
        {
        double value = darr->GetTuple1(vertex);
        double rgba[4];
        this->ColorLookup->GetColor(value, rgba);
        rgba[3] = this->ColorLookup->GetOpacity(value);
        return vtkColor4ub(static_cast<unsigned char>(rgba[0]*255),
                           static_cast<unsigned char>(rgba[1]*255),
                           static_cast<unsigned char>(rgba[2]*255),
                           static_cast<unsigned char>(rgba[3]*255));
        }
      }
    }
  return vtkColor4ub(128, 128, 128, 255);
}

vtkStdString ovGraphItem::VertexTooltip(vtkIdType vertex)
{
  vtkAbstractArray *arr = this->GetGraph()->GetVertexData()->GetAbstractArray(this->TooltipArray.c_str());
  if (arr)
    {
    return arr->GetVariantValue(vertex).ToString();
    }
  return "";
}

int ovGraphItem::DistanceFromFocus(vtkIdType vertex)
{
  if (this->FocusedVertices->GetNumberOfTuples() == 0)
    {
    return 2;
    }
  if (this->FocusedVertices->LookupValue(vertex) >= 0)
    {
    return 0;
    }
  vtkGraph *g = this->GetGraph();
  for (vtkIdType i = 0; i < this->FocusedVertices->GetNumberOfTuples(); ++i)
    {
    vtkIdType v = this->FocusedVertices->GetValue(i);
    bool found = false;
    for (vtkIdType j = 0; j < g->GetOutDegree(vertex); ++j)
      {
      if (g->GetOutEdge(vertex, j).Target == v)
        {
        found = true;
        break;
        }
      }
    for (vtkIdType j = 0; j < g->GetInDegree(vertex); ++j)
      {
      if (g->GetInEdge(vertex, j).Source == v)
        {
        found = true;
        break;
        }
      }
    if (!found)
      {
      return 2;
      }
    }
  return 1;
}

vtkStdString ovGraphItem::VertexLabel(vtkIdType vertex)
{
  vtkGraph *g = this->GetGraph();
  vtkAbstractArray *arr = g->GetVertexData()->GetAbstractArray(this->LabelArray.c_str());
  if (arr)
    {
    int dist = this->DistanceFromFocus(vertex);
    if (dist < 2)
      {
      return arr->GetVariantValue(vertex).ToString();
      }
    }
  return "";
}

void ovGraphItem::PaintBuffers(vtkContext2D *painter)
{
  // Turn off the tooltip if the superclass turned it on.
  this->PlaceTooltip(-1);

  vtkIdType hitVertex = this->HitVertex(this->MouseLocation);
  this->FocusedVertices->Initialize();
  if (hitVertex >= 0)
    {
    this->FocusedVertices->InsertNextValue(hitVertex);
    }

  this->Superclass::PaintBuffers(painter);

  this->GetLayout()->SetGravityPoint(vtkVector2f(this->GetScene()->GetSceneWidth()/2, this->GetScene()->GetSceneHeight()/2));

  // Paint labels
  float scale[2];
  float position[2];
  painter->GetTransform()->GetScale(scale);
  painter->GetTransform()->GetPosition(position);
  float sceneMinX = -position[0]/scale[0];
  float sceneMaxX = (this->Scene->GetViewWidth() - position[0])/scale[0];
  float sceneMinY = -position[1]/scale[1];
  float sceneMaxY = (this->Scene->GetViewHeight() - position[1])/scale[1];
  float sceneWidth = sceneMaxX - sceneMinX;
  float sceneHeight = sceneMaxY - sceneMinY;

  float resolution = sceneWidth/10.0f;
  int w = sceneWidth/resolution;
  int h = sceneHeight/resolution;
  std::vector<bool> buffer(w*h, false);
  painter->GetTextProp()->SetColor(0, 0, 0);
  painter->GetTextProp()->SetJustificationToCentered();
  painter->GetTextProp()->BoldOff();
  int numRendered = 0;
  for (vtkIdType i = 0; i < this->NumberOfVertices(); ++i)
    {
    vtkVector2f pos = this->VertexPosition(i);
    vtkStdString label = this->VertexLabel(i);
    if (label.length() == 0)
      {
      continue;
      }
    float bounds[4];
    painter->ComputeStringBounds(label, bounds);
    if (bounds[2] == 0.0f && bounds[3] == 0.0f)
      {
      continue;
      }
    bool overlap = false;
    int xmin = (pos.GetX() + bounds[0] - sceneMinX)/resolution;
    int xmax = (pos.GetX() + bounds[2] - sceneMinX)/resolution;
    int ymin = (pos.GetY() + bounds[1] - sceneMinY)/resolution;
    int ymax = (pos.GetY() + bounds[3] - sceneMinY)/resolution;
    if (xmin < 0 || xmax >= w || ymin < 0 || ymax >= h)
      {
      continue;
      }
    for (int x = xmin; x <= xmax; ++x)
      {
      for (int y = ymin; y <= ymax; ++y)
        {
        if (x >= 0 && x < w && y >= 0 && y < h && buffer[h*x + y])
          {
          overlap = true;
          break;
          }
        }
      }
    if (!overlap || this->DistanceFromFocus(i) == 0)
      {
      painter->GetTextProp()->SetFontSize(this->DistanceFromFocus(i) == 0 ? 20 : 12);
      painter->DrawString(pos.GetX(), pos.GetY(), label);
      ++numRendered;
      for (int x = xmin; x <= xmax; ++x)
        {
        for (int y = ymin; y <= ymax; ++y)
          {
          if (x >= 0 && x < w && y >= 0 && y < h)
            {
            buffer[h*x + y] = true;
            }
          }
        }
      }
    }
}

bool ovGraphItem::Hit(const vtkContextMouseEvent &event)
{
  return true;
}

bool ovGraphItem::MouseMoveEvent(const vtkContextMouseEvent &event)
{
  this->MouseLocation = event.GetPos();

  this->Superclass::MouseMoveEvent(event);
  return (this->HitVertex(event.GetPos()) >= 0) || (this->GetLayout()->GetFixed() >= 0);
}

bool ovGraphItem::MouseButtonPressEvent(const vtkContextMouseEvent &event)
{
  this->Superclass::MouseButtonPressEvent(event);
  return this->HitVertex(event.GetPos()) >= 0;
}
