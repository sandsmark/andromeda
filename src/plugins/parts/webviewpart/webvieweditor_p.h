#ifndef WEBVIEWEDITOR_P_H
#define WEBVIEWEDITOR_P_H

#include "webvieweditor.h"

#include <Parts/IFind>
#include <Parts/IHistory>

namespace WebView {

class WebViewFind : public Parts::IFind
{
public:
    explicit WebViewFind(WebViewEditor *editor);

    FindFlags supportedFindFlags() const;
    void resetIncrementalSearch();
    void clearResults();

    QString currentFindString() const;
    QString completedFindString() const;

    void highlightAll(const QString &txt, FindFlags findFlags);
    void findIncremental(const QString &txt, FindFlags findFlags);
    void findStep(const QString &txt, FindFlags findFlags);

private:
    WebViewEditor *m_editor;

    QString m_currentFindString;
};

class WebViewHistory : public Parts::IHistory
{
    Q_OBJECT
    Q_DISABLE_COPY(WebViewHistory)

public:
    explicit WebViewHistory(QObject *parent = 0);

    void setHistory(QWebHistory *history);

    void clear();
    int count() const;

    int currentItemIndex() const;
    void setCurrentItemIndex(int index);

    Parts::HistoryItem itemAt(int index) const;

    QByteArray store() const;
    void restore(const QByteArray &);

public slots:
    void updateCurrentItemIndex();

private:
    QWebHistory *m_history;
    int m_index;
};

} //namespace WebView

#endif // WEBVIEWEDITOR_P_H
