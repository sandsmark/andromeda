#ifndef WEBVIEWPLUGINIMPL_H
#define WEBVIEWPLUGINIMPL_H

#include <iplugin.h>

class WebViewPluginImpl : public ExtensionSystem::IPlugin
{
    Q_OBJECT
public:
    WebViewPluginImpl();

    virtual bool initialize();
    virtual void shutdown();

signals:

public slots:

};

#endif // WEBVIEWPLUGINIMPL_H
