/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovWorkflowQuickItem_h
#define ovWorkflowQuickItem_h

#include "QVTKQuickItem.h"
#include "vtkNew.h"

class ovWorkflowItem;
class vtkContextView;

class ovWorkflowQuickItem : public QVTKQuickItem
{
  Q_OBJECT
public:
  ovWorkflowQuickItem();
  ~ovWorkflowQuickItem();

public slots:
  void addModule(const QString& name);

protected:
  virtual void init();

  vtkNew<vtkContextView> m_view;
  vtkNew<ovWorkflowItem> m_workflow;
};

#endif
