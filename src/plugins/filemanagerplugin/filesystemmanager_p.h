#ifndef FILESYSTEMMANAGER_P_H
#define FILESYSTEMMANAGER_P_H

#include "filesystemmanager.h"

#include <QtCore/QMap>

namespace FileManagerPlugin {

class FileSystemManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FileSystemManager)

public:
    FileSystemManagerPrivate(FileSystemManager *qq) : q_ptr(qq) {}

    QUndoStack *undoStack;

    QList<FileSystemManager::FileOperation> operations;
    int currentIndex;
    QMap<int, QFileCopier*> mapToCopier;

    QFileCopier *copier(int index);
    void registerCopier(QFileCopier *copier, int index);

    FileSystemManager *q_ptr;

private slots:
    void onDone();
};

} // namespace FileManagerPlugin

#endif // FILESYSTEMMANAGER_P_H