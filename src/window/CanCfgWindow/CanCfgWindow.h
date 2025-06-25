#ifndef CANCFGWINDOW_H
#define CANCFGWINDOW_H

#include <core/ConfigurableWidget.h>
#include <QList>
#include <core/Backend.h>

class Backend;

namespace Ui {
class CanCfgWindow;
}

class CanCfgWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit CanCfgWindow(QWidget *parent, Backend &backend);
    ~CanCfgWindow();
    void disableWindow(int disable);

public slots:
    void refreshInterfaces();

    void comboBoxInterface_currentIndexChanged(const QString &arg1);

    void cbEnableRes_clicked(bool checked);

    void canDevRequestMsg(QByteArray msg);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::CanCfgWindow *ui;
    Backend &_backend;

    Backend &backend();
    QTimer *_timer;
    bool _first_open;
    QList<CanInterface *> CanDevList;
};

#endif // CANCFGWINDOW_H
