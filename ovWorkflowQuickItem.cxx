/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovWorkflowQuickItem.h"

#include "ovContextInteractorStyle.h"
#include "ovWorkflowItem.h"

#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkDelimitedTextReader.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStringToCategory.h"

ovWorkflowQuickItem::ovWorkflowQuickItem()
{
}

ovWorkflowQuickItem::~ovWorkflowQuickItem()
{
}

void ovWorkflowQuickItem::init()
{
  this->GetRenderWindow()->SetPolygonSmoothing(true);
  m_view->SetRenderWindow(this->GetRenderWindow());
  vtkNew<ovContextInteractorStyle> style;
  style->SetScene(m_view->GetScene());
  m_view->GetInteractor()->SetInteractorStyle(style.GetPointer());

  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  m_view->GetScene()->AddItem(trans.GetPointer());
  m_view->GetRenderer()->SetBackground(0.9, 0.9, 0.9);

  trans->AddItem(m_workflow.GetPointer());

  vtkNew<vtkDelimitedTextReader> reader;
  m_workflow->AddAlgorithm(reader.GetPointer(), "Reader");
}

void ovWorkflowQuickItem::addModule(const QString& name)
{
  vtkNew<vtkStringToCategory> cat;
  m_workflow->AddAlgorithm(cat.GetPointer(), name.toStdString());
}
