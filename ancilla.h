/**
 * Ancilla - worker thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef PLOTTER_H
#define PLOTTER_H

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPointer>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QString>

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
    void do_beginPlot(QList<hpgl_obj> _objList);
    void do_cancelPlot();
    void do_plotNext();

signals:
    void startedPlotting();
    void donePlotting();
    void serialOpened();
    void serialClosed();
    void plottingProgress(int percent);

private:
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;
    QSettings * settings;
    volatile int state;
    QList<hpgl_obj> objList;

    // plotting
    int index_obj;
    int index_cmd;
    hpgl_obj obj;
    QString printThis;
    int cmdCount;
    double time;
};

#endif // PLOTTER_H
