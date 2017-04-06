/**
 * ExtLoadFile - worker thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef EXTLOADFILE_H
#define EXTLOADFILE_H

#include <QtCore>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QVector>
#include <QtMath>
#include <QProcess>

#include "settings.h"
#include "hpgllistmodel.h"

namespace std {
class ExtLoadFile;
}

class ExtLoadFile : public QObject
{
    Q_OBJECT

public:
    ExtLoadFile(hpglListModel * model);
    ~ExtLoadFile();

public slots:
    void process();

signals:
    void progress(int percent);
    void newPolygon(QPersistentModelIndex index, QPolygonF poly);
    void finished(QPersistentModelIndex index);
    void statusUpdate(QString text, QColor textColor);

private:
    bool parseHPGL(const QPersistentModelIndex index, QString * hpgl_text);
    QPersistentModelIndex createHpglFile(file_uid _file);
    QString importSvg(QString filePath);
    QByteArray importDxf(QString filePath);
    void svgCreatePaths(QString filePath);

    void statusUpdate(QString _consoleStatus);
    hpglListModel * hpglModel;
    QString filePath;
};

#endif // EXTLOADFILE_H
