/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovWorkflowView_h
#define ovWorkflowView_h

#include "ovView.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class ovWorkflowItem;
class vtkContextView;

class ovWorkflowView : public ovView
{
  Q_OBJECT
public:
  ovWorkflowView(QObject *parent);
  ~ovWorkflowView();

  virtual bool acceptsType(const QString &type);
  virtual void setData(vtkDataObject *data, vtkContextView *view);
  virtual QString name();
  virtual void prepareForRender();

  virtual QStringList attributes();
  virtual QStringList attributeOptions(QString attribute);
  virtual void setAttribute(QString attribute, QString value);
  virtual QString getAttribute(QString attribute);

protected:
  vtkNew<ovWorkflowItem> m_item;
};

#endif
