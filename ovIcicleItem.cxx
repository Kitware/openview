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
#include "vtkPen.h"
#include "vtkPoints.h"
#include "vtkTextProperty.h"
#include "vtkTooltipItem.h"
#include "vtkTransform2D.h"
#include "vtkTree.h"
#include "vtkTreeDFSIterator.h"

vtkStandardNewMacro(ovTreeringItem);

ovTreeringItem::ovTreeringItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
  this->Tooltip->SetVisible(false);
  this->TargetVertex = 0;
  this->Scale->Identity();
  this->Translate->Identity();
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
  this->Scale->Identity();
  this->Translate->Identity();
  double rng[4];
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));
  area->GetRange(rng, 0);
  area->GetRange(rng+2, 2);
  this->Scale->GetMatrix()->SetElement(0, 0, 1.0/(rng[1] - rng[0]));
  this->Scale->GetMatrix()->SetElement(1, 1, 1.0/(rng[3] - rng[2]));
  this->Translate->GetMatrix()->SetElement(0, 2, -rng[0]);
  this->Translate->GetMatrix()->SetElement(1, 2, -rng[2]);
  this->TargetVertex = this->Tree->GetRoot();
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
  vtkVector2f transformed = pos;
  transformed[0] = transformed[0]/this->GetScene()->GetSceneWidth();
  transformed[1] = transformed[1]/this->GetScene()->GetSceneHeight();
  this->Scale->InverseTransformPoints(transformed.GetData(), transformed.GetData(), 1);
  this->Translate->InverseTransformPoints(transformed.GetData(), transformed.GetData(), 1);
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));
  for (vtkIdType v = 0; v < this->Tree->GetNumberOfVertices(); ++v)
    {
    float *pt = area->GetPointer(4*v);
    if (transformed.GetX() >= pt[0] && transformed.GetX() <= pt[1] && transformed.GetY() >= pt[2] && transformed.GetY() <= pt[3])
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

bool ovTreeringItem::MouseButtonReleaseEvent(const vtkContextMouseEvent &event)
{
  if (event.GetButton() == vtkContextMouseEvent::LEFT_BUTTON)
    {
    vtkIdType v = this->HitVertex(event.GetPos());
    vtkIdType parent = this->Tree->GetParent(v);
    if (parent == this->TargetVertex)
      {
      this->TargetVertex = this->Tree->GetRoot();
      }
    else
      {
      this->TargetVertex = parent;
      }
    return true;
    }
  return false;
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

bool ovTreeringItem::Paint(vtkContext2D *painter)
{
  painter->GetPen()->SetColor(0, 0, 0);
  painter->GetBrush()->SetColor(128, 128, 128);
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));

  painter->PushMatrix();
  vtkNew<vtkTransform2D> transform;
  transform->Scale(this->GetScene()->GetSceneWidth(), this->GetScene()->GetSceneHeight());
  painter->AppendTransform(transform.GetPointer());

#if 0

  float *targetRect = area->GetPointer(4*this->TargetVertex);

  float currentRect[4];

  float scale[2];
  this->Scale->GetScale(scale);
  float pos[2];
  this->Translate->GetPosition(pos);
  currentRect[0] = -pos[0];
  currentRect[1] = currentRect[0] + 1.0f/scale[0];
  currentRect[2] = -pos[1];
  currentRect[3] = currentRect[2] + 1.0f/scale[1];

  float deltaRect[4];
  deltaRect[0] = targetRect[0] - currentRect[0];
  deltaRect[1] = targetRect[1] - currentRect[1];
  deltaRect[2] = targetRect[2] - currentRect[2];
  deltaRect[3] = targetRect[3] - currentRect[3];

  float newRect[4];
  newRect[0] = currentRect[0] + 0.1*deltaRect[0];
  newRect[1] = currentRect[1] + 0.1*deltaRect[1];
  newRect[2] = currentRect[2] + 0.1*deltaRect[2];
  newRect[3] = currentRect[3] + 0.1*deltaRect[3];
  float newScale[2] = {1.0f/(newRect[1]-newRect[0]), 1.0f/(newRect[3]-newRect[2])};
  float newPos[2] = {-newRect[0], -newRect[2]};

  this->Scale->GetMatrix()->SetElement(0, 0, newScale[0]);
  this->Scale->GetMatrix()->SetElement(1, 1, newScale[1]);
  this->Translate->GetMatrix()->SetElement(0, 2, newPos[0]);
  this->Translate->GetMatrix()->SetElement(1, 2, newPos[1]);
#endif

  painter->AppendTransform(this->Scale.GetPointer());
  painter->AppendTransform(this->Translate.GetPointer());

  vtkNew<vtkTreeDFSIterator> it;
  it->SetTree(this->Tree.GetPointer());
  it->SetMode(vtkTreeDFSIterator::FINISH);
  it->SetStartVertex(this->Tree->GetRoot());
  vtkDataArray *arr = vtkDataArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str()));
  while (it->HasNext())
    {
    vtkIdType i = it->Next();
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
    painter->DrawRect(pt[0], pt[2], (pt[1]-pt[0]), (pt[3]-pt[2]));
    }

  it->Restart();
  painter->GetTextProp()->SetFontSize(20);
  painter->GetTextProp()->SetOpacity(0.5);
  painter->GetTextProp()->SetColor(0, 0, 0);
  painter->GetTextProp()->SetBold(true);
  painter->GetTextProp()->SetOrientation(90);
  int numLabels = 0;
  while (it->HasNext())
    {
    vtkIdType i = it->Next();
    float *pt = area->GetPointer(4*i);
    vtkStdString label = this->VertexLabel(i);
    float bounds[4];
    painter->ComputeStringBounds(label, bounds);
    if (numLabels < 100 && bounds[2] > 0.0f && bounds[3] > 0.0f)
      {
      float center[2] = {(pt[0]+pt[1])/2 + bounds[2]/2, (pt[2]+pt[3])/2 - bounds[3]/2};
      painter->DrawString(center[0], center[1], label);
      ++numLabels;
      }
    }

  painter->PopMatrix();
  this->PaintChildren(painter);
  return true;
}
