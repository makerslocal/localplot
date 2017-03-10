/**
 * ETC - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef ETC_H
#define ETC_H

#include <QtCore>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QVector>

QString timeStamp();
int get_nextInt(QString input, int * index);
double speedTranslate(int setting_speed);

// hpgl structs
struct file_uid {
    QString filename;
    QString path;
    int uid;
};

struct hpgl_file {
    file_uid name;
    QVector<QGraphicsPolygonItem *> hpgl_items;
    QGraphicsItemGroup * hpgl_items_group;
};

Q_DECLARE_METATYPE(QGraphicsItemGroup*)
Q_DECLARE_METATYPE(file_uid)
Q_DECLARE_METATYPE(hpgl_file)
Q_DECLARE_METATYPE(QVector<QGraphicsPolygonItem *>)
Q_DECLARE_METATYPE(QGraphicsPolygonItem *)


bool operator==(const file_uid& lhs, const file_uid& rhs);

#endif // ETC_H
