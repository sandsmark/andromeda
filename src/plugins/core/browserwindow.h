#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include "core_global.h"

#include <guisystem/mainwindow.h>

class QSettings;
class QUrl;

namespace GuiSystem {
class AbstractEditor;
class StackedContainer;
}

namespace Core {

class BrowserWindowPrivate;
class CORE_EXPORT BrowserWindow : public GuiSystem::MainWindow
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BrowserWindow)

public:
    explicit BrowserWindow(QWidget *parent = 0);
    ~BrowserWindow();

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

} // namespace Core

#endif // BROWSERWINDOW_H