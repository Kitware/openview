#ifndef ovViewPlugin_h
#define ovViewPlugin_h

#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include <QQmlComponent>

#include <iostream>

#include "ovView.h"

class ovViewPlugin : public QQmlExtensionPlugin
{
  Q_OBJECT
public:
  void registerTypes(const char *uri)
    {
    std::cerr << "uri: " << uri << std::endl;
    //Q_ASSERT(uri == QLatin1String(""));
    qmlRegisterType<ovView>(uri, 1, 0, "OVView");
    }
};

#endif
