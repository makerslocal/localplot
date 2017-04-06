/**
 * ExtBinPack - worker thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef EXTBINPACK_H
#define EXTBINPACK_H

#include <QtCore>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QVector>
#include <QtMath>

#include "settings.h"
#include "hpgllistmodel.h"
//#include "RectangleBinPack/ShelfBinPack.h"
#include "RectangleBinPack/MaxRectsBinPack.h"
#include "mainwindow.h"

namespace std {
class ExtBinPack;
}

class ExtBinPack : public QObject
{
    Q_OBJECT

public:
    ExtBinPack(hpglListModel * model);
    ~ExtBinPack();

public slots:
    void process();
    void cancel();

signals:
    void progress(int percent);
    void finished();
    void statusUpdate(QString text, QColor textColor);
    void packedRect(QPersistentModelIndex index, QRectF rect);

private:
    void statusUpdate(QString _consoleStatus);
    hpglListModel * hpglModel;
    bool cancelFlag;
};

#endif // EXTBINPACK_H
