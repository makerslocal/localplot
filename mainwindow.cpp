#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_serialRefresh, SIGNAL(clicked()), this, SLOT(do_refreshSerialList()));
    connect(ui->pushButton_serialConnect, SIGNAL(clicked()), this, SLOT(handle_serialConnectBtn()));

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
}

MainWindow::~MainWindow()
{
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
}

void MainWindow::handle_serialClosed()
{
    ui->textBrowser_console->append(timeStamp() + "Serial port closed :D");
    ui->pushButton_serialConnect->setText("Connect");
    ui->comboBox_serialPort->setEnabled(true);
    ui->pushButton_serialRefresh->setEnabled(true);
}









