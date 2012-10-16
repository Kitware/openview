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
#include "vtkContextScene.h"
#include "vtkDataSetAttributes.h"
#include "vtkFloatArray.h"
#include "vtkLookupTable.h"
#include "vtkPen.h"
#include "vtkTextProperty.h"
#include "vtkTransform2D.h"
#include "vtkTree.h"
#include "vtkTreeDFSIterator.h"

vtkStandardNewMacro(ovTreemapItem);

ovTreemapItem::ovTreemapItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
}

void ovTreemapItem::InitializeColorLookup()
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

void ovTreemapItem::SetTree(vtkTree *tree)
{
  this->Tree->ShallowCopy(tree);
  this->InitializeColorLookup();
}

void ovTreemapItem::SetColorArray(const std::string &name)
{
  this->ColorArray = name;
  this->InitializeColorLookup();
}

bool ovTreemapItem::Paint(vtkContext2D *painter)
{
  painter->GetPen()->SetColor(0, 0, 0);
  painter->GetBrush()->SetColor(128, 128, 128);
  vtkFloatArray *area = vtkFloatArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray("area"));
  painter->PushMatrix();
  vtkNew<vtkTransform2D> transform;
  transform->Scale(this->GetScene()->GetSceneWidth(), this->GetScene()->GetSceneHeight());
  painter->AppendTransform(transform.GetPointer());

  vtkNew<vtkTreeDFSIterator> it;
  it->SetTree(this->Tree.GetPointer());
  it->SetMode(vtkTreeDFSIterator::FINISH);
  it->SetStartVertex(this->Tree->GetRoot());
  vtkDataArray *arr = vtkDataArray::SafeDownCast(this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str()));
  vtkAbstractArray *labelArr = this->Tree->GetVertexData()->GetAbstractArray(this->LabelArray.c_str());
  painter->GetTextProp()->SetFontSize(20);
  painter->GetTextProp()->SetOpacity(0.75);
  painter->GetTextProp()->SetColor(0, 0, 0);
  while (it->HasNext())
    {
    vtkIdType i = it->Next();
    float *pt = area->GetPointer(4*i);
    if (!this->Tree->IsLeaf(i))
      {
      painter->GetBrush()->SetOpacityF(0.0f);
      float width = std::max(2.0f, 6.0f - 2.0f*this->Tree->GetLevel(i));
      painter->GetPen()->SetWidth(width);
      painter->GetPen()->SetOpacityF(1.0f);
      if (labelArr)
        {
        vtkStdString label = labelArr->GetVariantValue(i).ToString();
        float bounds[4];
        painter->ComputeStringBounds(label, bounds);
        if (bounds[2] > 0.0f && bounds[3] > 0.0f)
          {
          float center[2] = {(pt[0]+pt[1])/2 - bounds[2]/2, (pt[2]+pt[3])/2 - bounds[3]/2};
          painter->DrawString(center[0], center[1], label);
          }
        }
      }
    else
      {
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
      }
    painter->DrawRect(pt[0], pt[2], (pt[1]-pt[0]), (pt[3]-pt[2]));
    }
  painter->PopMatrix();
  return true;
}
