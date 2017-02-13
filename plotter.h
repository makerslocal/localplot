#ifndef PLOTTER_H
#define PLOTTER_H

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPointer>
#include <QSettings>
#include <QProcess>

#include "hpgl_obj.h"
#include "settings.h"

namespace std {
class Plotter;
}

class Plotter : public QObject
{
    Q_OBJECT

public:
    Plotter();
    ~Plotter();
    double speedTranslate(int setting_speed);

public slots:
    void do_run();
    void do_openSerial();
    void do_closeSerial();
    void do_plot(QList<hpgl_obj> _objList);

signals:
    void donePlotting();
    void serialOpened();
    void serialClosed();

private:
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;
    QSettings * settings;
};

#endif // PLOTTER_H
