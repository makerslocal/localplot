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
//    QPointer<QSerialPort> _port;

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
        emit serialOpened(); //handle_serialOpened();
    }
    else
    {
        emit statusUpdate("Serial port didn't open? :'(");
        closeSerial();
    }
    return (_port);
}

void AncillaryThread::closeSerial()
{
    if (!_port.isNull())
    {
        _port->close();
        _port.clear();
    }
    delete _port;
    emit serialClosed(); //handle_serialClosed();
}

void AncillaryThread::do_cancelPlot()
{
    cancelPlotFlag = true;
}

void AncillaryThread::do_beginPlot(QVector<hpgl_file*> * _hpglList)
{
    // Variables
//    int cutSpeed = settings->value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt();
//    int travelSpeed = settings->value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt();
    QPointer<QSerialPort> _port;
    QSettings settings;

    hpglList = _hpglList;
    hpglList_index = 0;
    hpgl_obj_index = 0;
    cancelPlotFlag = false;
    _port = openSerial();

    emit statusUpdate("Plotting file!");

    emit statusUpdate("There are " + QString::number(hpglList->length()) + " hpgl items to plot.");

    if (_port.isNull() || !_port->isOpen() || hpglList->isEmpty())
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

    do_plotNext();
}

void AncillaryThread::do_plotNext()
{
    QSettings settings;
    double time = 0;

    if (hpglList_index >= hpglList->length())
    {
        qDebug() << "No more hpgl files left.";
        closeSerial();
        emit plottingDone();
        return;
    }
    if (hpgl_obj_index >= hpglList->at(hpglList_index)->hpgl_items.length())
    {
        hpglList_index++;
        hpgl_obj_index = 0;
        do_plotNext();
        return;
    }
    if (cancelPlotFlag == true)
    {
        qDebug() << "Bailing out of plot, cancelled!";
        emit plottingCancelled();
        return;
    }

    qDebug() << "Plotting file number: " << hpglList_index << ", object number: " << hpgl_obj_index;

    int progress = ((double)hpglList_index/(hpglList->count()-1))*100;
    emit plottingProgress(progress);

    qDebug() << "Offset: " << hpglList->at(hpglList_index)->hpgl_items_group->pos();

    QString printThis = print(hpglList->at(hpglList_index)->hpgl_items.at(hpgl_obj_index)->polygon(),
                              hpglList->at(hpglList_index)->hpgl_items_group->pos());
    if (printThis == "OOB")
    {
//                ui->textBrowser_console->append("ERROR: Object Out Of Bounds! Cannot Plot! D:");
        //ui->textBrowser_console->append("(try the auto translation button)");
//                ui->textBrowser_console->append("(An X or Y value is less than zero)");
        emit plottingCancelled();
        return;
    }

    if (hpglList_index == (hpglList->count()-1) && hpgl_obj_index == (hpglList->at(hpglList_index)->hpgl_items.length()-1))
    {
        printThis += "PU0,0;SP0;IN;"; // Ending commands
    }

    _port->write(printThis.toStdString().c_str());
    if (settings.value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool())
    {
        _port->flush();
//        time = plotTime(hpglList->at(hpglList_index)->polygon());
//        time = obj.cmdLenHyp(index_cmd);
////                time = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
////                time = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));
//        qDebug() << "- distance: " << time;
//        if (obj.cmdGet(index_cmd).opcode == "PD")
//        {
//            time = time / speedTranslate(CUTSPEED);
//            qDebug() << "- PD, speedTranslate: " << speedTranslate(CUTSPEED);
//        }
//        else if (obj.cmdGet(index_cmd).opcode == "PU")
//        {
//            time = time / speedTranslate(TRAVELSPEED);
//            qDebug() << "- PU, speedTranslate: " << speedTranslate(TRAVELSPEED);
//        }
//        qDebug() << "- sleep time: " << time;
    }
    else
    {
        //
    }

    ++hpgl_obj_index;
    QTimer::singleShot(time*1000, this, SLOT(do_plotNext()));
//    do_plotNext(_port, hpgl_items, ++index);
}

QString AncillaryThread::print(QPolygonF hpgl_poly, QPointF offset)
{
    QString retval = "";

    // Scene offset incurred from dragging
//    QPointF offset = hpgl_poly.pos();
    hpgl_poly.translate(offset.x(), offset.y());

    // Create PU command
    retval += "PU";
    retval += QString::number(static_cast<int>(hpgl_poly.first().x()));
    retval += ",";
    retval += QString::number(static_cast<int>(hpgl_poly.first().y()));
    retval += ";";

    // Create PD command
    retval += "PD";
    for (int idx = 1; idx < hpgl_poly.count(); idx++)
    {
        QPointF point = hpgl_poly.at(idx);

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

/**
 * @brief AncillaryThread::lenHyp
 * @return - the length (in mm) of the hypotenuse of the command's line segments
 */
double AncillaryThread::lenHyp(const QPolygonF _poly)
{
    QPointF prev;
    QPointF curr;
    double mm = 0;
    prev.setX(0);
    prev.setY(0);

    for (int i = 0; i < _poly.length(); i++)
    {
        qreal x, y;
        curr = _poly.at(i);
        x = qFabs(curr.x() - prev.x());
        y = qFabs(curr.y() - prev.y());
        mm += qSqrt(x*x + y*y);
        prev.setX(curr.x());
        prev.setY(curr.y());
    }

    mm = mm * 0.025; // convert graphics units to mm
    return(mm);
}

double AncillaryThread::plotTime(const QPolygonF _poly)
{
    double retval = 0;
    QSettings settings;

    retval = lenHyp(_poly);
//    retval = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
//    retval = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));

    if (retval <= 0)
    {
        return(retval);
    }

    retval = retval / speedTranslate(settings.value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt());

    return(retval);
}

double AncillaryThread::plotTime(const QLineF _line)
{
    double retval = 0;
    QSettings settings;

    retval = _line.length();
//    retval = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
//    retval = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));

    if (retval <= 0)
    {
        return(retval);
    }

    retval = retval / speedTranslate(settings.value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt());

    return(retval);
}

int AncillaryThread::do_loadFile(const file_uid _file)
{
    QFile inputFile;
    QString buffer;

    if (_file.path.isEmpty())
    {
        qDebug() << "File path is empty.";
        return 1;
    }

    inputFile.setFileName(_file.path);
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

    parseHPGL(_file, &buffer);
    emit statusUpdate("HPGL finished parsing.");
    emit hpglParsingDone();
    return 0;
}

// Modifies input string, warning!
void AncillaryThread::parseHPGL(file_uid _file, QString * hpgl_text)
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
            emit newPolygon(_file, newItem);
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


































