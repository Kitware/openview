/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovContextInteractorStyle.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(ovContextInteractorStyle);

//--------------------------------------------------------------------------
ovContextInteractorStyle::ovContextInteractorStyle()
{
}

//--------------------------------------------------------------------------
ovContextInteractorStyle::~ovContextInteractorStyle()
{
}

//--------------------------------------------------------------------------
void ovContextInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void ovContextInteractorStyle::RenderNow()
{
}
