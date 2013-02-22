/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovGraphItem_h
#define ovGraphItem_h

#include "vtkGraphItem.h"
#include "vtkNew.h"

#include "vtkColorSeries.h"
#include "vtkIdTypeArray.h"
#include "vtkLookupTable.h"
#include "vtkSmartPointer.h"

#include <map>

class ovGraphItem : public vtkGraphItem
{
public:
  static ovGraphItem *New();
  vtkTypeMacro(ovGraphItem, vtkGraphItem);

  std::string GetColorArray() { return this->ColorArray; }
  void SetColorArray(const std::string &arr);

  std::string GetLabelArray() { return this->LabelArray; }
  void SetLabelArray(const std::string &arr) { this->LabelArray = arr; }

  std::string GetTooltipArray() { return this->TooltipArray; }
  void SetTooltipArray(const std::string &arr) { this->TooltipArray = arr; }

  virtual void SetGraph(vtkGraph *graph);

  virtual void SetFocusedVertices(vtkIdTypeArray *arr) { this->FocusedVertices = arr; }

protected:
  ovGraphItem();
  ~ovGraphItem() {}

  void InitializeColorLookup();

  int DistanceFromFocus(vtkIdType vertex);

  // Description:
  // Efficiently draws the contents of the buffers built in RebuildBuffers.
  // This occurs once per frame.
  virtual void PaintBuffers(vtkContext2D *painter);

  // Description:
  // Returns the label for each vertex. Override in a subclass to change the tooltip
  // text.
  virtual vtkStdString VertexLabel(vtkIdType vertex);

  virtual vtkStdString VertexTooltip(vtkIdType vertex);
  virtual vtkColor4ub VertexColor(vtkIdType vertex);
  virtual vtkColor4ub EdgeColor(vtkIdType edgeIdx, vtkIdType point);
  virtual float EdgeWidth(vtkIdType edgeIdx, vtkIdType point);

  virtual bool MouseMoveEvent(const vtkContextMouseEvent &event);
  virtual bool MouseButtonPressEvent(const vtkContextMouseEvent &event);
  virtual bool Hit(const vtkContextMouseEvent &event);

  std::string ColorArray;
  std::string LabelArray;
  std::string TooltipArray;

  vtkNew<vtkColorSeries> ColorSeries;
  vtkNew<vtkLookupTable> ColorLookup;

  vtkSmartPointer<vtkIdTypeArray> FocusedVertices;

  vtkVector2f MouseLocation;

  std::map<vtkVariant, vtkIdType> ColorIndexMap;

private:
  ovGraphItem(const ovGraphItem&); // Not implemented
  void operator=(const ovGraphItem&); // Not implemented
};

#endif
