#ifndef ovGraphItem_h
#define ovGraphItem_h

#include "vtkGraphItem.h"
#include "vtkNew.h"

#include "vtkColorSeries.h"
#include "vtkLookupTable.h"

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

protected:
  ovGraphItem();
  ~ovGraphItem() {}

  void InitializeColorLookup();

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

  std::string ColorArray;
  std::string LabelArray;
  std::string TooltipArray;

  vtkNew<vtkColorSeries> ColorSeries;
  vtkNew<vtkLookupTable> ColorLookup;

private:
  ovGraphItem(const ovGraphItem&); // Not implemented
  void operator=(const ovGraphItem&); // Not implemented
};

#endif
