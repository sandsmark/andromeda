#ifndef IVIEWFACTORY_H
#define IVIEWFACTORY_H

#include "guisystem_global.h"

#include <QtCore/QObject>
#include <QtCore/QString>

class QObject;

namespace GuiSystem {

class State;
class IView;

class GUISYSTEM_EXPORT IViewFactory : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IViewFactory)

public:
    explicit IViewFactory(QObject * parent = 0);
    virtual ~IViewFactory();

    virtual QString id() const = 0;
    virtual QString type() const { return QString(); }

    IView *view();

protected:
    virtual IView *createView() = 0;
};

} // namespace GuiSystem

#endif // IVIEWFACTORY_H
