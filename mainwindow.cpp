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

    // Set up the two drawing pens
    upPen.setStyle(Qt::DotLine);
    do_updatePens();

    // Update pens on ui change
    connect(ui->spinBox_downPen_size, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_downPen_red, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_downPen_green, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_downPen_blue, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_upPen_size, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_upPen_red, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_upPen_green, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));
    connect(ui->spinBox_upPen_blue, SIGNAL(valueChanged(int)), this, SLOT(do_drawView()));

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

void MainWindow::do_updatePens()
{
    // Variables
    int rgbColor[3];
    int penSize;
    QColor penColor;

    // Set downPen
    penSize = ui->spinBox_downPen_size->value();
    rgbColor[0] = ui->spinBox_downPen_red->value();
    rgbColor[1] = ui->spinBox_downPen_green->value();
    rgbColor[2] = ui->spinBox_downPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize);

    // Set upPen
    penSize = ui->spinBox_upPen_size->value();
    rgbColor[0] = ui->spinBox_upPen_red->value();
    rgbColor[1] = ui->spinBox_upPen_green->value();
    rgbColor[2] = ui->spinBox_upPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    upPen.setColor(penColor);
    upPen.setWidth(penSize);
}

void MainWindow::do_drawView()
{
    do_updatePens();

    int xScale = ui->graphicsView_view->physicalDpiX();
    int yScale = ui->graphicsView_view->physicalDpiY();

    qDebug() << "physical: x:" << xScale << " y:" << yScale;

    // Set up new graphics view.
    plotScene.clear();

    QList<QLine> lines_down;
    lines_down.clear();
    QList<QLine> lines_up;
    lines_up.clear();


    for (int i = 0; i < objList.length(); i++)
    {
        // Get a list of qlines
        lines_down = objList[i].line_list_down();
        lines_up = objList[i].line_list_up();

        // Transform qlines to be upright
        for (int i = 0; i < lines_down.length(); i++)
        {
            int x, y;
            x = lines_down[i].x1();
            y = lines_down[i].y1();
            x = x*(xScale/1016.0);
            y = y*-1*(yScale/1016.0);
            lines_down[i].setP1(QPoint(x, y));
            x = lines_down[i].x2();
            x = x*(xScale/1016.0);
            y = lines_down[i].y2();
            y = y*-1*(yScale/1016.0);
            lines_down[i].setP2(QPoint(x, y));
        }

        for (int i = 0; i < lines_up.length(); i++)
        {
            int x, y;
            x = lines_up[i].x1();
            x = x*(xScale/1016);
            y = lines_up[i].y1();
            y = y*-1*(yScale/1016);
            lines_up[i].setP1(QPoint(x, y));
            x = lines_up[i].x2();
            x = x*(xScale/1016);
            y = lines_up[i].y2();
            y = y*-1*(yScale/1016);
            lines_up[i].setP2(QPoint(x, y));
        }

        // Write qlines to the scene
        for (int i = 0; i < objList.length(); i++)
        {
            for (int l = 0; l < lines_down.length(); l++)
            {
                plotScene.addLine(lines_down[l], downPen);
            }
            for (int l = 0; l < lines_up.length(); l++)
            {
                plotScene.addLine(lines_up[l], upPen);
            }
        }
    }

    //plotScene.addRect(plotScene.sceneRect(), downPen);

    ui->graphicsView_view->setSceneRect(plotScene.sceneRect());
    ui->graphicsView_view->show();
}

void MainWindow::do_loadFile()
{
    if (inputFile.isOpen())
    {
        inputFile.close();
    }
    inputFile.setFileName(ui->lineEdit_filePath->text());
    inputFile.open(QIODevice::ReadOnly);
    //objList.clear();
    //ui->textBrowser_read->clear();

    QString buffer = "";
    QTextStream fstream(&inputFile);
    buffer = fstream.readAll();
    objList.push_back(hpgl_obj(buffer));

    do_drawView();
}

void MainWindow::do_plot()
{
    qDebug() << "Plotting file!";
    if (serialBuffer.isNull())
    {
        for (int i = 0; i < objList.count(); i++)
        {
            hpgl_obj obj = objList.at(i);
            int size = obj.printLen();
            for (int d = 0; d < size; d++)
            {
                qDebug() << "Raw output: " << obj.print();
            }
        }
        return;
    }
    if (!serialBuffer->isOpen() || objList.isEmpty())
    {
        ui->textBrowser_console->append(timeStamp() + "Can't plot!");
        return;
    }
    for (int i = 0; i < objList.count(); i++)
    {
        hpgl_obj obj = objList.at(i);
//        int size = obj.printLen();
        QString printThis = obj.print();
//        serialBuffer->write(cmdList.at(i).toStdString().c_str(), size);
        serialBuffer->write(printThis.toStdString().c_str());
//        for (int d = 0; d < size; d++)
//        {
//            qDebug() << "Raw output: " << cmd.print()[d];
//        }
    }
}








