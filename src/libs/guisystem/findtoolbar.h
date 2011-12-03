#ifndef FINDTOOLBAR_H
#define FINDTOOLBAR_H

#include <QtGui/QToolBar>
#include "ifind.h"

namespace GuiSystem {

class FindToolBarPrivate;
class FindToolBar : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FindToolBar)

public:
    explicit FindToolBar(QWidget *parent = 0);
    ~FindToolBar();

    IFind *find() const;
    void setFind(IFind *find);

    IFind::FindFlags currentFlags() const;
    QString findString() const;
    QString replaceString() const;

    QSize minimumSizeHint() const { return QSize(0,0); }

private slots:
    void onFilterChanged();
    void findPrev();
    void findNext();
    void close();
    void replace();
    void replaceNext();
    void replaceAll();

private:
    void setupActions();
    void setupUi();
    void retranslateUi();
    void setupConnections();

    void updateUi();

private:
    FindToolBarPrivate *d;
};

} // namespace GuiSystem

#endif // FINDTOOLBAR_H