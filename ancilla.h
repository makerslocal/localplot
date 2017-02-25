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
#include <QGraphicsPolygonItem>

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
class AncillaryThread : public QObject
{
    Q_OBJECT

public:
    AncillaryThread();
    ~AncillaryThread();

public slots:
    void do_run(); // kickstart
    void do_beginPlot(const QVector<QGraphicsPolygonItem *> hpgl_items);
    void do_cancelPlot();
    int do_loadFile(const QString _filepath);

private slots:
    void do_plotNext(QPointer<QSerialPort> _port, const QVector<QGraphicsPolygonItem *> hpgl_items, int index);
    QString print(const QVector<QGraphicsPolygonItem *> hpgl_items, int index);
    void parseHPGL(QString * hpgl_text);

signals:
    void plottingStarted();
    void plottingDone();
    void plottingCancelled();
    void plottingProgress(int percentComplete);
    void plottingEta(double seconds);
    void newPolygon(QPolygonF hpglItem);
    void serialOpened();
    void serialClosed();
    void fileOpened();
    void fileClosed();
    void hpglParsingDone();
    void statusUpdate(QString _consoleStatus);

private:
    QPointer<QSerialPort> openSerial();
    void closeSerial(QPointer<QSerialPort> _device);
    bool cancelPlotFlag;
};

#endif // PLOTTER_H
