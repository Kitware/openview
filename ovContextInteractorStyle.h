/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
// .NAME ovContextInteractorStyle - A context interator style that deactivates rendering
//
// .SECTION Description
// OpenView requires a context interactor style that does not cause rendering,
// since this will end up rendering at the wrong time and from the wrong thread.
// Instead, OpenView drives its own render loop.

#ifndef __ovContextInteractorStyle_h
#define __ovContextInteractorStyle_h

#include "vtkContextInteractorStyle.h"

class ovContextInteractorStyle : public vtkContextInteractorStyle
{
public:
  static ovContextInteractorStyle *New();
  vtkTypeMacro(ovContextInteractorStyle, vtkContextInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  ovContextInteractorStyle();
  ~ovContextInteractorStyle();

  virtual void RenderNow();
  virtual void OnSceneModified();

private:
  ovContextInteractorStyle(const ovContextInteractorStyle&); // Not implemented
  void operator=(const ovContextInteractorStyle&); // Not implemented
};

#endif
