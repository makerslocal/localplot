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

void AncillaryThread::run()
{
    // Set initial state
    state = 0; // 0->stopped 1->plotting 2->cancelled

    exec();
}

void AncillaryThread::do_openSerial()
{
    QSettings settings;
    QString _portLocation = settings.value("serial/port", SETDEF_SERIAL_PORT).toString();
    QSerialPortInfo _device;

    for (int i = 0; i < serialPorts.availablePorts().count(); i++)
    {
        if (_portLocation == serialPorts.availablePorts().at(i).systemLocation())
        {
            _device = serialPorts.availablePorts().at(i);
        }
    }

    if (!serialBuffer.isNull())
    {
        qDebug() << "Serial wasn't closed..? Trying to close it.";
        do_closeSerial();
    }

    if (_device.isNull())
    {
        serialBuffer = new QSerialPort(_portLocation);
    }
    else
    {
        serialBuffer = new QSerialPort(_device);
    }

    serialBuffer->setBaudRate(settings.value("serial/baud", SETDEF_SERIAL_BAUD).toInt());

    int dataBits = settings.value("serial/bytesize", SETDEF_SERIAL_BYTESIZE).toInt();
    if (dataBits == 8)
    {
        serialBuffer->setDataBits(QSerialPort::Data8);
    }
    else if (dataBits == 7)
    {
        serialBuffer->setDataBits(QSerialPort::Data7);
    }
    else if (dataBits == 6)
    {
        serialBuffer->setDataBits(QSerialPort::Data6);
    }
    else if (dataBits == 5)
    {
        serialBuffer->setDataBits(QSerialPort::Data5);
    }

    QString parity = settings.value("serial/parity", SETDEF_SERIAL_PARITY).toString();
    if (parity == "none")
    {
        serialBuffer->setParity(QSerialPort::NoParity);
    }
    else if (parity == "odd")
    {
        serialBuffer->setParity(QSerialPort::OddParity);
    }
    else if (parity == "even")
    {
        serialBuffer->setParity(QSerialPort::EvenParity);
    }
    else if (parity == "mark")
    {
        serialBuffer->setParity(QSerialPort::MarkParity);
    }
    else if (parity == "space")
    {
        serialBuffer->setParity(QSerialPort::SpaceParity);
    }

    int stopBits = settings.value("serial/stopbits", SETDEF_SERIAL_STOPBITS).toInt();
    if (stopBits == 1)
    {
        serialBuffer->setStopBits(QSerialPort::OneStop);
    }
    else if (stopBits == 3)
    {
        serialBuffer->setStopBits(QSerialPort::OneAndHalfStop);
    }
    else if (stopBits == 2)
    {
        serialBuffer->setStopBits(QSerialPort::TwoStop);
    }

    if (settings.value("serial/xonxoff", SETDEF_SERIAL_XONOFF).toBool())
    {
        serialBuffer->setFlowControl(QSerialPort::SoftwareControl);
    }
    else if (settings.value("serial/rtscts", SETDEF_SERIAL_RTSCTS).toBool())
    {
        serialBuffer->setFlowControl(QSerialPort::HardwareControl);
    }
    else
    {
        serialBuffer->setFlowControl(QSerialPort::NoFlowControl);
    }

    serialBuffer->open(QIODevice::WriteOnly);
    if (serialBuffer->isOpen())
    {
        qDebug() << "Flow control: " << serialBuffer->flowControl();
        emit serialOpened(); //handle_serialOpened();
    }
    else
    {
//        ui->textBrowser_console->append(timeStamp() + "Serial port didn't open? :'(");
        do_closeSerial();
    }
}

void AncillaryThread::do_closeSerial()
{
    if (!serialBuffer.isNull())
    {
        serialBuffer->close();
        serialBuffer.clear();
    }
    delete serialBuffer;
    emit serialClosed(); //handle_serialClosed();
}

void AncillaryThread::do_cancelPlot()
{
    state = 2;
}

void AncillaryThread::do_beginPlot(QList<hpgl> * _objList)
{
    // Variables
//    int cutSpeed = settings->value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt();
//    int travelSpeed = settings->value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt();
    QSettings settings;

    // Set state
    state = 1;

//    qDebug() << "Cut speed: " << CUTSPEED;
//    qDebug() << "Travel speed: " << TRAVELSPEED;

//    hpgl_obj obj;
//    QString printThis;
//    int cmdCount;
//    QString command;
//    double time;
//    QProcess process;

    qDebug() << "Plotting file!";
    if (serialBuffer.isNull() || !serialBuffer->isOpen() || objList.isEmpty())
    {
//        ui->textBrowser_console->append(timeStamp() + "Can't plot!");
        return;
    }

    // explain the situation
    if (settings.value("cutter/axis", SETDEF_DEVICE_SPEED_TRAVEL).toBool())
    {
        qDebug() << "Cutting with axis speed delay";
    }
    else
    {
        qDebug() << "Cutting with absolute speed delay";
    }
//    qDebug() << "Cutter speed: " << CUTSPEED;

    // Initialize plotting loop
    index_obj = 0;
    index_cmd = 0;
    obj = _objList->at(0);
    cmdCount = obj.cmdCount();

    emit startedPlotting();

    do_plotNext(_objList);

//    for (int i = 0; i < objList.count(); i++)
//    {
//        obj = objList.at(i);
//        cmdCount = obj.cmdCount();
//        for (int cmd_index = 0; cmd_index < cmdCount; cmd_index++)
//        {
//            if (state != 1)
//            {
//                qDebug() << "Bailing out of plot, cancelled!";
//                break;
//            }
//            printThis = obj.cmdPrint(cmd_index);
//            if (printThis == "OOB")
//            {
////                ui->textBrowser_console->append("ERROR: Object Out Of Bounds! Cannot Plot! D:");
//                //ui->textBrowser_console->append("(try the auto translation button)");
////                ui->textBrowser_console->append("(An X or Y value is less than zero)");
//                return;
//            }
//            serialBuffer->write(printThis.toStdString().c_str());
//            if (settings->value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool())
//            {
//                serialBuffer->flush();
//                command = "sleep ";
//                time = obj.cmdLenHyp(cmd_index);
////                time = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
////                time = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));
//                qDebug() << "- distance: " << time;
//                if (obj.cmdGet(cmd_index).opcode == "PD")
//                {
//                    time = time / speedTranslate(cutSpeed);
//                    qDebug() << "- PD, speedTranslate: " << speedTranslate(cutSpeed);
//                }
//                else if (obj.cmdGet(cmd_index).opcode == "PU")
//                {
//                    time = time / speedTranslate(travelSpeed);
//                    qDebug() << "- PU, speedTranslate: " << speedTranslate(travelSpeed);
//                }
//                qDebug() << "- sleep time: " << time;
//                if (time == 0)
//                    continue;
//                command += QString::number(time);
//                qDebug() << "Starting sleep command: sleep " << time;
//                process.start(command);
//                process.waitForFinished(60000); // Waits for up to 60s
//                qDebug() << "Done with sleep command";
//            }
//        }
//    }
//    qDebug() << "Done plotting.";
}

void AncillaryThread::do_plotNext(QList<hpgl> * _objList)
{
    QSettings settings;
    if (index_cmd >= cmdCount)
    {
        index_cmd = 0;
        index_obj++;
        qDebug() << "Moving on to the next object: " << index_obj;
        if (index_obj >= objList.count())
        {
            qDebug() << "No more objects left.";
            emit donePlotting();
            return;
        }
        obj = objList.at(index_obj);
        cmdCount = obj.cmdCount();
    }
    if (state != 1)
    {
        qDebug() << "Bailing out of plot, cancelled!";
        emit donePlotting();
        return;
    }
    qDebug() << "Plotting command number: " << index_cmd;

    int progress = ((double)index_cmd/(cmdCount-1))*100;
    emit plottingProgress(progress);

    printThis = obj.cmdPrint(index_cmd);
    if (printThis == "OOB")
    {
//                ui->textBrowser_console->append("ERROR: Object Out Of Bounds! Cannot Plot! D:");
        //ui->textBrowser_console->append("(try the auto translation button)");
//                ui->textBrowser_console->append("(An X or Y value is less than zero)");
        emit donePlotting();
        return;
    }
    serialBuffer->write(printThis.toStdString().c_str());
    if (settings.value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool())
    {
        serialBuffer->flush();
        time = obj.time(index_cmd);
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
    index_cmd++;
    QTimer::singleShot(time*1000, this, SLOT(do_plotNext(_objList)));
}
