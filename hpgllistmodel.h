/**
 * HPGL List Model - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLLISTMODEL_H
#define HPGLLISTMODEL_H

#include <QtCore>
#include <QAbstractListModel>
#include <QAbstractItemModel>
#include <QGraphicsItemGroup>

#include "etc.h"

#define QMODELINDEX_KEY (1)
#define QPLOTSCENE_KEY (2)

Q_DECLARE_METATYPE(QGraphicsItemGroup*)
Q_DECLARE_METATYPE(file_uid)
Q_DECLARE_METATYPE(hpgl_file)
Q_DECLARE_METATYPE(QVector<QGraphicsPolygonItem *>)
Q_DECLARE_METATYPE(QVector<QGraphicsPolygonItem *>*)
Q_DECLARE_METATYPE(QGraphicsPolygonItem *)

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
bool operator==(const file_uid& lhs, const file_uid& rhs);

enum hpglUserRoles {
    role_first = Qt::UserRole+100, // unused
    role_filename,
    role_path,
    role_uid,
    role_name,
    role_hpgl_items,
    role_hpgl_items_group,
    role_last // unused
};

namespace std {
class hpglListModel;
}

class hpglListModel : public QAbstractListModel
{
    Q_OBJECT
    QVector<hpgl_file *> hpglData;

public:
    explicit hpglListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setGroupFlag(const QModelIndex &index, QGraphicsItem::GraphicsItemFlag flag, bool flagValue);
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
//    Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // HPGLLISTMODEL_H
