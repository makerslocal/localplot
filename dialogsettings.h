#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QCoreApplication>
#include <QSettings>

#include <QSerialPortInfo>
#include <QGraphicsScene>
#include <QPen>

#include "settings.h"

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = 0);
    ~DialogSettings();

public slots:
    void do_refreshSerialList();
    void do_updateUi();
    void do_drawDemoView();
    void do_updatePens();
    void do_saveAndClose();

private:
    Ui::DialogSettings *ui;
    QSettings * settings;
    QSerialPortInfo serialPorts;
    QGraphicsScene penDownDemoScene;
    QGraphicsScene penUpDemoScene;
    QPen downPen;
    QPen upPen;
};

#endif // DIALOGSETTINGS_H
