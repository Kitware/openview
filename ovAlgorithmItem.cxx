/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovAlgorithmItem.h"

#include "vtkContext2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkAlgorithm.h"
#include "vtkBrush.h"
#include "vtkPen.h"
#include "vtkVectorOperators.h"

vtkStandardNewMacro(ovAlgorithmItem);
vtkCxxSetObjectMacro(ovAlgorithmItem, Algorithm, vtkAlgorithm);

ovAlgorithmItem::ovAlgorithmItem()
{
  this->Name = "Module";
  this->Algorithm = NULL;
  this->Pen->SetColor(0, 0, 0);
  this->Pen->SetWidth(1);
  this->Brush->SetColor(31, 119, 180);
  this->PortBrush->SetColor(174, 199, 232);
  this->Position = vtkVector2f(0, 0);
  this->Size[0] = 150;
  this->Size[1] = 100;
  this->TextProperty->SetFontSize(12);
  this->TextProperty->SetJustificationToCentered();
  this->TextProperty->SetVerticalJustificationToCentered();
  this->TextProperty->SetColor(1, 1, 1);
}

ovAlgorithmItem::~ovAlgorithmItem()
{
  this->Algorithm->Delete();
}

bool ovAlgorithmItem::Hit(const vtkContextMouseEvent &mouse)
{
  vtkVector2f pos = mouse.GetPos();
  return (pos[0] >= this->Position[0] && pos[0] <= this->Position[0] + this->Size[0]
      && pos[1] >= this->Position[1] && pos[1] <= this->Position[1] + this->Size[1]);
}

int ovAlgorithmItem::HitInputPort(const vtkVector2f &pos)
{
  if (!this->Algorithm) return -1;
  int numInputs = this->Algorithm->GetNumberOfInputPorts();
  for (int i = 0; i < numInputs; ++i)
    {
    double dist = (this->GetInputPortPosition(i) - pos).Norm();
    if (dist < 10)
      {
      return i;
      }
    }
  return -1;
}

int ovAlgorithmItem::HitOutputPort(const vtkVector2f &pos)
{
  if (!this->Algorithm) return -1;
  int numOutputs = this->Algorithm->GetNumberOfOutputPorts();
  for (int i = 0; i < numOutputs; ++i)
    {
    double dist = (this->GetOutputPortPosition(i) - pos).Norm();
    if (dist < 10)
      {
      return i;
      }
    }
  return -1;
}

void ovAlgorithmItem::SetPosition(const vtkVector2f &pos)
{
  this->Position = pos;
}

vtkVector2f ovAlgorithmItem::GetPosition()
{
  return this->Position;
}

vtkVector2f ovAlgorithmItem::GetInputPortPosition(int i)
{
  int numInputs = this->Algorithm->GetNumberOfInputPorts();
  float x = static_cast<float>(i+1)*this->Size[0]/(numInputs + 1);
  return vtkVector2f(this->Position[0] + x, this->Position[1] + this->Size[1]);
}

vtkVector2f ovAlgorithmItem::GetOutputPortPosition(int i)
{
  int numOutputs = this->Algorithm->GetNumberOfOutputPorts();
  float x = static_cast<float>(i+1)*this->Size[0]/(numOutputs + 1);
  return vtkVector2f(this->Position[0] + x, this->Position[1]);
}

bool ovAlgorithmItem::Paint(vtkContext2D *painter)
{
  painter->ApplyPen(this->Pen.GetPointer());
  painter->ApplyBrush(this->Brush.GetPointer());
  painter->DrawRect(this->Position[0], this->Position[1], this->Size[0], this->Size[1]);

  painter->ApplyTextProp(this->TextProperty.GetPointer());
  painter->DrawString(this->Position[0] + this->Size[0]/2, this->Position[1] + this->Size[1]/2, this->Name.c_str());

  painter->ApplyBrush(this->PortBrush.GetPointer());
  if (this->Algorithm)
    {
    int numInputs = this->Algorithm->GetNumberOfInputPorts();
    for (int i = 0; i < numInputs; ++i)
      {
      float x = static_cast<float>(i+1)*this->Size[0]/(numInputs + 1);
      painter->DrawEllipse(this->Position[0] + x, this->Position[1] + this->Size[1], 10, 10);
      }
    int numOutputs = this->Algorithm->GetNumberOfOutputPorts();
    for (int i = 0; i < numOutputs; ++i)
      {
      float x = static_cast<float>(i+1)*this->Size[0]/(numOutputs + 1);
      painter->DrawEllipse(this->Position[0] + x, this->Position[1], 10, 10);
      }
    }
  return true;
}
