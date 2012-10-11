/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovGraphItem.h"

#include "vtkAbstractArray.h"
#include "vtkColorSeries.h"
#include "vtkContext2D.h"
#include "vtkContextScene.h"
#include "vtkDataSetAttributes.h"
#include "vtkGraph.h"
#include "vtkLookupTable.h"
#include "vtkTextProperty.h"
#include "vtkTransform2D.h"

vtkStandardNewMacro(ovGraphItem);

ovGraphItem::ovGraphItem()
{
  this->ColorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PAIRED);
  this->ColorSeries->BuildLookupTable(this->ColorLookup.GetPointer());
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
      for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i)
        {
        this->ColorLookup->SetAnnotation(arr->GetVariantValue(i), "");
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

vtkColor4ub ovGraphItem::EdgeColor(vtkIdType vtkNotUsed(edgeIdx), vtkIdType vtkNotUsed(point))
{
  return vtkColor4ub(0, 0, 0, 64);
}

vtkColor4ub ovGraphItem::VertexColor(vtkIdType vertex)
{
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
      vtkIdType index = this->ColorLookup->GetAnnotatedValueIndex(arr->GetVariantValue(vertex));
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

vtkStdString ovGraphItem::VertexLabel(vtkIdType vertex)
{
  vtkAbstractArray *arr = this->GetGraph()->GetVertexData()->GetAbstractArray(this->LabelArray.c_str());
  if (arr)
    {
    return arr->GetVariantValue(vertex).ToString();
    }
  return "";
}

void ovGraphItem::PaintBuffers(vtkContext2D *painter)
{
  this->Superclass::PaintBuffers(painter);

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
  painter->GetTextProp()->BoldOn();
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
    int xmin = (pos.X() + bounds[0] - sceneMinX)/resolution;
    int xmax = (pos.X() + bounds[2] - sceneMinX)/resolution;
    int ymin = (pos.Y() + bounds[1] - sceneMinY)/resolution;
    int ymax = (pos.Y() + bounds[3] - sceneMinY)/resolution;
    if (xmin < 0 || xmax >= w || ymin < 0 || ymax >= h)
      {
      continue;
      }
    for (int x = xmin; x <= xmax; ++x)
      {
      for (int y = ymin; y <= ymax; ++y)
        {
        if (x >= 0 && x < w && y >= 0 && y < h && buffer[w*x + y])
          {
          overlap = true;
          break;
          }
        }
      }
    if (!overlap)
      {
      painter->DrawString(pos.X(), pos.Y(), label);
      ++numRendered;
      for (int x = xmin; x <= xmax; ++x)
        {
        for (int y = ymin; y <= ymax; ++y)
          {
          if (x >= 0 && x < w && y >= 0 && y < h)
            {
            buffer[w*x + y] = true;
            }
          }
        }
      }
    }
  //cerr << "number of labels rendered: " << numRendered << endl;
}
