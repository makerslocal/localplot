/**
 * ExtEta - Estimated time to plot
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef EXTETA_H
#define EXTETA_H

#include <QtCore>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QVector>
#include <QtMath>
#include <QModelIndex>
#include <QPersistentModelIndex>

#include "settings.h"
#include "hpgllistmodel.h"

namespace std {
class ExtEta;
}

/**
 * @brief The ExtEta class
 */
class ExtEta : public QObject
{
    Q_OBJECT

public:
    ExtEta(hpglListModel * model);
    ~ExtEta();
    static double speedTranslate(int setting_speed);
    static double plotTime(const QLineF _line);
    static double plotTime(const QPolygonF _poly);
    static double lenHyp(const QPolygonF _poly);

public slots:
    void process();

signals:
    void progress(int percentComplete);
    void finished(double seconds);
    void statusUpdate(QString text, QColor textColor);

private:
    void statusUpdate(QString _consoleStatus);

    hpglListModel * hpglModel;
};

#endif // EXTETA_H
