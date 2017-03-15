#include "hpgllistmodel.h"

bool operator==(const file_uid& lhs, const file_uid& rhs)
{
    return(lhs.path == rhs.path && lhs.uid == rhs.uid);
}

hpglListModel::hpglListModel(QObject *parent)
    :QAbstractListModel(parent)
{
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
    else if (role > hpglUserRoles::role_first && role < hpglUserRoles::role_last)
    {
        switch (role)
        {
        case hpglUserRoles::role_filename:
            return(hpglData.at(index.row())->name.filename);
            break;
        case hpglUserRoles::role_path:
            return(hpglData.at(index.row())->name.path);
            break;
        case hpglUserRoles::role_uid:
            return(hpglData.at(index.row())->name.uid);
            break;
        case hpglUserRoles::role_name:
            return( QVariant::fromValue(hpglData.at(index.row())->name) );
            break;
        case hpglUserRoles::role_hpgl_items:
            return( QVariant::fromValue(&(hpglData.at(index.row())->hpgl_items)) );
            break;
        case hpglUserRoles::role_hpgl_items_group:
            return( QVariant::fromValue(hpglData.at(index.row())->hpgl_items_group) );
            break;
        default:
            return(QVariant());
            break;
        }
    }
    return(QVariant());
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
        hpglData[index.row()]->name.filename = value.toString();
        changedRoles.push_back(Qt::DisplayRole);
        emit dataChanged(index, index, changedRoles);
        return true;
    }
    if (role > hpglUserRoles::role_first && role < hpglUserRoles::role_last)
    {
        switch (role)
        {
        case hpglUserRoles::role_filename:
            hpglData[index.row()]->name.filename = value.toString();
            changedRoles.push_back(Qt::DisplayRole);
            changedRoles.push_back(hpglUserRoles::role_filename);
            break;
        case hpglUserRoles::role_path:
            hpglData[index.row()]->name.path = value.toString();
            changedRoles.push_back(hpglUserRoles::role_path);
            break;
        case hpglUserRoles::role_uid:
            hpglData[index.row()]->name.uid = value.toInt();
            changedRoles.push_back(hpglUserRoles::role_uid);
            break;
        case hpglUserRoles::role_name:
            qDebug() << "Changing name of : " << index.row();
            hpglData[index.row()]->name = value.value<file_uid>();
            changedRoles.push_back(Qt::DisplayRole);
            changedRoles.push_back(hpglUserRoles::role_name);
            break;
        case hpglUserRoles::role_hpgl_items:
        {
            QGraphicsPolygonItem * _poly = value.value<QGraphicsPolygonItem *>();
            hpglData[index.row()]->hpgl_items.push_back(_poly);
            hpglData[index.row()]->hpgl_items_group->addToGroup(static_cast<QGraphicsItem*>(_poly));
//            hpglData[index.row()]->hpgl_items = value.value<QVector<QGraphicsPolygonItem *>>();
            changedRoles.push_back(hpglUserRoles::role_hpgl_items);
            break;
        }
        case hpglUserRoles::role_hpgl_items_group:
            hpglData[index.row()]->hpgl_items_group = value.value<QGraphicsItemGroup*>();
            changedRoles.push_back(hpglUserRoles::role_hpgl_items_group);
            break;
        default:
            return(false);
            break;
        }
        emit dataChanged(index, index, changedRoles);
        return(true);
    }
    return false;
}

bool hpglListModel::setGroupFlag(const QModelIndex &index, QGraphicsItem::GraphicsItemFlag flag, bool flagValue)
{
    if (index.row() >= 0 && index.row() < hpglData.length())
    {
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

//Qt::ItemFlags hpglListModel::flags(const QModelIndex &index) const
//{
//    if (!index.isValid())
//    {
//        return Qt::ItemIsEnabled;
//    }
//    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
//}

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
        newFile->hpgl_items_group = new QGraphicsItemGroup;
        newFile->hpgl_items_group->setData(QMODELINDEX_KEY, QPersistentModelIndex(index));
        newFile->name.filename = "NA";
        newFile->name.path = "NA";
        newFile->name.uid = -1;
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
//        delete hpglData[i]->hpgl_items_group;
        hpglData[i]->hpgl_items.clear();
        delete hpglData[i];
        hpglData.remove(i);
    }
    endRemoveRows();
    return true;
}



































