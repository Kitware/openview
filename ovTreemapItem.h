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

  void InitializeColorLookup();

  std::string ColorArray;
  std::string LabelArray;
  std::string TooltipArray;

  vtkNew<vtkColorSeries> ColorSeries;
  vtkNew<vtkLookupTable> ColorLookup;
  vtkNew<vtkTree> Tree;

private:
  ovTreemapItem(const ovTreemapItem&); // Not implemented
  void operator=(const ovTreemapItem&); // Not implemented
};

#endif
