/**
 * Ancilla - worker thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef PLOTTER_H
#define PLOTTER_H

#include <QtCore>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPolygonF>

#include "settings.h"
#include "etc.h"

namespace std {
class AncillaryThread;
}

/**
 * @brief The AncillaryThread class
 * This class/thread is responsible for managing:
 * - File loading and parsing
 * - Calculations (estimated plot time, plot pathing)
 * - Serial port connections
 * - HPGL plotting/output
 */
class AncillaryThread : public QThread
{
    Q_OBJECT

public:
    AncillaryThread();
    ~AncillaryThread();

public slots:
//    void do_loadFile(QString const * const filePath);
    void do_beginPlot();
    void do_cancelPlot();
    void do_plotNext();
    int load_file(const QString _filepath);
    void parseHPGL(QString * hpgl_text);

private slots:
    void do_openSerial();
    void do_closeSerial();

signals:
    void plottingStarted();
    void plottingDone();
    void plottingCancelled();
    void plottingProgress(int percentComplete);
    void plottingEta(double seconds);
    void newPolygon(QPolygonF hpglItem);
    void serialOpened();
    void serialClosed();

private:
    void run() override; // Reimplement from QThread
    bool cancelPlotFlag;

    // plotting
    int index_obj;
    int index_cmd;
    QString printThis;
    int cmdCount;
    double time;
};

#endif // PLOTTER_H
