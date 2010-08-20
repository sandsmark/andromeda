#ifndef CORE_H
#define CORE_H

#include "icore.h"

namespace Core
{

class CorePrivate;
class Core : public ICore
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Core);
    Q_DISABLE_COPY(Core);

public:
    explicit Core(QObject *parent = 0);

    virtual RegistrationSystem::RegistrationManager *registrationManager();

signals:

public slots:

protected:
    CorePrivate *d_ptr;
};

} // namespace Core

#endif // CORE_H
