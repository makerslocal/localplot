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
#include <QGraphicsRectItem>
#include <QMutex>
#include <QMutexLocker>
#include <QGraphicsScene>
#include <math.h>
#include <QGraphicsRectItem>

#include "settings.h"

#define QMODELINDEX_KEY (1)

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

namespace std {
class hpglListModel;
}

class hpglListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit hpglListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool dataGroup(const QPersistentModelIndex index,
                    QGraphicsItemGroup *&itemGroup);
    bool dataItemsGroup(const QPersistentModelIndex index,
                        QGraphicsItemGroup *&itemGroup, QVector<QGraphicsPolygonItem *> *&items);
    bool dataItems(const QPersistentModelIndex index,
                   QVector<QGraphicsPolygonItem *> *&items);
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setGroupFlag(const QModelIndex &index, QGraphicsItem::GraphicsItemFlag flag, bool flagValue);
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    void addPolygon(QPersistentModelIndex index, QGraphicsPolygonItem * poly);
    void constrainItems(QPointF bottomLeft, QPointF topLeft, QGraphicsRectItem *vinyl);
    bool setFileUid(const QModelIndex &index, const file_uid filename);
    void sort();
    // Item transformations
    void rotateSelectedItems(qreal rotation);
    void scaleSelectedItems(qreal x, qreal y);
    // Mutex
    void mutexLock();
    void mutexUnlock();
    bool mutexIsLocked();

public slots:
    void createCutoutBox(QPersistentModelIndex _index);
    void createCutoutBoxes();
    void removeCutoutBoxes();
    void removeCutoutBox(QPersistentModelIndex _index);
    void duplicateSelectedRows();

signals:
    void newPolygon(QPersistentModelIndex,QPolygonF);
    void newFileToScene(QPersistentModelIndex);
    void vinylLength(int);

private:
    QVector<hpgl_file *> hpglData;
    QObject * modelParent;
    QMutex mutex;
};

#endif // HPGLLISTMODEL_H
