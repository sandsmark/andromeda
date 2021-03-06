#ifndef HELLOWORLPLUGIN_H
#define HELLOWORLPLUGIN_H

#include <QtCore/QUrl>
#include <ExtensionSystem/IPlugin>
#include <Parts/CommandContainer>

class QAction;
class QModelIndex;
class QUrl;

namespace Bookmarks {

class BookmarksModel;
class BookmarksDocument;

class BookmarksPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "com.arch.Andromeda.BookmarksPlugin")
#endif
public:
    explicit BookmarksPlugin();

    static BookmarksPlugin *instance();

    bool initialize();
    void shutdown();

    BookmarksDocument *sharedDocument() const;
    Bookmarks::BookmarksModel *model() const { return m_model; }

private slots:
    void onIndexTriggered(const QModelIndex &index);

    void showBookmarks();
    void addBookmark();
    void addFolder();

private:
    void createActions();
    void showBookmarkDialog(const QModelIndex &index, bool isFolder);
    void addDefaultBookmarks();

private:
    Bookmarks::BookmarksModel *m_model;
    BookmarksDocument *m_document;
};

} // namespace Bookmarks

#endif // HELLOWORLPLUGIN_H
