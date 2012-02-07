#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include "coreplugin_global.h"

#include <guisystem/mainwindow.h>

class QSettings;
class QUrl;

namespace GuiSystem {
class AbstractEditor;
class StackedContainer;
}

namespace CorePlugin {

class BrowserWindowPrivate;
class COREPLUGIN_EXPORT BrowserWindow : public GuiSystem::MainWindow
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BrowserWindow)

public:
    explicit BrowserWindow(QWidget *parent = 0);
    ~BrowserWindow();

    void restoreSession(QSettings &s);
    void saveSession(QSettings &s);

    static BrowserWindow *currentWindow();
    static QList<BrowserWindow*> windows();
    static BrowserWindow *createWindow();

public slots:
    void up();

    void openNewWindow(const QUrl &url);
    void openNewWindow(const QList<QUrl> &urls);

    void newTab();
    static void newWindow();

protected:
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);

protected:
    BrowserWindowPrivate * d_ptr;
};

} // namespace CorePlugin

#endif // BROWSERWINDOW_H