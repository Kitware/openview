#ifndef ovGraphItem_h
#define ovGraphItem_h

#include "vtkGraphItem.h"

class ovGraphItem : public vtkGraphItem
{
public:
  static ovGraphItem *New();
  vtkTypeMacro(ovGraphItem, vtkGraphItem);

protected:
  ovGraphItem() {}
  ~ovGraphItem() {}

  virtual vtkStdString VertexTooltip(vtkIdType vertex);
  virtual vtkColor4ub VertexColor(vtkIdType vertex);

private:
  ovGraphItem(const ovGraphItem&); // Not implemented
  void operator=(const ovGraphItem&); // Not implemented
};

#endif
