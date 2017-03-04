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
#include <QtMath>

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
    void do_beginPlot(QVector<hpgl_file *> *_hpglList);
    void do_cancelPlot();
    int do_loadFile(const file_uid _file);

private slots:
    void do_plotNext();
    QString print(QPolygonF hpgl_poly, QPointF offset);
    void parseHPGL(file_uid _file, QString * hpgl_text);

signals:
    void plottingStarted();
    void plottingDone();
    void plottingCancelled();
    void plottingProgress(int percentComplete);
    void plottingEta(double seconds);
    void newPolygon(file_uid file, QPolygonF hpglItem);
    void serialOpened();
    void serialClosed();
    void fileOpened();
    void fileClosed();
    void hpglParsingDone();
    void statusUpdate(QString _consoleStatus);

private:
    double lenHyp(const QPolygonF _poly);
    double plotTime(const QLineF _line);
    double plotTime(const QPolygonF _poly);

    QPointer<QSerialPort> openSerial();
    void closeSerial();
    bool cancelPlotFlag;

    // plotting
    QPointer<QSerialPort> _port;
//    QVector<QGraphicsPolygonItem *> hpgl_items;
    QVector<hpgl_file *> * hpglList;
    int hpglList_index, hpgl_obj_index;
};

#endif // PLOTTER_H
