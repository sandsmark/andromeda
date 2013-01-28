#include "filemanagereditor.h"
#include "filemanagereditor_p.h"

#include <QtCore/QDataStream>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QUrl>

#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFileDialog>
#include <QtGui/QFileIconProvider>
#include <QtGui/QMenu>
#include <QtGui/QResizeEvent>
#include <QtGui/QToolBar>

#include <ExtensionSystem/PluginManager>
#include <GuiSystem/AbstractEditor>
#include <GuiSystem/EditorWindowFactory>
#include <GuiSystem/constants.h>
#include <Widgets/MiniSplitter>

#include <FileManager/DualPaneWidget>
#include <FileManager/FileExplorerWidget>
#include <FileManager/FileInfoDialog>
#include <FileManager/FileManagerHistory>
#include <FileManager/FileSystemManager>
#include <FileManager/FileSystemModel>
#include <FileManager/NavigationModel>
#include <FileManager/NavigationPanel>
#include <FileManager/constants.h>

#include "filemanagerdocument.h"
#include "openwitheditormenu.h"

using namespace GuiSystem;
using namespace FileManager;

FileManagerEditorHistory::FileManagerEditorHistory(QObject *parent) :
    IHistory(parent),
    m_widget(0),
    m_currentItemIndex(-1),
    m_pane(0)
{
}

void FileManagerEditorHistory::setDualPaneWidget(DualPaneWidget *widget)
{
    m_widget = widget;
    connect(m_widget, SIGNAL(activePaneChanged(DualPaneWidget::Pane)),
            SLOT(onActivePaneChanged(DualPaneWidget::Pane)));

    connect(m_widget->leftWidget()->history(), SIGNAL(currentItemIndexChanged(int)), SLOT(onLocalIndexChanged(int)));
}

void FileManagerEditorHistory::clear()
{
    // TODO: implement
    m_widget->leftWidget()->history()->clear();
    m_widget->rightWidget()->history()->clear();
}

void FileManagerEditorHistory::erase()
{
    m_indexes.clear();
    m_currentItemIndex = -1;
    m_pane = DualPaneWidget::LeftPane;
}

int FileManagerEditorHistory::count() const
{
    return m_indexes.count();
}

int FileManagerEditorHistory::currentItemIndex() const
{
    return m_currentItemIndex;
}

void FileManagerEditorHistory::setCurrentItemIndex(int index)
{
    if (m_currentItemIndex == index)
        return;

    if (index < 0 || index >= count())
        return;

    m_currentItemIndex = index;

    int localIndex = m_indexes[index];
    if (localIndex >= 0) {
        m_pane = DualPaneWidget::LeftPane;
        m_widget->setActivePane(DualPaneWidget::LeftPane);
        m_widget->leftWidget()->history()->setCurrentItemIndex(localIndex);
    } else {
        m_pane = DualPaneWidget::RightPane;
        m_widget->setDualPaneModeEnabled(true);
        m_widget->setActivePane(DualPaneWidget::RightPane);
        m_widget->rightWidget()->history()->setCurrentItemIndex(-localIndex - 2);
    }

    emit currentItemIndexChanged(m_currentItemIndex);
}

HistoryItem FileManagerEditorHistory::itemAt(int index) const
{
    if (index < 0 || index >= m_indexes.count())
        return HistoryItem();

    FileManagerHistoryItem item;
    int localIndex = m_indexes[index];
    if (localIndex >= 0) {
        item = m_widget->leftWidget()->history()->itemAt(localIndex);
    } else {
        item = m_widget->rightWidget()->history()->itemAt(-localIndex - 2);
    }
    HistoryItem result;
    result.setUrl(QUrl::fromLocalFile(item.path()));

    return result;
}

QByteArray FileManagerEditorHistory::store() const
{
    QByteArray history;
    QDataStream s(&history, QIODevice::WriteOnly);

    s << m_currentItemIndex;
    s << m_indexes;
    s << *(m_widget->leftWidget()->history());
    if (m_widget->rightWidget())
        s << *(m_widget->rightWidget()->history());

    return history;
}

void FileManagerEditorHistory::restore(const QByteArray &arr)
{
    QByteArray history(arr);
    QDataStream s(&history, QIODevice::ReadOnly);

    s >> m_currentItemIndex;
    s >> m_indexes;
    s >> *(m_widget->leftWidget()->history());
    if (m_widget->rightWidget())
        s >> *(m_widget->rightWidget()->history());

    emit currentItemIndexChanged(m_currentItemIndex);
}

void FileManagerEditorHistory::onLocalIndexChanged(int index)
{
    QObject *object = sender();
    int localIndex = m_currentItemIndex == -1 ? -1 : m_indexes[m_currentItemIndex];
    if (object == m_widget->leftWidget()->history()) {
    } else {
        if (index == 0)
            return;

        index = -index - 2;
    }

    if (localIndex == index)
        return;

    m_indexes.erase(m_indexes.begin() + m_currentItemIndex + 1, m_indexes.end());
    m_indexes.append(index);
    m_currentItemIndex++;

    emit currentItemIndexChanged(m_currentItemIndex);
}

void FileManagerEditorHistory::onActivePaneChanged(DualPaneWidget::Pane pane)
{
    if (m_pane == pane)
        return;

    m_pane = pane;

    m_indexes.erase(m_indexes.begin() + m_currentItemIndex + 1, m_indexes.end());
    if (pane == DualPaneWidget::LeftPane) {
        m_indexes.append(m_widget->leftWidget()->history()->currentItemIndex());
    } else {
        m_indexes.append(-m_widget->rightWidget()->history()->currentItemIndex() - 2);
    }
    m_currentItemIndex++;

    emit currentItemIndexChanged(m_currentItemIndex);
}

/*!
  \class FileManagerEditor
*/
FileManagerEditor::FileManagerEditor(QWidget *parent) :
    AbstractEditor(*new FileManagerDocument, parent),
    m_settings(new QSettings(this)),
    ignoreSignals(false)
{
    document()->setParent(this);
    setupUi();
    setupConnections();
    createActions();
    retranslateUi();

    connectDocument(qobject_cast<FileManagerDocument *>(document()));
}

void FileManagerEditor::setDocument(AbstractDocument *document)
{
    if (this->document() == document)
        return;

    FileManagerDocument *fmDocument = qobject_cast<FileManagerDocument *>(document);
    if (!fmDocument)
        return;

    connectDocument(fmDocument);

    AbstractEditor::setDocument(document);
}

/*!
  \internal

  Restores FileManagerEditor's default settings.
*/
void FileManagerEditor::restoreDefaults()
{
    bool showLeftPanel = true;

    showLeftPanel = m_settings->value(QLatin1String("fileManager/showLeftPanel"), true).toBool();

    bool showStatusBar = m_settings->value(QLatin1String("fileManager/showStatusBar"), false).toBool();
    QVariant value = m_settings->value(QLatin1String("fileManager/splitterState"));

    m_widget->setPanelVisible(showLeftPanel);
    m_widget->setStatusBarVisible(showStatusBar);

    if (value.isValid()) {
        m_widget->splitter()->restoreState(value.toByteArray());
    } else {
        m_widget->splitter()->setSizes(QList<int>() << 200 << 600);
    }

    DualPaneWidget *widget = m_widget->dualPane();
    widget->blockSignals(true);
    int sortOrder = m_settings->value(QLatin1String("fileManager/sortingOrder"), Qt::AscendingOrder).toInt();
    int sortColumn = m_settings->value(QLatin1String("fileManager/sortingColumn"), FileManagerWidget::NameColumn).toInt();
    int viewModeLeft = m_settings->value(QLatin1String("fileManager/viewModeLeft"), FileManagerWidget::IconView).toInt();
    bool dualPaneModeEnabled = m_settings->value(QLatin1String("fileManager/dualPaneModeEnabled"), false).toInt();
    int orientation = m_settings->value(QLatin1String("fileManager/orientation"), Qt::Horizontal).toInt();
    widget->setViewMode((FileManagerWidget::ViewMode)viewModeLeft);
    widget->setSortingOrder((Qt::SortOrder)sortOrder);
    widget->setSortingColumn((FileManagerWidget::Column)sortColumn);
    widget->setDualPaneModeEnabled(dualPaneModeEnabled);
    widget->setOrientation((Qt::Orientation)orientation);
    widget->blockSignals(false);
}

/*!
  \reimp
*/
bool FileManagerEditor::restoreState(const QByteArray &arr)
{
    QByteArray state = arr;
    QDataStream s(&state, QIODevice::ReadOnly);

    bool ok = true;
    QByteArray baseState, widgetState;

    s >> baseState;
    s >> widgetState;

    ok |= AbstractEditor::restoreState(baseState);
    // we have to block signals to not rewrite user settings
    bool bs0 = m_widget->blockSignals(true);
    bool bs1 = m_widget->splitter()->blockSignals(true);
    bool bs2 = m_widget->dualPane()->blockSignals(true);
    bool bs3 = m_widget->statusBar()->blockSignals(true);
    ok |= m_widget->restoreState(widgetState);
    m_widget->blockSignals(bs0);
    m_widget->splitter()->blockSignals(bs1);
    m_widget->dualPane()->blockSignals(bs2);
    m_widget->statusBar()->blockSignals(bs3);

    initRightPane(m_widget->dualPane()->dualPaneModeEnabled());

    return ok;
}

/*!
  \reimp
*/
QByteArray FileManagerEditor::saveState() const
{
    QByteArray state;
    QDataStream s(&state, QIODevice::WriteOnly);

    s << AbstractEditor::saveState();
    s << m_widget->saveState();

    return state;
}

/*!
  \reimp
*/
void FileManagerEditor::resizeEvent(QResizeEvent *e)
{
    m_widget->resize(e->size());
}

void FileManagerEditor::onSelectedPathsChanged()
{
    QStringList paths = m_widget->dualPane()->selectedPaths();;
    bool enabled = !paths.empty();

    m_openTabAction->setEnabled(enabled);
    m_openWindowAction->setEnabled(enabled);
    m_openEditorAction->setEnabled(enabled);
}

void FileManagerEditor::onViewModeChanged(FileManagerWidget::ViewMode mode)
{
    int pane = m_widget->dualPane()->activePane();
    if (pane == DualPaneWidget::LeftPane)
        m_settings->setValue(QLatin1String("fileManager/viewModeLeft"), mode);
    else
        m_settings->setValue(QLatin1String("fileManager/viewModeRight"), mode);
}

/*!
    \internal
*/
void FileManagerEditor::onSortingChanged()
{
    int sortOrder = m_widget->dualPane()->sortingOrder();
    int sortColumn = m_widget->dualPane()->sortingColumn();

    m_settings->setValue(QLatin1String("fileManager/sortingOrder"), sortOrder);
    m_settings->setValue(QLatin1String("fileManager/sortingColumn"), sortColumn);
}

/*!
    \internal
*/
void FileManagerEditor::onOrientationChanged(Qt::Orientation orientation)
{
    m_settings->setValue(QLatin1String("fileManager/orientation"), orientation);
}

void FileManagerEditor::onDualPaneModeChanged(bool enabled)
{
    m_settings->setValue(QLatin1String("fileManager/dualPaneModeEnabled"), enabled);
}

void FileManagerEditor::initRightPane(bool enabled)
{
    if (enabled) {
        FileManagerWidget *widget = m_widget->dualPane()->rightWidget();
        registerWidgetActions(widget);
        connect(widget->history(), SIGNAL(currentItemIndexChanged(int)),
                document()->history(), SLOT(onLocalIndexChanged(int)));
        int viewModeRight = m_settings->value(QLatin1String("fileManager/viewModeRight"), FileManagerWidget::IconView).toInt();
        widget->setViewMode((FileManagerWidget::ViewMode)viewModeRight);
    }
}

/*!
    \internal
*/
void FileManagerEditor::onPanelVisibleChanged(bool visible)
{
    m_settings->setValue(QLatin1String("fileManager/showLeftPanel"), visible);
}

/*!
    \internal
*/
void FileManagerEditor::onStatusBarVisibleChanged(bool visible)
{
    m_settings->setValue(QLatin1String("fileManager/showStatusBar"), visible);
}

/*!
    \internal
*/
void FileManagerEditor::onSplitterMoved(int, int)
{
    m_settings->setValue("fileManager/splitterState", m_widget->splitter()->saveState());
}

/*!
    \internal
*/
void FileManagerEditor::openPaths(const QStringList &paths,Qt::KeyboardModifiers modifiers)
{
    QStringList folders;
    foreach (const QString &path, paths) {
        QFileInfo info(path);
        if (info.isDir() && !info.isBundle()) {
            folders.append(path);
        } else {
#ifdef Q_OS_LINUX
            QFileInfo info(path);
            if (info.isExecutable()) {
                QProcess::startDetached(path);
                return;
            }
#endif
            // TODO: allow to open default editor instead
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    }

    if (folders.isEmpty())
        return;

    QList<QUrl> urls;
    foreach (const QString &path, folders) {
        urls.append(QUrl::fromLocalFile(path));
    }
    EditorWindowFactory *factory = EditorWindowFactory::defaultFactory();
    if (factory) {
        if (modifiers == Qt::NoModifier)
            factory->open(urls);
        else if (modifiers == (Qt::ControlModifier | Qt::AltModifier))
            factory->openNewWindow(urls);
        else if (modifiers == (Qt::ControlModifier))
            factory->openNewEditor(urls);
    }
}

/*!
    \internal
*/
void FileManagerEditor::openNewTab()
{
    QStringList paths = m_widget->dualPane()->selectedPaths();
    if (paths.isEmpty())
        return;

    QList<QUrl> urls;
    foreach (const QString &path, paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    EditorWindowFactory *factory = EditorWindowFactory::defaultFactory();
    if (factory)
        factory->openNewEditor(urls);
}

/*!
    \internal
*/
void FileManagerEditor::openNewWindow()
{
    QStringList paths = m_widget->dualPane()->selectedPaths();
    if (paths.isEmpty())
        return;

    QList<QUrl> urls;
    foreach (const QString &path, paths) {
        urls.append(QUrl::fromLocalFile(path));
    }

    EditorWindowFactory *factory = EditorWindowFactory::defaultFactory();
    if (factory)
        factory->openNewWindow(urls);
}

void FileManagerEditor::openEditor()
{
    QStringList paths = m_widget->dualPane()->selectedPaths();
    if (paths.isEmpty())
        return;

    QList<QUrl> urls;
    foreach (const QString &path, paths) {
        urls.append(QUrl::fromLocalFile(path));
    }

    EditorWindowFactory *factory = EditorWindowFactory::defaultFactory();
    if (factory)
        factory->open(urls);
}

void FileManagerEditor::openEditor(const QList<QUrl> &urls, const QByteArray &editor)
{
    EditorWindowFactory *factory = EditorWindowFactory::defaultFactory();
    if (!factory)
        return;

    factory->openEditor(urls, editor);
}

void FileManagerEditor::showContextMenu(const QPoint &pos)
{
    FileManagerWidget *widget = qobject_cast<FileManagerWidget *>(sender());
    Q_ASSERT(widget);

    QStringList paths = widget->selectedPaths();
    QMenu *menu = widget->createStandardMenu(paths);
    QList<QAction *> actions = menu->actions();

    if (!paths.isEmpty()) {
        QAction *actionBefore = actions.at(1);

        menu->insertAction(actionBefore, m_openTabAction);
        menu->insertAction(actionBefore, m_openWindowAction);

        OpenWithEditorMenu *openWithMenu = new OpenWithEditorMenu(menu);
        openWithMenu->setPaths(paths);
        connect(openWithMenu, SIGNAL(openRequested(QList<QUrl>,QByteArray)), SLOT(openEditor(QList<QUrl>,QByteArray)));

        if (!openWithMenu->isEmpty()) {
            menu->insertSeparator(actionBefore);
            menu->insertAction(actionBefore, m_openEditorAction);
            if (openWithMenu->actions().count() > 1)
                menu->insertMenu(actionBefore, openWithMenu);
        }
    }

    menu->exec(widget->mapToGlobal(pos));
    delete menu;
}

/*!
    \internal
*/
void FileManagerEditor::setupUi()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    NavigationModel *model = pm->object<NavigationModel>("navigationModel");

    m_widget = new FileExplorerWidget(model, this);
}

/*!
    \internal
*/
void FileManagerEditor::setupConnections()
{
    DualPaneWidget *widget = m_widget->dualPane();
    connect(widget, SIGNAL(selectedPathsChanged()), SLOT(onSelectedPathsChanged()));
    connect(widget, SIGNAL(openRequested(QStringList,Qt::KeyboardModifiers)),
            this, SLOT(openPaths(QStringList,Qt::KeyboardModifiers)));
    connect(widget, SIGNAL(sortingChanged()), SLOT(onSortingChanged()));
    connect(widget, SIGNAL(viewModeChanged(FileManagerWidget::ViewMode)), SLOT(onViewModeChanged(FileManagerWidget::ViewMode)));
    connect(widget, SIGNAL(orientationChanged(Qt::Orientation)), SLOT(onOrientationChanged(Qt::Orientation)));
    connect(widget, SIGNAL(dualPaneModeChanged(bool)), SLOT(onDualPaneModeChanged(bool)));
    connect(widget, SIGNAL(dualPaneModeChanged(bool)), SLOT(initRightPane(bool)));

    connect(m_widget->panel(), SIGNAL(triggered(QString)), widget, SLOT(setCurrentPath(QString)));

    connect(m_widget->splitter(), SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved(int,int)));

    connect(m_widget, SIGNAL(panelVisibleChanged(bool)), SLOT(onPanelVisibleChanged(bool)));
    connect(m_widget, SIGNAL(statusBarVisibleChanged(bool)), SLOT(onStatusBarVisibleChanged(bool)));
}

/*!
    \internal
*/
void FileManagerEditor::createActions()
{
    m_openTabAction = new QAction(this);
    m_openTabAction->setEnabled(false);
    m_openTabAction->setObjectName(Constants::Actions::OpenInTab);
    connect(m_openTabAction, SIGNAL(triggered()), SLOT(openNewTab()));
    addAction(m_openTabAction);

    m_openWindowAction = new QAction(this);
    m_openWindowAction->setEnabled(false);
    m_openWindowAction->setObjectName(Constants::Actions::OpenInWindow);
    connect(m_openWindowAction, SIGNAL(triggered()), SLOT(openNewWindow()));
    addAction(m_openWindowAction);

    m_openEditorAction = new QAction(this);
    m_openEditorAction->setEnabled(false);
    m_openEditorAction->setObjectName(Constants::Actions::OpenEditor);
    connect(m_openEditorAction, SIGNAL(triggered()), SLOT(openEditor()));
    addAction(m_openEditorAction);

    DualPaneWidget *widget = m_widget->dualPane();
    // TODO: register panes when created
    registerWidgetActions(widget->leftWidget());
}

void FileManagerEditor::retranslateUi()
{
    m_openTabAction->setText(tr("Open in new tab"));
    m_openWindowAction->setText(tr("Open in new window"));
    m_openEditorAction->setText(tr("Open in internal editor"));
    m_openEditorAction->setToolTip(tr("Opens selected files in an internal editor"));
}

void FileManagerEditor::registerWidgetActions(FileManagerWidget *widget)
{
    widget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(widget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
}

void FileManagerEditor::connectDocument(FileManagerDocument *document)
{
    Q_ASSERT(document);

    document->fmhistory()->setDualPaneWidget(m_widget->dualPane());

    connect(document, SIGNAL(currentPathChanged(QString)),
            m_widget->dualPane(), SLOT(setCurrentPath(QString)));
    connect(m_widget->dualPane(), SIGNAL(currentPathChanged(QString)),
            document, SLOT(setCurrentPath(QString)));
}

/*!
    \class FileManagerEditorFactory
*/

FileManagerEditorFactory::FileManagerEditorFactory(QObject *parent) :
    AbstractEditorFactory("FileManager", parent)
{
}

/*!
    \reimp
*/
AbstractEditor * FileManagerEditorFactory::createEditor(QWidget *parent)
{
    return new FileManagerEditor(parent);
}
