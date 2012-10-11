/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
// .NAME ovContextInteractorStyle - An interactor for chart views
// It observes the user events (mouse events) and propagates them
// to the scene. If the scene doesn't eat the event, it is propagated
// to the interactor style superclass.
//
// .SECTION Description

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

private:
  ovContextInteractorStyle(const ovContextInteractorStyle&); // Not implemented
  void operator=(const ovContextInteractorStyle&); // Not implemented
};

#endif
