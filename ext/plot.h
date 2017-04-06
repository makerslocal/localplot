/**
 * ExtPlot - worker thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef EXTPLOT_H
#define EXTPLOT_H

#include <QtCore>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QVector>
#include <QtMath>

#include "settings.h"
#include "hpgllistmodel.h"
#include "eta.h"

namespace std {
class ExtPlot;
}

class ExtPlot : public QObject
{
    Q_OBJECT

public:
    ExtPlot(hpglListModel * model);
    ExtPlot(hpglListModel * model, QRectF _perimeter);
    ~ExtPlot();

public slots:
    void process();
    void cancel();

private slots:
    void do_plotNext();
//    void do_jogPerimeter();
    QString print(QPolygonF hpgl_poly, QGraphicsItemGroup * itemGroup);

signals:
    void finished();
    void progress(int percent);
    void serialOpened();
    void serialClosed();
    void statusUpdate(QString text, QColor textColor);

private:
    void statusUpdate(QString _consoleStatus);

    QPointer<QSerialPort> openSerial();
    void closeSerial();
    bool cancelPlotFlag;
    bool runPerimeterFlag;
    QRectF perimeterRect;

    // plotting
    QPointer<QSerialPort> _port;
    hpglListModel * hpglModel;
    int hpglList_index, hpgl_obj_index;
    QPointF last_point;
};

#endif // EXTPLOT_H
