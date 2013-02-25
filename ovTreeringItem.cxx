/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovTreeringItem.h"

#include "vtkAbstractArray.h"
#include "vtkBrush.h"
#include "vtkColorSeries.h"
#include "vtkContext2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextScene.h"
#include "vtkDataSetAttributes.h"
#include "vtkFloatArray.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkPen.h"
#include "vtkPoints.h"
#include "vtkTextProperty.h"
#include "vtkTooltipItem.h"
#include "vtkTransform2D.h"
#include "vtkTree.h"
#include "vtkTreeDFSIterator.h"

#include <algorithm>

vtkStandardNewMacro(ovTreeringItem);

ovTreeringItem::ovTreeringItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
  this->Tooltip->SetVisible(false);
  this->AddItem(this->Tooltip.GetPointer());
}

void ovTreeringItem::InitializeColorLookup()
{
  this->ColorLookup->ResetAnnotations();
  if (this->ColorArray == "parent")
    {
    this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
    this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
    this->ColorLookup->IndexedLookupOn();
    for (vtkIdType i = 0; i < this->Tree->GetNumberOfVertices(); ++i)
      {
      this->ColorLookup->SetAnnotation(this->Tree->GetParent(i), "");
      }
    return;
    }
  vtkAbstractArray *arr = this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str());
  if (arr)
    {
    if (vtkDataArray::SafeDownCast(arr))
      {
      vtkDataArray *darr = vtkDataArray::SafeDownCast(arr);
      this->ColorLookup->SetNumberOfTableValues(6);
      //this->ColorLookup->SetScaleToLog10();
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
      double range[2];
      range[0] = VTK_DOUBLE_MAX;
      range[1] = VTK_DOUBLE_MIN;
      for (vtkIdType i = 0; i < this->Tree->GetNumberOfVertices(); ++i)
        {
        if (!this->Tree->IsLeaf(i))
          {
          continue;
          }
        range[0] = std::min(range[0], darr->GetTuple1(i));
        range[1] = std::max(range[1], darr->GetTuple1(i));
        }
      this->ColorLookup->SetRange(range);
      }
    else
      {
      this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
      this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
      this->ColorLookup->IndexedLookupOn();
      for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i)
        {
        this->ColorLookup->SetAnnotation(arr->GetVariantValue(i), "");
        }
      }
    }
}

void ovTreeringItem::SetTree(vtkTree *tree)
{
  this->Tree->ShallowCopy(tree);
  this->InitializeColorLookup();
}

void ovTreeringItem::SetColorArray(const std::string &name)
{
  this->ColorArray = name;
  this->InitializeColorLookup();
}

bool ovTreeringItem::MouseMoveEvent(const vtkContextMouseEvent &event)
{
  if (event.GetButton() == vtkContextMouseEvent::NO_BUTTON)
    {
    vtkIdType v = this->HitVertex(event.GetPos());
    this->Scene->SetDirty(true);
    if (v < 0)
      {
      this->Tooltip->SetVisible(false);
      return true;
      }
    vtkStdString text = this->VertexTooltip(v);
    if (text == "")
      {
      this->Tooltip->SetVisible(false);
      return true;
      }
    this->PlaceTooltip(v, event.GetPos());
    this->Tooltip->SetText(text);
    this->Tooltip->SetVisible(true);
    return true;
    }

  if (this->Tooltip->GetVisible())
    {
    vtkIdType v = this->HitVertex(event.GetPos());
    this->PlaceTooltip(v, event.GetPos());
    this->Scene->SetDirty(true);
    }

  return false;
}

bool ovTreeringItem::MouseEnterEvent(const vtkContextMouseEvent &vtkNotUsed(event))
{
  return true;
}

bool ovTreeringItem::MouseLeaveEvent(const vtkContextMouseEvent &vtkNotUsed(event))
{
  this->Tooltip->SetVisible(false);
  return true;
}

vtkIdType ovTreeringItem::HitVertex(const vtkVector2f &pos)
{
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));
  float radius = pos.Norm();
  float angle = vtkMath::DegreesFromRadians(atan2(pos.GetY(), pos.GetX()));
  if (angle < 0.0f)
    {
    angle += 360.0f;
    }
  for (vtkIdType v = 0; v < this->Tree->GetNumberOfVertices(); ++v)
    {
    float *pt = area->GetPointer(4*v);
    if (angle >= pt[0] && angle <= pt[1] && radius >= pt[2] && radius <= pt[3])
      {
      return v;
      }
    }
  return -1;
}

std::string ovTreeringItem::VertexTooltip(vtkIdType vertex)
{
  vtkAbstractArray *arr = this->Tree->GetVertexData()->GetAbstractArray(
    this->Tree->IsLeaf(vertex) ?
    this->TooltipArray.c_str() :
    this->GroupNameArray.c_str() );
  if (arr)
    {
    return arr->GetVariantValue(vertex).ToString();
    }
  return "";
}

std::string ovTreeringItem::VertexLabel(vtkIdType vertex)
{
  vtkAbstractArray *arr = this->Tree->GetVertexData()->GetAbstractArray(
    this->Tree->IsLeaf(vertex) ?
    this->LabelArray.c_str() :
    this->GroupNameArray.c_str() );
  if (arr)
    {
    return arr->GetVariantValue(vertex).ToString();
    }
  return "";
}

bool ovTreeringItem::Hit(const vtkContextMouseEvent &event)
{
  vtkIdType v = this->HitVertex(event.GetPos());
  return (v >= 0);
}

void ovTreeringItem::PlaceTooltip(vtkIdType v, const vtkVector2f &pos)
{
  if (v >= 0)
    {
    this->Tooltip->SetPosition(pos[0]+5, pos[1]+5);
    this->Tooltip->SetVisible(true);
    }
  else
    {
    this->Tooltip->SetVisible(false);
    }
}

class SortOrder
{
public:
    SortOrder(vtkGraph *g) : sortGraph(g) {;}

    bool operator()(vtkIdType a, vtkIdType b) const
    {
        return sortGraph->GetInDegree(a) + sortGraph->GetOutDegree(a) > sortGraph->GetInDegree(b) + sortGraph->GetOutDegree(b);
    }

private:
    vtkGraph *sortGraph;
};

bool ovTreeringItem::Paint(vtkContext2D *painter)
{
  painter->GetPen()->SetColor(0, 0, 0);
  painter->GetBrush()->SetColor(128, 128, 128);
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));

  vtkNew<vtkTreeDFSIterator> it;
  it->SetTree(this->Tree.GetPointer());
  it->SetMode(vtkTreeDFSIterator::DISCOVER);
  it->SetStartVertex(this->Tree->GetRoot());
  vtkDataArray *arr = vtkDataArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str()));
  while (it->HasNext())
    {
    vtkIdType i = it->Next();
    if (i == this->Tree->GetRoot())
      {
      continue;
      }
    float *pt = area->GetPointer(4*i);
    if (this->ColorArray == "parent")
      {
      vtkIdType index = this->ColorLookup->GetAnnotatedValueIndex(this->Tree->GetParent(i));
      double rgba[4];
      this->ColorLookup->GetTableValue(index, rgba);
      painter->GetBrush()->SetColorF(rgba);
      }
    else
      {
      unsigned char *rgba = this->ColorLookup->MapValue(arr->GetTuple1(i));
      painter->GetBrush()->SetColor(rgba);
      }
    painter->GetPen()->SetWidth(1.0f);
    painter->GetPen()->SetOpacityF(0.5f);
    painter->GetBrush()->SetOpacityF(1.0f);

    float innerRadius = pt[2];
    float outerRadius = pt[3];
    float startAngle = pt[0];
    float endAngle = pt[1];

    painter->DrawEllipseWedge(0.0f, 0.0f, outerRadius, outerRadius, innerRadius, innerRadius, startAngle, endAngle);

    // Draw border
    painter->DrawEllipticArc(0.0f, 0.0f, outerRadius, outerRadius, startAngle, endAngle);
    painter->DrawEllipticArc(0.0f, 0.0f, innerRadius, innerRadius, startAngle, endAngle);

    float p[4];
    float rstart = vtkMath::RadiansFromDegrees(startAngle);
    p[0] = innerRadius * cos(rstart);
    p[1] = innerRadius * sin(rstart);
    p[2] = outerRadius * cos(rstart);
    p[3] = outerRadius * sin(rstart);
    painter->DrawLine(p);

    float rend = vtkMath::RadiansFromDegrees(endAngle);
    p[0] = innerRadius * cos(rend);
    p[1] = innerRadius * sin(rend);
    p[2] = outerRadius * cos(rend);
    p[3] = outerRadius * sin(rend);
    painter->DrawLine(p);
    }

  painter->GetTextProp()->SetFontSize(20);
  painter->GetTextProp()->SetOpacity(0.5);
  painter->GetTextProp()->SetColor(0, 0, 0);
  painter->GetTextProp()->SetBold(true);
  painter->GetTextProp()->SetJustificationToCentered();

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
  painter->GetTextProp()->BoldOn();
  int numRendered = 0;

  std::vector<vtkIdType> indices;
  for (vtkIdType i = 0; i < this->Tree->GetNumberOfVertices(); ++i)
    {
    indices.push_back(i);
    }
  std::sort(indices.begin(), indices.end(), SortOrder(this->Tree.GetPointer()));

  for (size_t ind = 0; ind < indices.size(); ++ind)
    {
    vtkIdType i = indices[ind];
    float *pt = area->GetPointer(4*i);
    float angle = vtkMath::RadiansFromDegrees((pt[0] + pt[1])/2.0f);
    float radius = (pt[2] + pt[3])/2.0f;
    vtkVector2f pos(radius * cos(angle), radius * sin(angle));
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
    if (!overlap)
      {
      //painter->GetTextProp()->SetFontSize(this->DistanceFromFocus(i) == 0 ? 20 : 12);
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



  this->PaintChildren(painter);
  return true;
}
