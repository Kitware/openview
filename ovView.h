/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef ovView_h
#define ovView_h

#include <QObject>

#include <QStringList>

#include <vector>

class vtkContextView;
class vtkDataObject;

class ovView : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name)
  Q_PROPERTY(QStringList attributes READ attributes)
public:
  ovView(QObject *parent);
  ~ovView();

  virtual bool acceptsType(const QString &type) { return false; }
  virtual void setData(vtkDataObject *data, vtkContextView *view) = 0;
  virtual QString name() = 0;
  virtual QStringList attributes() { return QStringList(); }
  virtual void prepareForRender() { }

public slots:
  virtual QStringList attributeOptions(QString attribute) { return QStringList(); }
  virtual void setAttribute(QString attribute, QString value) { }
  virtual QString getAttribute(QString attribute) { return QString(); }

protected:
};

#endif
