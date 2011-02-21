#ifndef CORE_P_H
#define CORE_P_H

#include "core.h"

namespace CorePlugin
{

class CorePrivate
{
public:
//    EditorManager *editorManager;
    ExtensionSystem::QObjectPool *pool;
};

} // namespace Core

#endif // CORE_P_H
