#include "actionmanager.h"

#include "command.h"
#include "commandcontainer.h"

#include <QtCore/QHash>
#include <QtGui/QApplication>
#include <QtGui/QAction>

namespace GuiSystem {

class ActionManagerPrivate
{
public:
    QHash<QString, QObject *> objects;
};

} // namespace GuiSystem

using namespace GuiSystem;

/*!
    \class ActionManager

    \brief The ActionManager class is a container for commands

    It allows to implicitly register Commands and CommandContainers and recieve them by their id.
    Commands and CommandContainers are registered when created using their constructor. Once registered,
    they can by retreived using ActionManager::command and ActionManager::container methods.
*/

/*!
    Creates a ActionManager with the given \a parent. You are not suppose to construct ActionManager
    manually, use ActionManager::instance instead.
*/
ActionManager::ActionManager(QObject *parent) :
    QObject(parent),
    d_ptr(new ActionManagerPrivate)
{
    qApp->installEventFilter(this);
}

/*!
    \brief Destructor
*/
ActionManager::~ActionManager()
{
    delete d_ptr;
}

Q_GLOBAL_STATIC(ActionManager, get_instance)
/*!
    Returns pointer to static instance of an ActionManager.
    Ths method should only be called after QApplication is constructed.
*/
ActionManager *ActionManager::instance()
{
    return get_instance();
}

/*!
    \internal
*/
void ActionManager::registerCommand(Command *cmd)
{
    Q_D(ActionManager);

    d->objects.insert(cmd->id(), cmd);
    if (!cmd->parent())
        cmd->setParent(this);
}

/*!
    \internal
*/
void ActionManager::registerContainer(CommandContainer *c)
{
    Q_D(ActionManager);

    d->objects.insert(c->id(), c);
    if (!c->parent())
        c->setParent(this);
}

/*!
    Returns pointer to Command previously created with \id
*/
Command * ActionManager::command(const QString &id)
{
    return qobject_cast<Command *>(d_func()->objects.value(id));
}

/*!
    Returns pointer to CommandContainer previously created with \id
*/
CommandContainer * ActionManager::container(const QString &id)
{
    return qobject_cast<CommandContainer *>(d_func()->objects.value(id));
}

/*!
    \internal
*/
void ActionManager::unregisterCommand(Command *cmd)
{
    Q_D(ActionManager);

    d->objects.remove(cmd->id());
    if (cmd->parent() == this)
        cmd->deleteLater();
}

/*!
    \internal
*/
void ActionManager::unregisterContainer(CommandContainer *c)
{
    Q_D(ActionManager);

    d->objects.remove(c->id());
    if (c->parent() == this)
        c->deleteLater();
}

/*!
    \internal

    Monitors changing of widgets in application to keep Commands up to date with visible widgets.
*/
bool ActionManager::eventFilter(QObject *o, QEvent *e)
{
    if (o->isWidgetType()) {
        QWidget *w = static_cast<QWidget*>(o);

        if (e->type() == QEvent::Show) {
            if (w->isActiveWindow()) {
                setActionsEnabled(w, true, Command::WindowCommand);
            }
        } else if (e->type() == QEvent::Hide) {
            if (w->isActiveWindow()) {
                setActionsEnabled(w, false, Command::WindowCommand);
            }
        } else if (e->type() == QEvent::FocusIn) {
            while (w) {
                setActionsEnabled(w, true, Command::WidgetCommand);
                w = w->parentWidget();
            }
        } else if (e->type() == QEvent::FocusOut) {
            while (w) {
                setActionsEnabled(w, false, Command::WidgetCommand);
                w = w->parentWidget();
            }
        } else if (e->type() == QEvent::ActivationChange) {
            bool enable = w->isActiveWindow();
            QWidgetList widgets = w->findChildren<QWidget*>();
            widgets.prepend(w);
            foreach (QWidget *w, widgets) {
                setActionsEnabled(w, enable, Command::WindowCommand);
            }
        }
    }
    return QObject::eventFilter(o, e);
}

/*!
    \internal

    Connects actions for widget \w to Commands with id equal to actions' objectNames.
*/
void ActionManager::setActionsEnabled(QWidget *w, bool enabled, Command::CommandContext context)
{
    Q_D(ActionManager);

    foreach (QAction *action, w->actions()) {
        QString id = action->objectName();
        if (!id.isEmpty()) {
            Command *c = qobject_cast<Command *>(d->objects.value(id));
            if (c && c->context() == context) {
                c->setRealAction(enabled ? action : 0);
            }
        }
    }
}
