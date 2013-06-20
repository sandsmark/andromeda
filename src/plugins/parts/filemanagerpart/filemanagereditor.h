#ifndef FILEMANAGEREDITOR_H
#define FILEMANAGEREDITOR_H

#include <Parts/AbstractEditor>
#include <Parts/AbstractEditorFactory>

#include <FileManager/FileManagerWidget>

class MiniSplitter;
class QSettings;

namespace Parts {
class SharedProperties;
}

namespace FileManager {

class FileManagerDocument;
class FileManagerWidget;
class NavigationPanel;
class FileExplorerWidget;

class FileManagerEditor : public Parts::AbstractEditor
{
    Q_OBJECT

public:
    enum ViewMode {
        IconView = 0, ColumnView, TableView, TreeView, DualPane
    };

    explicit FileManagerEditor(QWidget *parent = 0);

    int currentIndex() const;
    void setCurrentIndex(int index);

    void setDocument(Parts::AbstractDocument *document);

    void restoreDefaults();
    bool restoreState(const QByteArray &state);
    QByteArray saveState() const;

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void onSelectedPathsChanged();
    void onSortingChanged();
    void onSplitterMoved(int,int);
    void openPaths(const QStringList &paths, Qt::KeyboardModifiers modifiers);
    void openNewTab();
    void openNewWindow();
    void openEditor();
    void openEditor(const QList<QUrl> &urls, const QByteArray &editor);
    void showContextMenu(const QPoint &pos);

private:
    void setupUi();
    void setupPropetries();
    void setupConnections();
    void createActions();
    void retranslateUi();
    void registerWidgetActions(FileManagerWidget *widget);
    void connectDocument(FileManagerDocument *document);

private:
    FileExplorerWidget *m_widget;

    Parts::SharedProperties *m_properties;
    QAction *m_openTabAction;
    QAction *m_openWindowAction;
    QAction *m_openEditorAction;

    bool ignoreSignals;
};

class FileManagerEditorFactory : public Parts::AbstractEditorFactory
{
public:
    explicit FileManagerEditorFactory(QObject *parent = 0);

protected:
    Parts::AbstractEditor *createEditor(QWidget *parent);
};

} // namespace FileManager

#endif // FILEMANAGEREDITOR_H