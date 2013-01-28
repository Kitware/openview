/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovWorkflowQuickItem.h"

#include "ovContextInteractorStyle.h"
#include "ovWorkflowItem.h"

#include "vtkAbstractArray.h"
#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkDelimitedTextReader.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkNewickTreeReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStringToCategory.h"
#include "vtkTable.h"
#include "vtkTableReader.h"
#include "vtkVariant.h"

ovWorkflowQuickItem::ovWorkflowQuickItem()
{
  m_connect->Connect(
        m_workflow.GetPointer(),
        vtkCommand::SelectionChangedEvent,
        this,
        SLOT(workflowSelectionChanged(vtkObject*,unsigned long,void*,void*)));
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
}

void ovWorkflowQuickItem::openFile(const QString &url)
{
  this->m_viewLock.lock();
  QString fileName = QUrl(url).toLocalFile();
  vtkSmartPointer<vtkAlgorithm> algorithm;
  if (fileName.endsWith(".vtk"))
    {
    vtkSmartPointer<vtkTableReader> reader;
    reader->SetFileName(fileName.toUtf8());
    reader->Update();
    algorithm = reader.GetPointer();
    }
  else if (fileName.endsWith(".tre"))
    {
    vtkNew<vtkNewickTreeReader> reader;
    reader->SetFileName(fileName.toUtf8());
    reader->Update();
    algorithm = reader.GetPointer();
    }
  else // delimited text
    {
    vtkNew<vtkDelimitedTextReader> reader;
    reader->SetFileName(fileName.toUtf8());
    reader->SetHaveHeaders(true);
    if (fileName.endsWith(".tab") || fileName.endsWith(".tsv"))
      {
      reader->SetFieldDelimiterCharacters("\t");
      }
    reader->Update();
    vtkTable *table = reader->GetOutput();

    // Figure out if it really has headers
    // Are the column names contained in their own columns?
    int matchCount = 0;
    for (vtkIdType col = 0; col < table->GetNumberOfColumns(); ++col)
      {
      vtkAbstractArray *column = table->GetColumn(col);
      vtkVariant name(column->GetName());
      if (column->LookupValue(name) >= 0)
        {
        ++matchCount;
        }
      }
    if (matchCount > 0)
      {
      reader->SetHaveHeaders(false);
      reader->Update();
      }
    algorithm = reader.GetPointer();
    }
  m_workflow->AddAlgorithm(algorithm.GetPointer(), algorithm->GetClassName());
  this->m_viewLock.unlock();
}

void ovWorkflowQuickItem::addModule(const QString& name)
{
  vtkNew<vtkStringToCategory> cat;
  m_workflow->AddAlgorithm(cat.GetPointer(), name.toStdString());
}

void ovWorkflowQuickItem::workflowSelectionChanged(vtkObject *object, unsigned long event, void *clientData, void *callData)
{
  cerr << "in workflowSelectionChanged" << endl;
  m_viewItem->setData(static_cast<vtkDataObject*>(callData));
}
