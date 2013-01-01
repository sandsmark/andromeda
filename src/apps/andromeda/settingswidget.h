#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QtGui/QWidget>

namespace Ui {
class SettingsWidget;
}

class QModelIndex;

namespace Andromeda {

class SettingsModel;

class SettingsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsWidget)
public:
    explicit SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();

    SettingsModel *model() const;
    void setModel(SettingsModel *model);

private slots:
    void remove();
    void removeAll();
    void refresh();

private:
    QModelIndex selectedRow() const;

private:
    Ui::SettingsWidget *ui;
    SettingsModel *m_model;
    QAction *m_closeAction;
};

} // namespace Andromeda

#endif // SETTINGSWIDGET_H
