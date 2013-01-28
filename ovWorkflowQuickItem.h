/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovWorkflowQuickItem_h
#define ovWorkflowQuickItem_h

#include "QVTKQuickItem.h"
#include "vtkNew.h"
#include "vtkEventQtSlotConnect.h"
#include "ovViewQuickItem.h"

class ovWorkflowItem;
class vtkContextView;

class ovWorkflowQuickItem : public QVTKQuickItem
{
  Q_OBJECT
  Q_PROPERTY(ovViewQuickItem* viewItem READ viewItem WRITE setViewItem)
public:
  ovWorkflowQuickItem();
  ~ovWorkflowQuickItem();

  void setViewItem(ovViewQuickItem *item) { cerr << "setting view item: " << item << endl; m_viewItem = item; }
  ovViewQuickItem *viewItem() { return m_viewItem; }

public slots:
  void openFile(const QString &url);
  void addModule(const QString &name);

private slots:
  void workflowSelectionChanged(vtkObject *object, unsigned long event, void *clientData, void *callData);

protected:
  virtual void init();

  vtkNew<vtkContextView> m_view;
  vtkNew<ovWorkflowItem> m_workflow;
  vtkNew<vtkEventQtSlotConnect> m_connect;
  ovViewQuickItem *m_viewItem;
};

#endif
