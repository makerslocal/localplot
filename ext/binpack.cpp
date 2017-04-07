#include "binpack.h"

ExtBinPack::ExtBinPack(hpglListModel *model)
{
    cancelFlag = false;
    hpglModel = model;
}

ExtBinPack::~ExtBinPack()
{
    //
}

void ExtBinPack::process()
{
    QSettings settings;
//    rbp::ShelfBinPack packer;
    rbp::MaxRectsBinPack mrPacker;
    QPersistentModelIndex index;
    QGraphicsItemGroup * itemGroup;

    hpglModel->sort();

    int initX, initY;
    QLineF widthLine = MainWindow::get_widthLine();
    initX = widthLine.p2().y() - widthLine.p1().y();
    initY = 0;

    for (int i = 0; i < hpglModel->rowCount(); ++i)
    {
        if (cancelFlag)
        {
            statusUpdate("Cancelling auto arrange.", Qt::darkRed);
            emit finished();
            return;
        }
        index = hpglModel->index(i);
        itemGroup = NULL;
        hpglModel->dataGroup(index, itemGroup);
        hpglModel->mutexLock();

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontainselected().";
            hpglModel->mutexUnlock();
            emit finished();
            return;
        }

        initY += qMax(itemGroup->boundingRect().width(), itemGroup->boundingRect().height());

        hpglModel->mutexUnlock();
    }

//    packer.Init(initX, initY, true);
    mrPacker.Init(initX, initY);

    for (int i = 0; i < hpglModel->rowCount(); ++i)
    {
        if (cancelFlag)
        {
            statusUpdate("Cancelling auto arrange.", Qt::darkRed);
            emit finished();
            return;
        }
        index = hpglModel->index(i);
        itemGroup = NULL;
        hpglModel->dataGroup(index, itemGroup);
        hpglModel->mutexLock();

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontainselected().";
            hpglModel->mutexUnlock();
            emit finished();
            return;
        }

        QMarginsF margin;
        double padding = settings.value("device/cutoutboxes/padding", SETDEF_DEVICE_CUTOUTBOXES_PADDING).toDouble();
        if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
        {
            padding = padding * 2.54;
        }
        padding = padding * 1016.0;

        margin.setTop(padding);
        margin.setRight(padding);

//        rbp::Rect thisrect = packer.Insert(itemGroup->boundingRect().marginsAdded(margin).width(),
//                                           itemGroup->boundingRect().marginsAdded(margin).height(),
//                                           rbp::ShelfBinPack::ShelfChoiceHeuristic::ShelfWorstAreaFit);

        rbp::Rect thisrect = mrPacker.Insert(itemGroup->boundingRect().marginsAdded(margin).width(),
                                             itemGroup->boundingRect().marginsAdded(margin).height(),
                                             rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBottomLeftRule);

        QRectF rect;
        rect.setX(thisrect.y);
        rect.setY(thisrect.x);
        rect.setWidth(thisrect.height);
        rect.setHeight(thisrect.width);
        emit packedRect(index, rect);

        hpglModel->mutexUnlock();
    }

    emit statusUpdate("Finished arranging files.");
    emit finished();
}

void ExtBinPack::cancel()
{
    cancelFlag = true;
}

void ExtBinPack::statusUpdate(QString _consoleStatus)
{
    emit statusUpdate(_consoleStatus, Qt::black);
}































