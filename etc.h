/**
 * ETC - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef ETC_H
#define ETC_H

#include <QtCore>
#include <QGraphicsItemGroup>

QString timeStamp();
int get_nextInt(QString input, int * index);
double speedTranslate(int setting_speed);

// hpgl structs
struct file_uid {
    QString path;
    int uid;
};

struct hpgl_file {
    file_uid name;
    QVector<QGraphicsPolygonItem *> hpgl_items;
    QGraphicsItemGroup * hpgl_items_group;
};

bool operator==(const file_uid& lhs, const file_uid& rhs);

#endif // ETC_H
