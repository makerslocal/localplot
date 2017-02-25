/**
 * Ancilla - worker thread
 * Christopher Bero <bigbero@gmail.com>
 */
#include "ancilla.h"

AncillaryThread::AncillaryThread()
{
    // Instantiation happens on thread start (do_run).
}

AncillaryThread::~AncillaryThread()
{
    //
}

void AncillaryThread::do_run()
{
    //?
}

QPointer<QSerialPort> AncillaryThread::openSerial()
{
    QSettings settings;
    QString _portLocation = settings.value("serial/port", SETDEF_SERIAL_PORT).toString();
    QSerialPortInfo _deviceInfo;
    QPointer<QSerialPort> _device;

    for (int i = 0; i < _deviceInfo.availablePorts().count(); i++)
    {
        if (_portLocation == _deviceInfo.availablePorts().at(i).systemLocation())
        {
            _deviceInfo = _deviceInfo.availablePorts().at(i);
        }
    }

    if (_deviceInfo.isNull())
    {
        _device = new QSerialPort(_portLocation);
    }
    else
    {
        _device = new QSerialPort(_deviceInfo);
    }

    _device->setBaudRate(settings.value("serial/baud", SETDEF_SERIAL_BAUD).toInt());

    int dataBits = settings.value("serial/bytesize", SETDEF_SERIAL_BYTESIZE).toInt();
    if (dataBits == 8)
    {
        _device->setDataBits(QSerialPort::Data8);
    }
    else if (dataBits == 7)
    {
        _device->setDataBits(QSerialPort::Data7);
    }
    else if (dataBits == 6)
    {
        _device->setDataBits(QSerialPort::Data6);
    }
    else if (dataBits == 5)
    {
        _device->setDataBits(QSerialPort::Data5);
    }

    QString parity = settings.value("serial/parity", SETDEF_SERIAL_PARITY).toString();
    if (parity == "none")
    {
        _device->setParity(QSerialPort::NoParity);
    }
    else if (parity == "odd")
    {
        _device->setParity(QSerialPort::OddParity);
    }
    else if (parity == "even")
    {
        _device->setParity(QSerialPort::EvenParity);
    }
    else if (parity == "mark")
    {
        _device->setParity(QSerialPort::MarkParity);
    }
    else if (parity == "space")
    {
        _device->setParity(QSerialPort::SpaceParity);
    }

    int stopBits = settings.value("serial/stopbits", SETDEF_SERIAL_STOPBITS).toInt();
    if (stopBits == 1)
    {
        _device->setStopBits(QSerialPort::OneStop);
    }
    else if (stopBits == 3)
    {
        _device->setStopBits(QSerialPort::OneAndHalfStop);
    }
    else if (stopBits == 2)
    {
        _device->setStopBits(QSerialPort::TwoStop);
    }

    if (settings.value("serial/xonxoff", SETDEF_SERIAL_XONOFF).toBool())
    {
        _device->setFlowControl(QSerialPort::SoftwareControl);
    }
    else if (settings.value("serial/rtscts", SETDEF_SERIAL_RTSCTS).toBool())
    {
        _device->setFlowControl(QSerialPort::HardwareControl);
    }
    else
    {
        _device->setFlowControl(QSerialPort::NoFlowControl);
    }

    _device->open(QIODevice::WriteOnly);
    if (_device->isOpen())
    {
        qDebug() << "Flow control: " << _device->flowControl();
        emit serialOpened(); //handle_serialOpened();
    }
    else
    {
        emit statusUpdate("Serial port didn't open? :'(");
        closeSerial(_device);
    }
    return (_device);
}

void AncillaryThread::closeSerial(QPointer<QSerialPort> _device)
{
    if (!_device.isNull())
    {
        _device->close();
        _device.clear();
    }
    delete _device;
    emit serialClosed(); //handle_serialClosed();
}

void AncillaryThread::do_cancelPlot()
{
    cancelPlotFlag = true;
}

void AncillaryThread::do_beginPlot(const QVector<QGraphicsPolygonItem *> hpgl_items)
{
    // Variables
//    int cutSpeed = settings->value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt();
//    int travelSpeed = settings->value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt();
    QPointer<QSerialPort> _port;
    QSettings settings;

    cancelPlotFlag = false;

    _port = openSerial();

    emit statusUpdate("Plotting file!");

    emit statusUpdate("There are " + QString::number(hpgl_items.count()) + " hpgl items to plot.");

    if (_port.isNull() || !_port->isOpen() || hpgl_items.isEmpty())
    {
        emit statusUpdate("Can't plot!");
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

    emit plottingStarted();

    _port->write("IN;SP1;");

    do_plotNext(_port, hpgl_items, 0);
}

void AncillaryThread::do_plotNext(QPointer<QSerialPort> _port,
                                  const QVector<QGraphicsPolygonItem *> hpgl_items,
                                  int index)
{
    QSettings settings;

    if (index >= hpgl_items.count())
    {
        qDebug() << "No more objects left.";
        emit plottingDone();
        return;
    }
    if (cancelPlotFlag == true)
    {
        qDebug() << "Bailing out of plot, cancelled!";
        emit plottingCancelled();
        return;
    }

    qDebug() << "Plotting command number: " << index;

    int progress = ((double)index/(hpgl_items.count()-1))*100;
    emit plottingProgress(progress);

    QString printThis = print(hpgl_items, index);
    if (printThis == "OOB")
    {
//                ui->textBrowser_console->append("ERROR: Object Out Of Bounds! Cannot Plot! D:");
        //ui->textBrowser_console->append("(try the auto translation button)");
//                ui->textBrowser_console->append("(An X or Y value is less than zero)");
        emit plottingCancelled();
        return;
    }
    _port->write(printThis.toStdString().c_str());
//    if (settings.value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool())
//    {
//        _device->flush();
//        time = obj.time(index_cmd);
////        time = obj.cmdLenHyp(index_cmd);
//////                time = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
//////                time = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));
////        qDebug() << "- distance: " << time;
////        if (obj.cmdGet(index_cmd).opcode == "PD")
////        {
////            time = time / speedTranslate(CUTSPEED);
////            qDebug() << "- PD, speedTranslate: " << speedTranslate(CUTSPEED);
////        }
////        else if (obj.cmdGet(index_cmd).opcode == "PU")
////        {
////            time = time / speedTranslate(TRAVELSPEED);
////            qDebug() << "- PU, speedTranslate: " << speedTranslate(TRAVELSPEED);
////        }
////        qDebug() << "- sleep time: " << time;
//    }
//    else
//    {
//        //
//    }

//    QTimer::singleShot(time*1000, this, SLOT(do_plotNext(_objList)));
    do_plotNext(_port, hpgl_items, ++index);
}

QString AncillaryThread::print(const QVector<QGraphicsPolygonItem *> hpgl_items,
                               int index)
{
    QString retval = "";
    QPolygonF poly = hpgl_items[index]->polygon();

    // Create PU command
    retval += "PU";
    retval += QString::number(poly.first().x());
    retval += ",";
    retval += QString::number(poly.first().y());
    retval += ";";

    // Create PD command
    retval += "PD";
    for (int idx = 1; idx < poly.count(); idx++)
    {
        QPointF point = poly.at(idx);

        if (point.x() < 0 || point.y() < 0)
        {
        retval = "OOB"; // Out of Bounds
        return retval;
        }

        retval += QString::number(point.x());
        retval += ",";
        retval += QString::number(point.y());
        if (idx < (poly.count()-1))
        {
            retval += ",";
        }
    }
    retval += ";";

    if (index == (hpgl_items.count()-1))
    {
        retval += "PU0,0;SP0;IN;"; // Ending commands
    }

    return(retval);
}

int AncillaryThread::do_loadFile(QString const _filepath)
{
    QFile inputFile;
    QString buffer;

    if (_filepath.isEmpty())
    {
        qDebug() << "File path is empty.";
        return 1;
    }

    inputFile.setFileName(_filepath);
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        // failed to open
        qDebug() << "Failed to open file.";
        return 1;
    }
    emit statusUpdate("File opened.");

    QTextStream fstream(&inputFile);
    buffer = fstream.readAll();
    inputFile.close();
    emit statusUpdate("File closed.");

    parseHPGL(&buffer);
    emit statusUpdate("HPGL finished parsing.");
    emit hpglParsingDone();
    return 0;
}

// Modifies input string, warning!
void AncillaryThread::parseHPGL(QString * hpgl_text)
{
    QPointF tail(0, 0);

    hpgl_text->remove('\n');
    int numCmds = hpgl_text->count(';');
    for (int i = 0; i < numCmds; i++)
    {
        QPolygonF newItem;
        QString cmdText;
        QString opcode;
        int pen;

        cmdText = hpgl_text->section(';', i, i);

        qDebug() << "====\n= Processing command: ";

        // Get opcode, first two characters
        opcode = cmdText.mid(0, 2);

        qDebug() << "= " << opcode;

        // Parse opcode
        if (opcode == "PU")
        {
            // Pen up - we assume a single line (two points)
            int commaCount, newX, newY;
            cmdText.remove(0,2);
            commaCount = cmdText.count(',');
//            qDebug() << "= Comma count: " << commaCount;
            int i = commaCount - 1;
            newX = cmdText.section(',', i, i).toInt();
            i++;
            newY = cmdText.section(',', i, i).toInt();
            tail.setX(newX);
            tail.setY(newY);
        }
        else if (opcode == "PD")
        {
            // Pen down
            cmdText.remove(0,2);
            int commaCount = cmdText.count(',');
//            qDebug() << "= Comma count: " << commaCount;
            newItem << tail; // Begin from last PU location
            for (int i = 0; i < commaCount; i++)
            {
                //qDebug() << "processing coord: " << text.at(i) << endl;
                int newX = cmdText.section(',', i, i).toInt();
                i++;
                int newY = cmdText.section(',', i, i).toInt();
//                qDebug() << "= Found x: " << newX << " y: " << newY;
                newItem << QPointF(newX, newY);
//                if (i < (commaCount-2) && ((i+1) % 500) == 0)
//                {
//                    qDebug() << "Breaking line";
//                    cmdList.push_back(newCmd);
//                    newCmd.coordList.clear();
//                    newCmd.coordList.push_back(QPoint(newX, newY));
//                }
            }
            emit newPolygon(newItem);
        }
        else if (opcode == "IN")
        {
            // Begin plotting (automatically handled)
        }
        else if (opcode == "SP")
        {
            // Set pen
            pen = cmdText.mid(2,1).toInt();
            qDebug() << "[" << QString::number(pen) << "]";
        }
        int progress = ((double)i/(numCmds-1))*100;
        emit plottingProgress(progress);
    }
}


































