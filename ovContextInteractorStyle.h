/*=========================================================================

  Program:   Visualization Toolkit
  Module:    ovContextInteractorStyle.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
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
