/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovWorkflowItem.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkContext2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextScene.h"
#include "vtkPen.h"
#include "vtkVectorOperators.h"

#include "ovAlgorithmItem.h"

#include <map>

vtkStandardNewMacro(ovWorkflowItem);

ovWorkflowItem::ovWorkflowItem()
{
  this->HitAlgorithm = -1;
  this->HitInputPort = -1;
  this->HitOutputPort = -1;
  this->Dragging = false;
}

void ovWorkflowItem::AddAlgorithm(vtkAlgorithm *algorithm)
{
  vtkNew<ovAlgorithmItem> item;
  item->SetAlgorithm(algorithm);
  this->AddItem(item.GetPointer());
  this->Algorithms.push_back(item.GetPointer());
}

void ovWorkflowItem::RemoveAlgorithm(vtkAlgorithm *algorithm)
{
}

bool ovWorkflowItem::MouseMoveEvent(const vtkContextMouseEvent &event)
{
  if (event.GetButton() == vtkContextMouseEvent::LEFT_BUTTON)
    {
    if (this->HitInputPort >= 0)
      {
      this->ActiveConnectionStart = event.GetPos();
      this->GetScene()->SetDirty(true);
      return true;
      }
    if (this->HitOutputPort >= 0)
      {
      this->ActiveConnectionEnd = event.GetPos();
      this->GetScene()->SetDirty(true);
      return true;
      }
    if (this->HitAlgorithm >= 0)
      {
      vtkVector2f oldPos = this->Algorithms[HitAlgorithm]->GetPosition();
      this->Algorithms[HitAlgorithm]->SetPosition(oldPos + event.GetPos() - event.GetLastPos());
      this->GetScene()->SetDirty(true);
      return true;
      }
    return true;
    }

  return false;
}

bool ovWorkflowItem::MouseEnterEvent(const vtkContextMouseEvent &vtkNotUsed(event))
{
  return true;
}

bool ovWorkflowItem::MouseLeaveEvent(const vtkContextMouseEvent &vtkNotUsed(event))
{
  return true;
}

bool ovWorkflowItem::MouseButtonPressEvent(const vtkContextMouseEvent &event)
{
  if (event.GetButton() == vtkContextMouseEvent::LEFT_BUTTON)
    {
    this->UpdateHitAlgorithm(event);
    if (this->HitInputPort >= 0)
      {
      vtkVector2f pos = this->Algorithms[this->HitAlgorithm]->GetInputPortPosition(this->HitInputPort);
      this->ActiveConnectionStart = pos;
      this->ActiveConnectionEnd = pos;
      }
    if (this->HitOutputPort >= 0)
      {
      vtkVector2f pos = this->Algorithms[this->HitAlgorithm]->GetOutputPortPosition(this->HitOutputPort);
      this->ActiveConnectionStart = pos;
      this->ActiveConnectionEnd = pos;
      }
    if (this->HitAlgorithm >= 0)
      {
      this->Dragging = true;
      return true;
      }
    }
  return false;
}

bool ovWorkflowItem::MouseButtonReleaseEvent(const vtkContextMouseEvent &event)
{
  if (this->Dragging && event.GetButton() == vtkContextMouseEvent::LEFT_BUTTON)
    {
    int inPort = this->HitInputPort;
    int outPort = this->HitOutputPort;
    int alg = this->HitAlgorithm;
    this->UpdateHitAlgorithm(event);
    if (inPort >= 0)
      {
      if (this->HitOutputPort >= 0)
        {
        // Make a connection
        this->Algorithms[alg]->GetAlgorithm()->SetInputConnection(inPort, this->Algorithms[this->HitAlgorithm]->GetAlgorithm()->GetOutputPort(this->HitOutputPort));
        }
      }
    if (outPort >= 0)
      {
      if (this->HitInputPort >= 0)
        {
        // Make a connection
        this->Algorithms[this->HitAlgorithm]->GetAlgorithm()->SetInputConnection(this->HitInputPort, this->Algorithms[alg]->GetAlgorithm()->GetOutputPort(outPort));
        }
      }
    this->HitInputPort = -1;
    this->HitOutputPort = -1;
    this->HitAlgorithm = -1;
    this->Dragging = false;
    return true;
    }
  return false;
}

void ovWorkflowItem::UpdateHitAlgorithm(const vtkContextMouseEvent &event)
{
  for (size_t i = 0; i < this->Algorithms.size(); ++i)
    {
    int inPort = this->Algorithms[i]->HitInputPort(event.GetPos());
    if (inPort >= 0)
      {
      this->HitInputPort = inPort;
      this->HitOutputPort = -1;
      this->HitAlgorithm = i;
      return;
      }
    int outPort = this->Algorithms[i]->HitOutputPort(event.GetPos());
    if (outPort >= 0)
      {
      this->HitInputPort = -1;
      this->HitOutputPort = outPort;
      this->HitAlgorithm = i;
      return;
      }
    if (this->Algorithms[i]->Hit(event))
      {
      this->HitInputPort = -1;
      this->HitOutputPort = -1;
      this->HitAlgorithm = i;
      return;
      }
    }
  this->HitInputPort = -1;
  this->HitOutputPort = -1;
  this->HitAlgorithm = -1;
}

bool ovWorkflowItem::Hit(const vtkContextMouseEvent &event)
{
  if (this->Dragging) return true;
  this->UpdateHitAlgorithm(event);
  return this->HitAlgorithm >= 0;
}

bool ovWorkflowItem::Paint(vtkContext2D *painter)
{
  painter->ApplyPen(this->ConnectionPen.GetPointer());
  if (this->Dragging && (this->HitInputPort >= 0 || this->HitOutputPort >= 0))
    {
    painter->DrawLine(this->ActiveConnectionStart[0], this->ActiveConnectionStart[1],
        this->ActiveConnectionEnd[0], this->ActiveConnectionEnd[1]);
    }
  std::map<vtkAlgorithmOutput*, ovAlgorithmItem*> outputs;
  for (size_t i = 0; i < this->Algorithms.size(); ++i)
    {
    vtkAlgorithm *alg = this->Algorithms[i]->GetAlgorithm();
    for (int j = 0; j < alg->GetNumberOfOutputPorts(); ++j)
      {
      outputs[alg->GetOutputPort(j)] = this->Algorithms[i];
      }
    }
  for (size_t i = 0; i < this->Algorithms.size(); ++i)
    {
    vtkAlgorithm *alg = this->Algorithms[i]->GetAlgorithm();
    for (int j = 0; j < alg->GetNumberOfInputPorts(); ++j)
      {
      if (alg->GetNumberOfInputConnections(j) == 0)
        {
        continue;
        }
      vtkAlgorithmOutput *output = alg->GetInputConnection(j, 0);
      if (outputs.find(output) != outputs.end())
        {
        vtkVector2f start = outputs[output]->GetOutputPortPosition(output->GetIndex());
        vtkVector2f end = this->Algorithms[i]->GetInputPortPosition(j);
        painter->DrawLine(start[0], start[1], end[0], end[1]);
        }
      }
    }
  this->PaintChildren(painter);
  return true;
}
