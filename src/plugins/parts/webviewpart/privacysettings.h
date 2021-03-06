#ifndef PRIVACYSETTINGS_H
#define PRIVACYSETTINGS_H

#include <QtCore/qglobal.h>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

class CookieJar;
class QSettings;
class QWebSettings;

namespace Ui {
class PrivacySettingsWidget;
}

class PrivacySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrivacySettingsWidget(QWidget *parent = 0);
    ~PrivacySettingsWidget();

    CookieJar *cookieJar() const;
    void setCookieJar(CookieJar *cookieJar);

public slots:
    void setBlockPopupWindows(bool yes);
    void setJavascriptEnabled(bool yes);
    void setImagesEnabled(bool yes);
    void setLocalStorageEnabled(bool yes);
    void setPluginsEnabled(bool yes);

    void setAcceptPolicy(int policy);
    void setKeepPolicy(int policy);
    void setSessionLength(int index);
    void setFilterTrackingCookies(bool filter);

    void showCookies();
    void showExceptions();

private:
    void loadSettings();
    void loadCookieSettings();

private:
    Ui::PrivacySettingsWidget *ui;
    QWebSettings *m_webSettings;
    CookieJar *m_cookieJar;
};

#include <Parts/SettingsPage>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QFileIconProvider>
#else
#include <QtGui/QFileIconProvider>
#endif

#include "webviewplugin.h"

namespace WebView {

class PrivacySettingsPage : public Parts::SettingsPage
{
    Q_OBJECT

public:
    explicit PrivacySettingsPage(QObject *parent = 0) :
        SettingsPage(QLatin1String("Privacy"), QLatin1String("WebView"), parent)
    {}

    QString name() const { return tr("Privacy"); }
    QIcon icon() const { return QIcon(); }

    QString categoryName() const { return tr("Web View"); }
    QIcon categoryIcon() const { return QFileIconProvider().icon(QFileIconProvider::Network); }

    QWidget *createPage(QWidget *parent)
    {
        PrivacySettingsWidget *widget = new PrivacySettingsWidget(parent);
        widget->setCookieJar(WebViewPlugin::instance()->cookieJar());
        return widget;
    }
};

} // namespace WebView

#endif // PRIVACYSETTINGS_H
