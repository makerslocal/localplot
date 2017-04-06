#include "hpgllistmodel.h"

bool operator==(const file_uid& lhs, const file_uid& rhs)
{
    return(lhs.path == rhs.path && lhs.uid == rhs.uid);
}

hpglListModel::hpglListModel(QObject *parent)
    :QAbstractListModel(parent)
{
    modelParent = parent;
    hpglData.clear();
}

QVariant hpglListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= hpglData.length() || index.row() < 0)
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return hpglData.at(index.row())->name.filename;
    }
    return(QVariant());
}

bool hpglListModel::dataGroup(const QPersistentModelIndex index, QMutex *& retLocker,
                                   QGraphicsItemGroup *& itemGroup)
{
    if (!index.isValid())
    {
        return false;
    }
    if (index.row() >= hpglData.length() || index.row() < 0)
    {
        return false;
    }

    retLocker = &(hpglData[index.row()]->mutex);
    itemGroup = hpglData.at(index.row())->hpgl_items_group;

    return true;
}

bool hpglListModel::dataItemsGroup(const QPersistentModelIndex index, QMutex *& retLocker,
                                   QGraphicsItemGroup *& itemGroup, QVector<QGraphicsPolygonItem*> *& items)
{
    if (!index.isValid())
    {
        return false;
    }
    if (index.row() >= hpglData.length() || index.row() < 0)
    {
        return false;
    }

    retLocker = &(hpglData[index.row()]->mutex);
    itemGroup = hpglData.at(index.row())->hpgl_items_group;
    items = &(hpglData.at(index.row())->hpgl_items);
    return true;
}

bool hpglListModel::dataItems(const QPersistentModelIndex index, QMutex *& retLocker, QVector<QGraphicsPolygonItem*> *& items)
{
    if (!index.isValid())
    {
        return false;
    }
    if (index.row() >= hpglData.length() || index.row() < 0)
    {
        return false;
    }

    retLocker = &(hpglData[index.row()]->mutex);
    items = &(hpglData.at(index.row())->hpgl_items);
    return true;
}

bool hpglListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QVector<int> changedRoles;

    if (!index.isValid())
    {
        return false;
    }
    if (index.row() >= hpglData.length() || index.row() < 0)
    {
        return false;
    }

    if (role == Qt::DisplayRole)
    {
        QMutexLocker locker(&(hpglData[index.row()]->mutex));
        hpglData[index.row()]->name.filename = value.toString();
        changedRoles.push_back(Qt::DisplayRole);
        emit dataChanged(index, index, changedRoles);
        return true;
    }
    return false;
}

bool hpglListModel::setFileUid(const QModelIndex &index, const file_uid filename)
{
    if (index.isValid())
    {
        QMutexLocker locker(&(hpglData[index.row()]->mutex));
        hpglData.at(index.row())->name.filename = filename.filename;
        hpglData.at(index.row())->name.path = filename.path;
        return true;
    }
    return false;
}

bool hpglListModel::setGroupFlag(const QModelIndex &index, QGraphicsItem::GraphicsItemFlag flag, bool flagValue)
{
    if (index.row() >= 0 && index.row() < hpglData.length())
    {
        QMutexLocker locker(&(hpglData[index.row()]->mutex));
        hpglData[index.row()]->hpgl_items_group->setFlag(flag, flagValue);
        return true;
    }
    return false;
}

QModelIndex hpglListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= 0 && row < hpglData.length())
    {
        return(createIndex(row, column));
    }
    return (QModelIndex());
}

int hpglListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return hpglData.length();
}

void hpglListModel::duplicateSelectedRows()
{
    QSettings settings;
    QModelIndex _index;
    QVector<QPersistentModelIndex> indexes;

    for (int i = 0; i < hpglData.length(); ++i)
    {
        _index = index(i);
        if (!hpglData[i]->mutex.tryLock())
        {
            qDebug() << "Mutex already locked, giving up.";
            return;
        }

        if (hpglData.at(i)->hpgl_items_group->isSelected())
        {
            qDebug() << "Index is selected: " << _index;
            indexes.push_back(_index);
        }

        hpglData[i]->mutex.unlock();
    }

    for (int i = 0; i < indexes.length(); ++i)
    {
        QPersistentModelIndex _index = indexes.at(i);
        QString oldName;

        oldName = data(_index, Qt::DisplayRole).toString();
        if (!hpglData[_index.row()]->mutex.tryLock())
        {
            qDebug() << "Mutex already locked, giving up.";
            return;
        }

        int newRow = hpglData.length();
        insertRow(newRow);
        QPersistentModelIndex newIndex = index(newRow);

        hpglData[_index.row()]->mutex.unlock();
        setData(newIndex, oldName, Qt::DisplayRole);
//        mutex->lock();

        int loopOffset = 0;
        if (settings.value("device/cutoutboxes", SETDEF_DEVICE_CUTOUTBOXES).toBool())
        {
            loopOffset = 1;
        }
        for (int i2 = 0; i2 < (hpglData[_index.row()]->hpgl_items.length() - loopOffset); ++i2)
        {
            QPolygonF polygon = hpglData[_index.row()]->hpgl_items.at(i2)->polygon();
//            addPolygon(newIndex, polygon);
            emit newPolygon(newIndex, polygon);
//            hpglData[newIndex.row()]->hpgl_items.at(i2)->setPos(hpglData[_index.row()]->hpgl_items.at(i2)->pos());
        }

//        mutex->unlock();
        emit newFileToScene(newIndex);
    }
}

bool hpglListModel::insertRow(int row, const QModelIndex &parent)
{
    return insertRows(row, 1, parent);
}

bool hpglListModel::removeRow(int row, const QModelIndex &parent)
{
    return removeRows(row, 1, parent);
}

bool hpglListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 0 || row < 0 || row > hpglData.length())
    {
        return false;
    }
    beginInsertRows(parent, row, (row + count - 1));
    for (int i = row; i < (row+count); ++i)
    {
        hpgl_file * newFile;
        QModelIndex index = createIndex(i, 0);
        newFile = new hpgl_file;
        QMutexLocker locker(&(newFile->mutex));
        newFile->hpgl_items_group = new QGraphicsItemGroup;
        newFile->hpgl_items_group->setData(QMODELINDEX_KEY, QPersistentModelIndex(index));
        newFile->name.filename = "NA";
        newFile->name.path = "NA";
        if (hpglData.length() == 0)
        {
            newFile->name.uid = 0;
        }
        else
        {
            newFile->name.uid = hpglData.last()->name.uid + 1;
        }
        newFile->hpgl_items.clear();
        hpglData.insert(i, newFile);
    }
    endInsertRows();
    return true;
}

bool hpglListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "removerows: " << row << count << parent << hpglData.length();
    if (count <= 0 || row < 0 || (row+count-1) >= hpglData.length())
    {
        return false;
    }
    beginRemoveRows(parent, row, (row + count - 1));
    for (int i = (row+count-1); i >= row; --i)
    {
        {
            QMutexLocker locker(&(hpglData[i]->mutex));
    //        delete hpglData[i]->hpgl_items_group;
            hpglData[i]->hpgl_items.clear();
            locker.unlock();
        }
        delete hpglData[i];
        hpglData.remove(i);
    }
    endRemoveRows();
    return true;
}

void hpglListModel::addPolygon(QPersistentModelIndex index, QGraphicsPolygonItem * poly)
{
    if (!index.isValid())
    {
        qDebug() << "Error invalid index.";
        return;
    }

    QMutexLocker locker(&(hpglData[index.row()]->mutex));
    hpglData[index.row()]->hpgl_items.push_back(poly);
    hpglData[index.row()]->hpgl_items_group->addToGroup(static_cast<QGraphicsItem*>(poly));
}

void hpglListModel::constrainItems(QPointF bottomLeft, QPointF topLeft)
{
    int modCount;
    int length = 0;

    for (int i = 0; i < hpglData.length(); ++i)
    {
        QPointF pos;
        QRectF rect;
        QGraphicsItemGroup * itemGroup;
        modCount = 0;

        if (!hpglData[i]->mutex.tryLock())
        {
            qDebug() << "Mutex already locked, giving up.";
            return;
        }

        itemGroup = hpglData.at(i)->hpgl_items_group;
        pos = itemGroup->pos();
        rect = itemGroup->sceneBoundingRect();

        if (rect.x() < bottomLeft.x())
        {
            ++modCount;
            pos.setX(pos.x() + qFabs(rect.x()));
        }

        if (rect.y() < bottomLeft.y())
        {
            ++modCount;
            pos.setY(pos.y() + qFabs(rect.y()));
        }
        else if ((rect.y() + rect.height()) >
                 topLeft.y())
        {
            ++modCount;
            pos.setY(pos.y() - ((rect.y() + rect.height()) - topLeft.y()));
        }

        if (modCount)
        {
            itemGroup->setPos(pos);
        }

        if ((rect.x() + rect.width()) > length)
        {
            length = (rect.x() + rect.width());
        }
        hpglData[i]->mutex.unlock();
    }
    emit vinylLength(length);
}

void hpglListModel::sort()
{
    bool swapped;
    do
    {
        swapped = false;
        for (int i = 1; i < hpglData.length(); ++i)
        {
            int max1, max2;
            max1 = qMax(hpglData.at(i-1)->hpgl_items_group->boundingRect().width(), hpglData.at(i-1)->hpgl_items_group->boundingRect().height());
            max2 = qMax(hpglData.at(i)->hpgl_items_group->boundingRect().width(), hpglData.at(i)->hpgl_items_group->boundingRect().height());
            qDebug() << max1 << max2;
            if (max1 < max2)
            {
                hpglData.move(i, i-1);
                swapped = true;
            }
        }
    } while (swapped);
}

void hpglListModel::rotateSelectedItems(qreal rotation)
{
    QModelIndex _index;
    qreal translateWidth, translateheight, xOffset, yOffset;

    for (int i = 0; i < rowCount(); ++i)
    {
        _index = index(i);
        QMutexLocker rowLocker(&(hpglData[i]->mutex));

        QGraphicsItemGroup * itemGroup = hpglData.at(i)->hpgl_items_group;

        if (itemGroup->isSelected())
        {
            xOffset = itemGroup->boundingRect().x();
            yOffset = itemGroup->boundingRect().y();
            translateWidth = itemGroup->boundingRect().width();
            translateheight = itemGroup->boundingRect().height();
            itemGroup->setTransformOriginPoint((translateWidth/2.0)+xOffset, (translateheight/2.0)+yOffset);
            itemGroup->setRotation(itemGroup->rotation() + rotation);
        }
    }
}

void hpglListModel::scaleSelectedItems(qreal x, qreal y)
{
    QModelIndex _index;
    QTransform transform;
    qreal translateWidth, translateheight;

    for (int i = 0; i < rowCount(); ++i)
    {
        _index = index(i);
        QMutexLocker rowLocker(&(hpglData[i]->mutex));

        QGraphicsItemGroup * itemGroup = hpglData.at(i)->hpgl_items_group;

        if (itemGroup->isSelected())
        {
            translateWidth = itemGroup->boundingRect().width();
            translateheight = itemGroup->boundingRect().height();
            transform.translate(translateWidth/2.0, translateheight/2.0);
            transform.scale(x, y);
            transform.translate(-translateWidth/2.0, -translateheight/2.0);
            itemGroup->setTransform(itemGroup->transform() * transform);
        }
    }
}

void hpglListModel::createCutoutBox(QPersistentModelIndex _index)
{
    QSettings settings;

    if (_index.row() < 0 || _index.row() >= hpglData.length() || !_index.isValid())
    {
        return;
    }

    QMutexLocker rowLocker(&(hpglData[_index.row()]->mutex));

    double padding = settings.value("device/cutoutboxes/padding", SETDEF_DEVICE_CUTOUTBOXES_PADDING).toDouble();
    if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
    {
        padding = padding * 2.54;
    }
    padding = padding * 1016.0;
    QRectF cutoutRect = hpglData.at(_index.row())->hpgl_items_group->sceneBoundingRect();
    cutoutRect = cutoutRect.marginsAdded(QMarginsF(padding, padding, padding, padding));

    rowLocker.unlock();
    emit newPolygon(_index, static_cast<QPolygonF>(cutoutRect));
}

void hpglListModel::createCutoutBoxes()
{
    for (int i = 0; i < hpglData.length(); ++i)
    {
        createCutoutBox(index(i));
    }
}

void hpglListModel::removeCutoutBox(QPersistentModelIndex _index)
{
    QVector<QGraphicsPolygonItem *> * items;

    if (_index.row() < 0 || _index.row() >= hpglData.length() || !_index.isValid())
    {
        return;
    }

    items = &(hpglData[_index.row()]->hpgl_items);

    hpglData[_index.row()]->hpgl_items_group->removeFromGroup(items->last());
    delete items->last();
    items->removeLast();
}

void hpglListModel::removeCutoutBoxes()
{
    for (int i = 0; i < hpglData.length(); ++i)
    {
        removeCutoutBox(index(i));
    }
}
































