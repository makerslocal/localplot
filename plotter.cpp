#include "plotter.h"

/*
 * Plotter thread
 */

Plotter::Plotter()
{
    // Instantiation happens on thread start (do_run).
}

Plotter::~Plotter()
{
    //
}

void Plotter::do_run()
{
    // Instantiate settings object
    init_localplot_settings();
    settings = new QSettings();

    // Set initial state
    state = 0; // 0->stopped 1->plotting 2->cancelled
}

void Plotter::do_openSerial()
{
    QString _portLocation = settings->value("serial/port", SETDEF_SERIAL_PORT).toString();
    QSerialPortInfo _device;

    for (int i = 0; i < serialPorts.availablePorts().count(); i++)
    {
        if (_portLocation == serialPorts.availablePorts().at(i).systemLocation())
        {
            _device = serialPorts.availablePorts().at(i);
        }
    }

    if (_device.isNull())
    {
        qDebug() << "Serial list needs to be refreshed or something";
        do_closeSerial();
        return;
    }

    if (!serialBuffer.isNull())
    {
        serialBuffer.clear();
        emit serialClosed(); //handle_serialClosed();
    }
    serialBuffer = new QSerialPort(_device);
    serialBuffer->setBaudRate(settings->value("serial/baud", SETDEF_SERIAL_BAUD).toInt());

    int dataBits = settings->value("serial/bytesize", SETDEF_SERIAL_BYTESIZE).toInt();
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

    QString parity = settings->value("serial/parity", SETDEF_SERIAL_PARITY).toString();
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

    int stopBits = settings->value("serial/stopbits", SETDEF_SERIAL_STOPBITS).toInt();
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

    if (settings->value("serial/xonxoff", SETDEF_SERIAL_XONOFF).toBool())
    {
        serialBuffer->setFlowControl(QSerialPort::SoftwareControl);
    }
    else if (settings->value("serial/rtscts", SETDEF_SERIAL_RTSCTS).toBool())
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

void Plotter::do_closeSerial()
{
    if (!serialBuffer.isNull())
    {
        serialBuffer->close();
        serialBuffer.clear();
    }
    emit serialClosed(); //handle_serialClosed();
}

void Plotter::do_cancelPlot()
{
    state = 2;
}

void Plotter::do_beginPlot(QList<hpgl_obj> _objList)
{
    // Variables
//    int cutSpeed = settings->value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt();
//    int travelSpeed = settings->value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt();

    objList = _objList;

    // Set state
    state = 1;

    qDebug() << "Cut speed: " << CUTSPEED;
    qDebug() << "Travel speed: " << TRAVELSPEED;

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
    if (settings->value("cutter/axis", SETDEF_DEVICE_SPEED_TRAVEL).toBool())
    {
        qDebug() << "Cutting with axis speed delay";
    }
    else
    {
        qDebug() << "Cutting with absolute speed delay";
    }
    qDebug() << "Cutter speed: " << CUTSPEED;

    // Initialize plotting loop
    index_obj = 0;
    index_cmd = 0;
    obj = objList.at(0);
    cmdCount = obj.cmdCount();

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

void Plotter::do_plotNext()
{
    if (index_cmd >= cmdCount)
    {
        index_cmd = 0;
        index_obj++;
        if (index_obj >= objList.count())
        {
            // end
        }
        obj = objList.at(index_obj);
        cmdCount = obj.cmdCount();
    }
    if (state != 1)
    {
        qDebug() << "Bailing out of plot, cancelled!";
        // end
    }
    printThis = obj.cmdPrint(index_cmd);
    if (printThis == "OOB")
    {
//                ui->textBrowser_console->append("ERROR: Object Out Of Bounds! Cannot Plot! D:");
        //ui->textBrowser_console->append("(try the auto translation button)");
//                ui->textBrowser_console->append("(An X or Y value is less than zero)");
        return;
    }
    serialBuffer->write(printThis.toStdString().c_str());
    if (settings->value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool())
    {
        serialBuffer->flush();
        time = obj.cmdLenHyp(index_cmd);
//                time = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
//                time = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));
        qDebug() << "- distance: " << time;
        if (obj.cmdGet(index_cmd).opcode == "PD")
        {
            time = time / speedTranslate(CUTSPEED);
            qDebug() << "- PD, speedTranslate: " << speedTranslate(CUTSPEED);
        }
        else if (obj.cmdGet(index_cmd).opcode == "PU")
        {
            time = time / speedTranslate(TRAVELSPEED);
            qDebug() << "- PU, speedTranslate: " << speedTranslate(TRAVELSPEED);
        }
        qDebug() << "- sleep time: " << time;
        QTimer::singleShot(time*1000, this, SLOT(do_plotNext()));
    }

}

double Plotter::speedTranslate(int setting_speed)
{
//    return((0.5*setting_speed) + 30);
    return((0.3*setting_speed) + 70);
//    return((0.52*setting_speed) + 24.8);
}
