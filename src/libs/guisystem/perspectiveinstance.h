#ifndef PERSPECTIVEINSTANCE_H
#define PERSPECTIVEINSTANCE_H

#include "guisystem_global.h"

#include <QtCore/QObject>

namespace GuiSystem {

class IView;
class Perspective;
class PerspectiveInstancePrivate;
class GUISYSTEM_EXPORT PerspectiveInstance : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PerspectiveInstance)
public:
    explicit PerspectiveInstance(Perspective *perspective);
    ~PerspectiveInstance();

    QList<IView *> views();

signals:

public slots:

protected:
    PerspectiveInstancePrivate *d_ptr;
};

} // namespace GuiSystem

#endif // PERSPECTIVEINSTANCE_H
