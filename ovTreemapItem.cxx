/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovTreemapItem.h"

#include "vtkAbstractArray.h"
#include "vtkColorSeries.h"
#include "vtkContext2D.h"
#include "vtkContextScene.h"
#include "vtkDataSetAttributes.h"
#include "vtkLookupTable.h"
#include "vtkTextProperty.h"
#include "vtkTransform2D.h"
#include "vtkTree.h"

vtkStandardNewMacro(ovTreemapItem);

ovTreemapItem::ovTreemapItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
}

void ovTreemapItem::InitializeColorLookup()
{
  this->ColorLookup->ResetAnnotations();
  vtkAbstractArray *arr = this->Tree->GetVertexData()->GetAbstractArray(this->ColorArray.c_str());
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
}
