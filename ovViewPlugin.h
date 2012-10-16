#ifndef ovViewPlugin_h
#define ovViewPlugin_h

#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include <QQmlComponent>

#include <iostream>

#include "ovViewQuickItem.h"

class ovViewPlugin : public QQmlExtensionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ovviewplugin")
public:
  void registerTypes(const char *uri)
    {
    //Q_ASSERT(uri == QLatin1String(""));
    qmlRegisterType<ovViewQuickItem>(uri, 1, 0, "OVView");
    }
};

#endif
