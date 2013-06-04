/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovTreemapItem_h
#define ovTreemapItem_h

#include "vtkContextItem.h"
#include "vtkNew.h"

#include "vtkColorSeries.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkTransform2D.h"

#include <map>

class vtkTooltipItem;
class vtkTree;

class ovTreemapItem : public vtkContextItem
{
public:
  static ovTreemapItem *New();
  vtkTypeMacro(ovTreemapItem, vtkContextItem);

  std::string GetColorArray() { return this->ColorArray; }
  void SetColorArray(const std::string &arr);

  std::string GetLabelArray() { return this->LabelArray; }
  void SetLabelArray(const std::string &arr) { this->LabelArray = arr; }

  std::string GetTooltipArray() { return this->TooltipArray; }
  void SetTooltipArray(const std::string &arr) { this->TooltipArray = arr; }

  virtual void SetTree(vtkTree *tree);

  virtual bool Paint(vtkContext2D *painter);

protected:
  ovTreemapItem();
  ~ovTreemapItem() {}

  // Description:
  // Return index of hit vertex, or -1 if no hit.
  virtual vtkIdType HitVertex(const vtkVector2f &pos);

  // Description:
  // Handle mouse events.
  virtual bool MouseMoveEvent(const vtkContextMouseEvent &event);
  virtual bool MouseLeaveEvent(const vtkContextMouseEvent &event);
  virtual bool MouseEnterEvent(const vtkContextMouseEvent &event);
  virtual bool MouseButtonReleaseEvent(const vtkContextMouseEvent &event);

  // Description:
  // Whether this graph item is hit.
  virtual bool Hit(const vtkContextMouseEvent &event);

  // Description:
  // Change the position of the tooltip based on the vertex hovered.
  virtual void PlaceTooltip(vtkIdType v, const vtkVector2f &pos);

  std::string VertexTooltip(vtkIdType vertex);
  void InitializeColorLookup();

  std::string ColorArray;
  std::string LabelArray;
  std::string TooltipArray;

  vtkIdType TargetVertex;
  vtkNew<vtkTransform2D> Scale;
  vtkNew<vtkTransform2D> Translate;

  vtkNew<vtkColorSeries> ColorSeries;
  vtkNew<vtkLookupTable> ColorLookup;
  vtkNew<vtkTree> Tree;
  vtkNew<vtkTooltipItem> Tooltip;
  std::map<vtkVariant, vtkIdType> ColorIndexMap;

private:
  ovTreemapItem(const ovTreemapItem&); // Not implemented
  void operator=(const ovTreemapItem&); // Not implemented
};

#endif
