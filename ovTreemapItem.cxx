/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovTreemapItem.h"

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
#include "vtkTextProperty.h"
#include "vtkTooltipItem.h"
#include "vtkTransform2D.h"
#include "vtkTree.h"
#include "vtkTreeDFSIterator.h"

vtkStandardNewMacro(ovTreemapItem);

ovTreemapItem::ovTreemapItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
  this->Tooltip->SetVisible(false);
  this->TargetVertex = 0;
  this->Scale->Identity();
  this->Translate->Identity();
  this->AddItem(this->Tooltip.GetPointer());
}

void ovTreemapItem::InitializeColorLookup()
{
  this->ColorLookup->ResetAnnotations();
  if (this->ColorArray == "parent")
    {
    this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
    this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
    this->ColorLookup->IndexedLookupOn();
    vtkIdType c = 0;
    for (vtkIdType i = 0; i < this->Tree->GetNumberOfVertices(); ++i)
      {
      vtkVariant value = this->Tree->GetParent(i);
      if (this->ColorIndexMap.count(value) == 0)
        {
        this->ColorIndexMap[value] = c;
        c = (c + 1) % this->ColorSeries->GetNumberOfColors();
        }
      }
    }
  vtkAbstractArray *arr = this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str());
  if (arr)
    {
    if (vtkDataArray::SafeDownCast(arr))
      {
      vtkDataArray *darr = vtkDataArray::SafeDownCast(arr);
      this->ColorLookup->SetNumberOfTableValues(6);
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

void ovTreemapItem::SetTree(vtkTree *tree)
{
  this->Tree->ShallowCopy(tree);
  this->InitializeColorLookup();
  this->Scale->Identity();
  this->Translate->Identity();
  this->TargetVertex = this->Tree->GetRoot();
}

void ovTreemapItem::SetColorArray(const std::string &name)
{
  this->ColorArray = name;
  this->InitializeColorLookup();
}

bool ovTreemapItem::MouseMoveEvent(const vtkContextMouseEvent &event)
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

bool ovTreemapItem::MouseEnterEvent(const vtkContextMouseEvent &vtkNotUsed(event))
{
  return true;
}

bool ovTreemapItem::MouseLeaveEvent(const vtkContextMouseEvent &vtkNotUsed(event))
{
  this->Tooltip->SetVisible(false);
  return true;
}

vtkIdType ovTreemapItem::HitVertex(const vtkVector2f &pos)
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
    if (this->Tree->IsLeaf(v) && transformed.GetX() >= pt[0] && transformed.GetX() <= pt[1] && transformed.GetY() >= pt[2] && transformed.GetY() <= pt[3])
      {
      return v;
      }
    }
  return -1;
}

std::string ovTreemapItem::VertexTooltip(vtkIdType vertex)
{
  vtkAbstractArray *arr = this->Tree->GetVertexData()->GetAbstractArray(this->TooltipArray.c_str());
  if (arr)
    {
    return arr->GetVariantValue(vertex).ToString();
    }
  return "";
}

bool ovTreemapItem::Hit(const vtkContextMouseEvent &event)
{
  vtkIdType v = this->HitVertex(event.GetPos());
  return (v >= 0);
}

bool ovTreemapItem::MouseButtonReleaseEvent(const vtkContextMouseEvent &event)
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

void ovTreemapItem::PlaceTooltip(vtkIdType v, const vtkVector2f &pos)
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

bool ovTreemapItem::Paint(vtkContext2D *painter)
{
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));
  if (this->Tree->GetNumberOfVertices() == 0 || !area)
    {
    return false;
    }

  painter->GetPen()->SetColor(0, 0, 0);
  painter->GetBrush()->SetColor(128, 128, 128);

  painter->PushMatrix();
  vtkNew<vtkTransform2D> transform;
  float sceneWidth = this->GetScene()->GetSceneWidth();
  float sceneHeight = this->GetScene()->GetSceneHeight();
  transform->Scale(sceneWidth, sceneHeight);
  painter->AppendTransform(transform.GetPointer());

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

  painter->AppendTransform(this->Scale.GetPointer());
  painter->AppendTransform(this->Translate.GetPointer());

  vtkNew<vtkTreeDFSIterator> it;
  it->SetTree(this->Tree.GetPointer());
  it->SetMode(vtkTreeDFSIterator::DISCOVER);
  it->SetStartVertex(this->Tree->GetRoot());
  vtkDataArray *arr = vtkDataArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str()));
  while (it->HasNext())
    {
    vtkIdType i = it->Next();
    float *pt = area->GetPointer(4*i);
    if (!this->Tree->IsLeaf(i))
      {
      painter->GetBrush()->SetOpacityF(0.0f);
      float width = std::max(2.0f, 6.0f - 2.0f * this->Tree->GetLevel(i));
      painter->GetPen()->SetWidth(width);
      painter->GetPen()->SetOpacityF(1.0f);
      }
    else
      {
      if (this->ColorArray == "parent")
        {
        vtkIdType index = this->ColorIndexMap[this->Tree->GetParent(i)];
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
      }
    painter->DrawRect(pt[0], pt[2], (pt[1] - pt[0]), (pt[3] - pt[2]));
    }

  vtkAbstractArray *labelArr = this->Tree->GetVertexData()->GetAbstractArray(this->LabelArray.c_str());
  if (labelArr)
    {
    int minFontSize = 10;
    painter->GetTextProp()->SetOpacity(0.5);
    painter->GetTextProp()->SetColor(0, 0, 0);
    painter->GetTextProp()->SetBold(true);
    it->Restart();
    vtkTransform2D *fullTransform = painter->GetTransform();
    float fullScreenArea = sceneWidth * sceneHeight;
    while (it->HasNext())
      {
      vtkIdType i = it->Next();
      float *pt = area->GetPointer(4*i);
      float bottomLeft[2] = {pt[0], pt[2]};
      float topRight[2] = {pt[1], pt[3]};
      float boxWidth = topRight[0] - bottomLeft[0];

      float screenBottomLeft[2];
      float screenTopRight[2];
      fullTransform->TransformPoints(bottomLeft, screenBottomLeft, 1);
      fullTransform->TransformPoints(topRight, screenTopRight, 1);
      float screenCenter[2] = {(screenBottomLeft[0] + screenTopRight[0]) / 2, (screenBottomLeft[1] + screenTopRight[1]) / 2};
      bool onScreenX = screenCenter[0] > 0 && screenCenter[0] < sceneWidth;
      bool onScreenY = screenCenter[1] > 0 && screenCenter[1] < sceneHeight;
      float screenBoxWidth = screenTopRight[0] - screenBottomLeft[0];
      float screenBoxHeight = screenTopRight[1] - screenBottomLeft[1];
      float screenArea = screenBoxWidth * screenBoxHeight;

      int maxFontSize = this->Tree->GetLevel(i) <= 1 ? 32 : 15;

      // Determine if this node is a descendent of the currently
      // focused (i.e. zoomed-in-on) node.
      bool focused = false;
      vtkIdType curNode = i;
      while (curNode >= 0)
        {
        if (curNode == this->TargetVertex)
          {
          focused = true;
          break;
          }
        curNode = this->Tree->GetParent(curNode);
        }

      // Increase performance by checking if focused, onscreen, and a decent screen area.
      if (focused && onScreenX && onScreenY && screenArea / fullScreenArea > 0.01)
        {
        vtkStdString label = labelArr->GetVariantValue(i).ToString();

        // Check min font size before full search for performance
        float bounds[4];
        painter->GetTextProp()->SetFontSize(minFontSize);
        painter->ComputeStringBounds(label, bounds);
        if (bounds[2] > boxWidth)
          {
          continue;
          }

        // Do a binary search for the best font size
        int font = 16;
        for (int delta = 8; delta >= 1; delta /= 2)
          {
          painter->GetTextProp()->SetFontSize(font);
          painter->ComputeStringBounds(label, bounds);
          font += (bounds[2] > boxWidth ? -1 : 1) * delta;
          }

        // The loop will get us to an odd number between 1 and 31.
        // Do one more check to see if we switch to an even number,
        // then set the final size and bounds.
        painter->GetTextProp()->SetFontSize(font);
        painter->ComputeStringBounds(label, bounds);
        font += (bounds[2] > boxWidth) ? -1 : 0;
        font = std::min(font, maxFontSize);
        painter->GetTextProp()->SetFontSize(font);
        painter->ComputeStringBounds(label, bounds);

        if (font >= minFontSize)
          {
          float center[2] = {(bottomLeft[0] + topRight[0]) / 2 - bounds[2] / 2, (bottomLeft[1] + topRight[1]) / 2 - bounds[3] / 2};
          painter->DrawString(center[0], center[1], label);
          }
        }
      }
    }

  painter->PopMatrix();
  this->PaintChildren(painter);
  return true;
}
