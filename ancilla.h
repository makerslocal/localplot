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
#include <QThread>

#include "hpgl.h"
#include "settings.h"

namespace std {
class AncillaryThread;
}

class AncillaryThread : public QThread
{
    Q_OBJECT

public:
    AncillaryThread();
    ~AncillaryThread();
    double speedTranslate(int setting_speed);

public slots:
//    void do_run();
    void do_openSerial();
    void do_closeSerial();
    void do_beginPlot(QList<hpgl> * _objList);
    void do_cancelPlot();
    void do_plotNext(QList<hpgl> *_objList);

signals:
    void startedPlotting();
    void donePlotting();
    void serialOpened();
    void serialClosed();
    void plottingProgress(int percent);

private:
    void run() override; // Reimplement from QThread
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;
    volatile int state;
    QList<hpgl> objList;

    // plotting
    int index_obj;
    int index_cmd;
    hpgl obj;
    QString printThis;
    int cmdCount;
    double time;
};

#endif // PLOTTER_H
