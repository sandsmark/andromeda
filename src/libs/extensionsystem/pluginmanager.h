#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore/QObject>

#include "pluginspec.h"

namespace ExtensionSystem {

class PluginSpec;
class PluginManagerPrivate;
class EXTENSIONSYSTEM_EXPORT PluginManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginManager)
    Q_DECLARE_PRIVATE(PluginManager)
    Q_PROPERTY(QString pluginsFolder READ pluginsFolder WRITE setPluginsFolder)

public:
    explicit PluginManager(QObject *parent = 0);
    virtual ~PluginManager();

    static PluginManager *instance();

    void addObject(QObject * object, const QString &type = "");
    void removeObject(QObject * object);
    QObjectList objects();
    QObject * object(const QString &name);

    template <class T> T* object()
    {
        foreach (QObject * object, objects()) {
            T * t = qobject_cast<T*>(object);
            if (t)
                return t;
            return 0;
        }
    }

    QObjectList objects(const QString &name);
    template <class T> QList<T*> objects()
    {
        QList<T*> result;
        foreach (QObject * object, objects()) {
            T * t = qobject_cast<T*>(object);
            if (t)
                result.append(t);
        }
        return result;
    }

    void loadPlugins();
    void shutdown();

    QString pluginsFolder() const;
    void setPluginsFolder(const QString &name);

    QList<PluginSpec *> plugins() const;

signals:
    void pluginsChanged();
    void objectAdded(QObject *object, const QString &type);
    void objectRemoved(QObject *object);

public slots:
    void updateDirectory(const QString &);
    void updateLibrary(const QString &);

protected:
    void timerEvent(QTimerEvent *event);

    PluginManagerPrivate *d_ptr;

private:
    static PluginManager *m_instance;
};

} // namespace ExtensionSystem

#endif // PLUGINMANAGER_H
