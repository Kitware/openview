/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovWorkflowItem_h
#define ovWorkflowItem_h

#include "vtkContextItem.h"
#include "vtkNew.h"
#include "vtkPen.h"
#include "vtkSmartPointer.h"
#include "vtkVector.h"

#include <vector>

class vtkAlgorithm;
class ovAlgorithmItem;

class ovWorkflowItem : public vtkContextItem
{
public:
  static ovWorkflowItem *New();
  vtkTypeMacro(ovWorkflowItem, vtkContextItem);

  virtual void AddAlgorithm(vtkAlgorithm *algorithm, const vtkStdString& name);
  virtual void RemoveAlgorithm(vtkAlgorithm *algorithm);

  virtual bool Paint(vtkContext2D *painter);

protected:
  ovWorkflowItem();
  ~ovWorkflowItem() {}

  // Description:
  // Update the HitAlgorithm, HitInputPort, HitOutputPort member variables.
  virtual void UpdateHitAlgorithm(const vtkContextMouseEvent &event);

  // Description:
  // Handle mouse events.
  virtual bool MouseMoveEvent(const vtkContextMouseEvent &event);
  virtual bool MouseLeaveEvent(const vtkContextMouseEvent &event);
  virtual bool MouseEnterEvent(const vtkContextMouseEvent &event);
  virtual bool MouseButtonPressEvent(const vtkContextMouseEvent &event);
  virtual bool MouseButtonReleaseEvent(const vtkContextMouseEvent &event);

  // Description:
  // Whether this item is hit.
  virtual bool Hit(const vtkContextMouseEvent &event);

  std::vector<vtkSmartPointer<ovAlgorithmItem> > Algorithms;
  int HitAlgorithm;
  int HitInputPort;
  int HitOutputPort;
  bool Dragging;
  vtkVector2f ActiveConnectionStart;
  vtkVector2f ActiveConnectionEnd;
  vtkNew<vtkPen> ConnectionPen;

private:
  ovWorkflowItem(const ovWorkflowItem&); // Not implemented
  void operator=(const ovWorkflowItem&); // Not implemented
};

#endif
