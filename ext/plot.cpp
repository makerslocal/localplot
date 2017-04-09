/**
 * Ancilla - worker thread
 * Christopher Bero <bigbero@gmail.com>
 */
#include "plot.h"

ExtPlot::ExtPlot(hpglListModel * model)
{
    runPerimeterFlag = false;
    hpglModel = model;
}

ExtPlot::ExtPlot(hpglListModel * model, QRectF _perimeter)
{
    runPerimeterFlag = true;
    perimeterRect = _perimeter;
    hpglModel = model;
}

ExtPlot::~ExtPlot()
{
    closeSerial();
}

QPointer<QSerialPort> ExtPlot::openSerial()
{
    QSettings settings;
    QString _portLocation = settings.value("serial/port", SETDEF_SERIAL_PORT).toString();
    QSerialPortInfo _deviceInfo;

    for (int i = 0; i < _deviceInfo.availablePorts().count(); i++)
    {
        if (_portLocation == _deviceInfo.availablePorts().at(i).systemLocation())
        {
            _deviceInfo = _deviceInfo.availablePorts().at(i);
        }
    }

    if (_deviceInfo.isNull())
    {
        _port = new QSerialPort(_portLocation);
    }
    else
    {
        _port = new QSerialPort(_deviceInfo);
    }

    _port->setBaudRate(settings.value("serial/baud", SETDEF_SERIAL_BAUD).toInt());

    int dataBits = settings.value("serial/bytesize", SETDEF_SERIAL_BYTESIZE).toInt();
    if (dataBits == 8)
    {
        _port->setDataBits(QSerialPort::Data8);
    }
    else if (dataBits == 7)
    {
        _port->setDataBits(QSerialPort::Data7);
    }
    else if (dataBits == 6)
    {
        _port->setDataBits(QSerialPort::Data6);
    }
    else if (dataBits == 5)
    {
        _port->setDataBits(QSerialPort::Data5);
    }

    QString parity = settings.value("serial/parity", SETDEF_SERIAL_PARITY).toString();
    if (parity == "none")
    {
        _port->setParity(QSerialPort::NoParity);
    }
    else if (parity == "odd")
    {
        _port->setParity(QSerialPort::OddParity);
    }
    else if (parity == "even")
    {
        _port->setParity(QSerialPort::EvenParity);
    }
    else if (parity == "mark")
    {
        _port->setParity(QSerialPort::MarkParity);
    }
    else if (parity == "space")
    {
        _port->setParity(QSerialPort::SpaceParity);
    }

    int stopBits = settings.value("serial/stopbits", SETDEF_SERIAL_STOPBITS).toInt();
    if (stopBits == 1)
    {
        _port->setStopBits(QSerialPort::OneStop);
    }
    else if (stopBits == 3)
    {
        _port->setStopBits(QSerialPort::OneAndHalfStop);
    }
    else if (stopBits == 2)
    {
        _port->setStopBits(QSerialPort::TwoStop);
    }

    if (settings.value("serial/xonxoff", SETDEF_SERIAL_XONOFF).toBool())
    {
        _port->setFlowControl(QSerialPort::SoftwareControl);
    }
    else if (settings.value("serial/rtscts", SETDEF_SERIAL_RTSCTS).toBool())
    {
        _port->setFlowControl(QSerialPort::HardwareControl);
    }
    else
    {
        _port->setFlowControl(QSerialPort::NoFlowControl);
    }

    _port->open(QIODevice::WriteOnly);
    if (_port->isOpen())
    {
        qDebug() << "Flow control: " << _port->flowControl();
        emit serialOpened();
    }
    else
    {
        emit statusUpdate("Serial port didn't open? :'(");
        closeSerial();
    }
    return (_port);
}

void ExtPlot::closeSerial()
{
    if (!_port.isNull())
    {
        _port->flush();
        _port->close();
        _port.clear();
    }
    delete _port;
    emit serialClosed();
}

void ExtPlot::cancel()
{
    cancelPlotFlag = true;
}

void ExtPlot::process()
{
    // Variables
    QPointer<QSerialPort> _port;
    QSettings settings;

    hpglList_index = 0;
    hpgl_obj_index = 0;
    cancelPlotFlag = false;
    _port = openSerial();

    emit statusUpdate("Plotting file!");

    emit statusUpdate("There are " + QString::number(hpglModel->rowCount()) + " hpgl items to plot.");

    if (_port.isNull() || !_port->isOpen() || hpglModel->rowCount() == 0)
    {
        emit statusUpdate("Can't plot!", Qt::darkRed);
        emit finished();
        return;
    }

    qDebug() << "do perim: " << runPerimeterFlag;
    if (runPerimeterFlag)
    {
        QPointF point;
        QString retval = "IN;SP1;PU";
        point = perimeterRect.topLeft();
        retval += QString::number(static_cast<int>(point.x()));
        retval += ",";
        retval += QString::number(static_cast<int>(point.y()));
        retval += ",";
        point = perimeterRect.topRight();
        retval += QString::number(static_cast<int>(point.x()));
        retval += ",";
        retval += QString::number(static_cast<int>(point.y()));
        retval += ",";
        point = perimeterRect.bottomRight();
        retval += QString::number(static_cast<int>(point.x()));
        retval += ",";
        retval += QString::number(static_cast<int>(point.y()));
        retval += ",";
        point = perimeterRect.bottomLeft();
        retval += QString::number(static_cast<int>(point.x()));
        retval += ",";
        retval += QString::number(static_cast<int>(point.y()));
        retval += ",0,0;SP0;IN;";
        qDebug() << "perimeter: " << retval;
        _port->write(retval.toStdString().c_str());
        _port->flush();
        closeSerial();
        emit finished();
        return;
    }

    // explain the situation
    if (settings.value("cutter/axis", SETDEF_DEVICE_SPEED_TRAVEL).toBool())
    {
        emit statusUpdate("Cutting with axis speed delay");
    }
    else
    {
        emit statusUpdate("Cutting with absolute speed delay");
    }

    _port->write("IN;SP1;");

    do_plotNext();
}

void ExtPlot::do_plotNext()
{
    QSettings settings;
    double time = 0;
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QVector<QGraphicsPolygonItem*> * items;

    if (hpglList_index >= hpglModel->rowCount())
    {
        emit statusUpdate("No more hpgl files left to plot.");
        closeSerial();
        emit finished();
        return;
    }

    index = hpglModel->index(hpglList_index);
    hpglModel->dataItemsGroup(index, itemGroup, items);

    if (itemGroup == NULL)
    {
        qDebug() << "Error: itemgroup is null in do_plotnext().";
        return;
    }
    if (items == NULL)
    {
        qDebug() << "Error: items is null in do_plotnext().";
        return;
    }

    hpglModel->mutexLock();

    if (hpgl_obj_index >= items->length())
    {
        hpglModel->mutexUnlock();
        hpglList_index++;
        hpgl_obj_index = 0;
        do_plotNext();
        return;
    }
    index = hpglModel->index(hpglList_index);
    if (cancelPlotFlag == true)
    {
        hpglModel->mutexUnlock();
        emit statusUpdate("Bailing out of plot, cancelled!", Qt::darkRed);
        closeSerial();
        emit finished();
        return;
    }

//    qDebug() << "Plotting file number: " << hpglList_index << ", object number: " << hpgl_obj_index;
//    qDebug() << "Total file count: " << hpglList->count();

    int progressPercent = (((double)hpglList_index+((double)hpgl_obj_index/(items->count()-1)))/(hpglModel->rowCount()))*100;
    emit progress(progressPercent);

    QString printThis = print(items->at(hpgl_obj_index)->polygon(),
                              itemGroup);

    if (printThis == "OOB")
    {
        hpglModel->mutexUnlock();
        emit finished();
        return;
    }

    if (hpglList_index == (hpglModel->rowCount()-1) && hpgl_obj_index == (items->length()-1))
    {
        printThis += "PU0,0;SP0;IN;"; // Ending commands
    }

    _port->write(printThis.toStdString().c_str());
    if (settings.value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool())
    {
        _port->flush();
        time = ExtEta::plotTime(items->at(hpgl_obj_index)->polygon());
        QLineF last_line;
        last_line.setP1(last_point);
        last_line.setP2(itemGroup->mapToScene(items->at(hpgl_obj_index)->polygon().first()));
        time += ExtEta::plotTime(last_line);
        last_point = itemGroup->mapToScene(items->at(hpgl_obj_index)->polygon().last());
    }

    hpglModel->mutexUnlock();

    ++hpgl_obj_index;
    QTimer::singleShot(time*1000, this, SLOT(do_plotNext()));
}

QString ExtPlot::print(QPolygonF hpgl_poly, QGraphicsItemGroup * itemGroup)
{
    QString retval = "";
    QPointF point;

    // Create PU command
    point = itemGroup->mapToScene(hpgl_poly.first());
    retval += "PU";
    retval += QString::number(static_cast<int>(point.x()));
    retval += ",";
    retval += QString::number(static_cast<int>(point.y()));
    retval += ";";

    // Create PD command
    retval += "PD";
    for (int idx = 1; idx < hpgl_poly.count(); idx++)
    {
        point = itemGroup->mapToScene(hpgl_poly.at(idx));

        if (point.x() < 0 || point.y() < 0)
        {
        retval = "OOB"; // Out of Bounds
        return retval;
        }

        retval += QString::number(static_cast<int>(point.x()));
        retval += ",";
        retval += QString::number(static_cast<int>(point.y()));
        if (idx < (hpgl_poly.length()-1))
        {
            retval += ",";
        }
    }
    retval += ";";

    return(retval);
}

void ExtPlot::statusUpdate(QString _consoleStatus)
{
    emit statusUpdate(_consoleStatus, Qt::black);
}






































