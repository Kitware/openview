/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovAlgorithmItem_h
#define ovAlgorithmItem_h

#include "vtkContextItem.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkVector.h"
#include "vtkStdString.h"
#include "vtkTextProperty.h"

class vtkAlgorithm;
class vtkBrush;
class vtkPen;

class ovAlgorithmItem : public vtkContextItem
{
public:
  static ovAlgorithmItem *New();
  vtkTypeMacro(ovAlgorithmItem, vtkContextItem);

  vtkGetObjectMacro(Algorithm, vtkAlgorithm);
  virtual void SetAlgorithm(vtkAlgorithm *algorithm);

  virtual bool Paint(vtkContext2D *painter);

  virtual bool Hit(const vtkContextMouseEvent &mouse);
  virtual int HitInputPort(const vtkVector2f &pos);
  virtual int HitOutputPort(const vtkVector2f &pos);

  virtual void SetPosition(const vtkVector2f &pos);
  virtual vtkVector2f GetPosition();

  virtual vtkVector2f GetInputPortPosition(int i);
  virtual vtkVector2f GetOutputPortPosition(int i);

  virtual void SetName(const vtkStdString& name) { this->Name = name; }
  virtual vtkStdString GetName() { return this->Name; }

protected:
  ovAlgorithmItem();
  ~ovAlgorithmItem();

  vtkAlgorithm *Algorithm;
  vtkVector2f Position;
  vtkVector2f Size;
  vtkNew<vtkBrush> Brush;
  vtkNew<vtkPen> Pen;
  vtkNew<vtkBrush> PortBrush;
  vtkNew<vtkTextProperty> TextProperty;
  vtkStdString Name;

private:
  ovAlgorithmItem(const ovAlgorithmItem&); // Not implemented
  void operator=(const ovAlgorithmItem&); // Not implemented
};

#endif
