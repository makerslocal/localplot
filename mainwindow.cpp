#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * Reference: http://cstep.luberth.com/HPGL.pdf
 *
 * Language Structure:
 * Using Inkscape's default format -> XXy1,y1,y2,y2;
 * Two uppercase characters followed by a CSV list and terminated with a semicolon.
 *
 * Path Vertex Object:
 * State: up or down
 * Vertex coordinate: in graphic units (1/1016") (0.025mm)
 *
 * Program file structure
 * hpgl_coord - Structure for storing simple coordinates, may be merged later
 * hpgl_cmd - Structure for storing a single hpgl command
 * hpgl_obj - An hpgl object, or cluster of commands that share similar
 *             properties and transformations.
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_serialRefresh, SIGNAL(clicked()), this, SLOT(do_refreshSerialList()));
    connect(ui->pushButton_serialConnect, SIGNAL(clicked()), this, SLOT(handle_serialConnectBtn()));
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->pushButton_fileLoad, SIGNAL(clicked()), this, SLOT(do_loadFile()));
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));

    // Initialize interface
    ui->comboBox_baud->insertItems(0, QStringList() << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200");
    ui->comboBox_baud->setCurrentIndex(2);
    //ui->comboBox_bytesize->insertItems(0, QStringList() << "8" << "7" << "6" << "5");
    ui->comboBox_bytesize->addItem("8", 8);
    ui->comboBox_bytesize->addItem("7", 7);
    ui->comboBox_bytesize->addItem("6", 6);
    ui->comboBox_bytesize->addItem("5", 5);
    ui->comboBox_parity->insertItems(0, QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space");
    ui->comboBox_stopbits->insertItems(0, QStringList() << "1" << "1.5" << "2");
    do_refreshSerialList();

    ui->graphicsView_view->setScene(&plotScene);
}

MainWindow::~MainWindow()
{
    do_closeSerial();

    delete ui;
}

QString MainWindow::timeStamp()
{
    return(QTime::currentTime().toString("[HH:mm:ss:zzz] "));
}

void MainWindow::do_refreshSerialList()
{
    QList<QSerialPortInfo> _ports = serialPorts.availablePorts();
    int _index = _ports.count();
    ui->comboBox_serialPort->clear();
    for (int i = 0; i < _index; ++i)
    {
        ui->comboBox_serialPort->insertItem(i, _ports.at(i).portName());
        ui->textBrowser_console->append(_ports.at(i).portName());
    }
    ui->textBrowser_console->append(timeStamp() + "Serial list refreshed \\o/");
}

void MainWindow::do_openSerial()
{
    int _portIndex = ui->comboBox_serialPort->currentIndex();
    QSerialPortInfo _device;

    if (_portIndex >= 0 && serialPorts.availablePorts().count() > _portIndex)
        _device = serialPorts.availablePorts().at(_portIndex);
    else
    {
        do_refreshSerialList();
        do_closeSerial();
        return;
    }
    if (!serialBuffer.isNull())
    {
        serialBuffer.clear();
        handle_serialClosed();
    }
    serialBuffer = new QSerialPort(_device);
    serialBuffer->setBaudRate(atoi(ui->comboBox_baud->currentText().toStdString().c_str()));

    int dataBits = ui->comboBox_bytesize->currentData().toInt();
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

    QString parity = ui->comboBox_parity->currentText();
    if (parity == "None")
    {
        serialBuffer->setParity(QSerialPort::NoParity);
    }
    else if (parity == "Odd")
    {
        serialBuffer->setParity(QSerialPort::OddParity);
    }
    else if (parity == "Even")
    {
        serialBuffer->setParity(QSerialPort::EvenParity);
    }
    else if (parity == "Mark")
    {
        serialBuffer->setParity(QSerialPort::MarkParity);
    }
    else if (parity == "Space")
    {
        serialBuffer->setParity(QSerialPort::SpaceParity);
    }

    QString stopBits = ui->comboBox_stopbits->currentText();
    if (stopBits == "1")
    {
        serialBuffer->setStopBits(QSerialPort::OneStop);
    }
    else if (stopBits == "1.5")
    {
        serialBuffer->setStopBits(QSerialPort::OneAndHalfStop);
    }
    else if (stopBits == "2")
    {
        serialBuffer->setStopBits(QSerialPort::TwoStop);
    }

    if (ui->radioButton_XonXoff->isChecked())
    {
        serialBuffer->setFlowControl(QSerialPort::SoftwareControl);
    }
    else if (ui->radioButton_RtsCts->isChecked())
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
        handle_serialOpened();
    }
    else
    {
        ui->textBrowser_console->append(timeStamp() + "Serial port didn't open? :'(");
        do_closeSerial();
    }

}

void MainWindow::do_closeSerial()
{
    if (!serialBuffer.isNull())
    {
        serialBuffer->close();
        serialBuffer.clear();
    }
    handle_serialClosed();
}

void MainWindow::handle_serialConnectBtn()
{
    if (ui->pushButton_serialConnect->text() == "Connect")
    {
        do_openSerial();
    }
    else
    {
        do_closeSerial();
    }
}

void MainWindow::handle_serialOpened()
{
    ui->textBrowser_console->append(timeStamp() + "Serial port opened x)");
    ui->pushButton_serialConnect->setText("Disconnect");
    ui->comboBox_serialPort->setEnabled(false);
    ui->pushButton_serialRefresh->setEnabled(false);
    ui->comboBox_baud->setEnabled(false);
    ui->comboBox_bytesize->setEnabled(false);
    ui->comboBox_parity->setEnabled(false);
    ui->comboBox_stopbits->setEnabled(false);
    ui->radioButton_DsrDtr->setEnabled(false);
    ui->radioButton_RtsCts->setEnabled(false);
    ui->radioButton_XonXoff->setEnabled(false);
}

void MainWindow::handle_serialClosed()
{
    ui->textBrowser_console->append(timeStamp() + "Serial port closed :D");
    ui->pushButton_serialConnect->setText("Connect");
    ui->comboBox_serialPort->setEnabled(true);
    ui->pushButton_serialRefresh->setEnabled(true);
    ui->comboBox_baud->setEnabled(true);
    ui->comboBox_bytesize->setEnabled(true);
    ui->comboBox_parity->setEnabled(true);
    ui->comboBox_stopbits->setEnabled(true);
    ui->radioButton_DsrDtr->setEnabled(true);
    ui->radioButton_RtsCts->setEnabled(true);
    ui->radioButton_XonXoff->setEnabled(true);
}

void MainWindow::handle_selectFileBtn()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), "", tr("HPGL Files (*.hpgl *.HPGL)"));

    ui->lineEdit_filePath->setText(fileName);
}

int MainWindow::get_nextInt(QString input, int * index)
{
    QChar tmp = input[*index];
    QString buffer = "";

    while (tmp != ',' && tmp != ';')
    {
        buffer.append(tmp);
        tmp = input[++*index];
    }
    return(atoi(buffer.toStdString().c_str()));
}

void MainWindow::do_loadFile()
{
    if (inputFile.isOpen())
    {
        inputFile.close();
    }
    inputFile.setFileName(ui->lineEdit_filePath->text());
    inputFile.open(QIODevice::ReadOnly);
    cmdList.clear();
    ui->textBrowser_read->clear();
//    while (!inputFile.atEnd())
//    {
//        QString buffer = "";
//        char tmp = 0;
//        while (tmp != ';' && !inputFile.atEnd())
//        {
//            inputFile.read(&tmp, 1);
//            buffer += tmp;
//        }
//        if (buffer.endsWith(";"))
//        {
//            hpgl_cmd aparser = hpgl_cmd(buffer);
//            cmdList.append(aparser);
//            ui->textBrowser_read->append(cmdList.last().print());
//        }
//    }

    QString buffer = "";
    QTextStream fstream(&inputFile);
    buffer = fstream.readAll();
    buffer.remove('\n');
    int numCmds = buffer.count(';');
    qDebug() << "File: " << buffer;
    for (int i = 0; i < numCmds; i++)
    {
        QString tmp;
        tmp = buffer.section(';', i, i);
        cmdList.push_back(hpgl_cmd(tmp));
        qDebug() << "Just added: " << cmdList.back().print();
    }

    // Set up new graphics view.

    plotScene.clear();

//    int curX, curY;
//    for (int i = 0; i < cmdList.count(); i++)
//    {
//        if (cmdList.at(i)[0] == 'P' && cmdList.at(i)[1] == 'D')
//        {
//            ui->textBrowser_console->append("Found PD: " + cmdList.at(i));
//            int charIndex = 1;
//            int nextX, nextY;
//            while (cmdList.at(i)[charIndex] != ';')
//            {
//                charIndex++;
//                nextX = get_nextInt(cmdList.at(i), &charIndex);
//                charIndex++;
//                nextY = get_nextInt(cmdList.at(i), &charIndex);

//                plotScene.addLine(curX, -curY, nextX, -nextY);
//                ui->textBrowser_console->append(timeStamp() + "Adding line [" +
//                                                QString::number(curX) + "," + QString::number(curY) +
//                                                "] to [" + QString::number(nextX) + "," +
//                                                QString::number(nextY) + "].");
//                curX = nextX;
//                curY = nextY;
//            }
//        }
//        else if (cmdList.at(i)[0] == 'P' && cmdList.at(i)[1] == 'U')
//        {
//            ui->textBrowser_console->append("Found PU: " + cmdList.at(i));
//            int charIndex = 1;
//            while (cmdList.at(i)[charIndex] != ';')
//            {
//                charIndex++;
//                curX = get_nextInt(cmdList.at(i), &charIndex);
//                charIndex++;
//                curY = get_nextInt(cmdList.at(i), &charIndex);
//            }
//        }
//    }

    ui->graphicsView_view->setSceneRect(plotScene.sceneRect());
    ui->graphicsView_view->show();
}

void MainWindow::do_plot()
{
    if (!serialBuffer->isOpen() || cmdList.isEmpty())
    {
        ui->textBrowser_console->append(timeStamp() + "Can't plot!");
        return;
    }
//    for (int i = 0; i < cmdList.count(); i++)
//    {
//        int size = cmdList.at(i).length();
//        serialBuffer->write(cmdList.at(i).toStdString().c_str(), size);
//    }
}








