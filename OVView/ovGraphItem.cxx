#include "ovGraphItem.h"

#include "vtkAbstractArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkGraph.h"

vtkStandardNewMacro(ovGraphItem);

vtkColor4ub ovGraphItem::VertexColor(vtkIdType vertex)
{
  vtkAbstractArray *domainArr = this->GetGraph()->GetVertexData()->GetAbstractArray("domain");
  if (domainArr)
    {
    vtkStdString domain = domainArr->GetVariantValue(vertex).ToString();
    if (domain == "source")
      {
      return vtkColor4ub(128, 128, 255, 255);
      }
    else if (domain == "target")
      {
      return vtkColor4ub(255, 128, 128, 255);
      }
    }
  return vtkColor4ub(128, 128, 128, 255);
}

vtkStdString ovGraphItem::VertexTooltip(vtkIdType vertex)
{
  vtkAbstractArray *arr = this->GetGraph()->GetVertexData()->GetAbstractArray("label");
  if (arr)
    {
    return arr->GetVariantValue(vertex).ToString();
    }
  return "";
}
