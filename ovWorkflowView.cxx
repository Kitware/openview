/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "ovWorkflowView.h"

#include "ovGraphItem.h"
#include "ovViewQuickItem.h"
#include "ovWorkflowItem.h"

#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkDelimitedTextReader.h"
#include "vtkStringToCategory.h"

ovWorkflowView::ovWorkflowView(QObject *parent) : ovView(parent)
{
}

ovWorkflowView::~ovWorkflowView()
{
}

bool ovWorkflowView::acceptsType(const QString &type)
{
  return (type == "vtkTable");
}

void ovWorkflowView::setData(vtkDataObject *data, vtkContextView *view)
{
  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  view->GetScene()->AddItem(trans.GetPointer());

  trans->AddItem(this->m_item.GetPointer());

  vtkNew<vtkDelimitedTextReader> reader;
  this->m_item->AddAlgorithm(reader.GetPointer());

  for (int i = 0; i < 10; ++i)
    {
    vtkNew<vtkStringToCategory> cat;
    this->m_item->AddAlgorithm(cat.GetPointer());
    }
}

QString ovWorkflowView::name()
{
  return "WORKFLOW";
}

QStringList ovWorkflowView::attributes()
{
  return QStringList();
}

QStringList ovWorkflowView::attributeOptions(QString attribute)
{
  return QStringList();
}

void ovWorkflowView::setAttribute(QString attribute, QString value)
{
}

QString ovWorkflowView::getAttribute(QString attribute)
{
  return QString();
}

void ovWorkflowView::prepareForRender()
{
}
