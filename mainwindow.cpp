#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialogabout.h"
#include "ui_dialogabout.h"

#include "dialogsettings.h"
#include "ui_dialogsettings.h"

/**
 * Reference: http://cstep.luberth.com/HPGL.pdf
 *
 * Language Structure:
 * Using Inkscape's default format -> XXy1,y1,y2,y2;
 * Two uppercase characters followed by a CSV list
 * and terminated with a semicolon.
 *
 * Path Vertex Object:
 * State: up or down
 * Vertex coordinate: in graphic units (1/1016") (0.025mm)
 *
 * Program file structure
 * hpgl_cmd - Structure for storing a single hpgl command
 * hpgl_obj - An hpgl object, or cluster of commands that share similar
 *             properties and transformations.
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Instantiate settings object
    init_localplot_settings();
    settings = new QSettings();

    // Connect actions
    connect(ui->pushButton_serialConnect, SIGNAL(clicked()), this, SLOT(handle_serialConnectBtn()));
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(open_dialogAbout()));

    // Set up the drawing pens
    upPen.setStyle(Qt::DotLine);
    do_updatePens();

    connect(ui->lineEdit_filePath, SIGNAL(editingFinished()), this, SLOT(update_filePath()));

    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalSizeChanged(QSizeF)),
            this, SLOT(do_drawView())); // Update view if the pixel DPI changes

    connect(ui->doubleSpinBox_objScale, SIGNAL(valueChanged(double)),
            this, SLOT(handle_objectTransform())); // Update view if the scale changes
    connect(ui->doubleSpinBox_objRotation, SIGNAL(valueChanged(double)),
            this, SLOT(handle_objectTransform())); // Update view if the scale changes
    connect(ui->spinBox_objTranslationX, SIGNAL(valueChanged(int)),
            this, SLOT(handle_objectTransform())); // Update view if the scale changes
    connect(ui->spinBox_objTranslationY, SIGNAL(valueChanged(int)),
            this, SLOT(handle_objectTransform())); // Update view if the scale changes

    ui->graphicsView_view->setScene(&plotScene);

    ui->lineEdit_filePath->setText(settings->value("mainwindow/filePath", "").toString());

    do_drawView();
//    do_drawDemoView();
}

MainWindow::~MainWindow()
{
    do_closeSerial();

    delete settings;
    delete ui;
}

QString MainWindow::timeStamp()
{
    return(QTime::currentTime().toString("[HH:mm:ss:zzz] "));
}

void MainWindow::open_dialogAbout()
{
    // This way of doing things will allow the QDialog to be reused. (?)
//    QDialog * widget = new QDialog;
//    Ui::DialogAbout about_ui;
//    about_ui.setupUi(widget);
//    widget->exec();

    // This way will make a application specific DialogAbout.
    DialogAbout * newwindow;
    newwindow = new DialogAbout(this);
    newwindow->setWindowTitle("About localplot");
    connect(newwindow, SIGNAL(please_close()), newwindow, SLOT(close()));
    newwindow->exec();
}

void MainWindow::open_dialogSettings()
{
    DialogSettings * newwindow;
    newwindow = new DialogSettings(this);
    newwindow->setWindowTitle("localplot settings");
    newwindow->exec();
}

void MainWindow::update_penDown()
{
    settings->beginGroup("pen/down");
    settings->setValue("size", settings->value("pen/down/size", 2).toInt());
    settings->setValue("red", settings->value("pen/down/red", 100).toInt());
    settings->setValue("green", settings->value("pen/down/green", 150).toInt());
    settings->setValue("blue", settings->value("pen/down/blue", 200).toInt());
    settings->endGroup();
}

void MainWindow::update_penUp()
{
    settings->beginGroup("pen/up");
    settings->setValue("size", settings->value("pen/up/size", 1).toInt());
    settings->setValue("red", settings->value("pen/up/red", 250).toInt());
    settings->setValue("green", settings->value("pen/up/green", 150).toInt());
    settings->setValue("blue", settings->value("pen/up/blue", 150).toInt());
    settings->endGroup();
    do_drawView();
}

void MainWindow::update_filePath()
{
    settings->beginGroup("mainwindow");
    settings->setValue("filePath", ui->lineEdit_filePath->text());
    settings->endGroup();
}

void MainWindow::do_openSerial()
{
    int _portIndex = settings->value("serial/port", 0).toInt(); //ui->comboBox_serialPort->currentIndex();
    QSerialPortInfo _device;

    if (_portIndex >= 0 && serialPorts.availablePorts().count() > _portIndex)
        _device = serialPorts.availablePorts().at(_portIndex);
    else
    {
        qDebug() << "Serial list needs to be refreshed or something";
        do_closeSerial();
        return;
    }
    if (!serialBuffer.isNull())
    {
        serialBuffer.clear();
        handle_serialClosed();
    }
    serialBuffer = new QSerialPort(_device);
    serialBuffer->setBaudRate(atoi(settings->value("serial/baud", 9600).toString().toStdString().c_str()));

    int dataBits = atoi(settings->value("serial/bytesize", "").toString().toStdString().c_str());
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

    QString parity = settings->value("serial/parity").toString();
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

    QString stopBits = settings->value("serial/stopbits", 0).toString();
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

    if (settings->value("serial/xonxoff", false).toBool())
    {
        serialBuffer->setFlowControl(QSerialPort::SoftwareControl);
    }
    else if (settings->value("serial/rtscts", false).toBool())
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
}

void MainWindow::handle_serialClosed()
{
    ui->textBrowser_console->append(timeStamp() + "Serial port closed :D");
    ui->pushButton_serialConnect->setText("Connect");
}

void MainWindow::handle_selectFileBtn()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), "", tr("HPGL Files (*.hpgl *.HPGL)"));

    ui->lineEdit_filePath->setText(fileName);
    do_loadFile();
}

//void MainWindow::handle_autoTranslateBtn()
//{
//    for (int i = 0; i < objList.count(); i++)
//    {
//        if (objList[i].minX() < 0)
//        {
//            int val = ui->spinBox_objTranslationX
//        }
//    }
//}

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
    penSize = settings->value("pen/down/size", 2).toInt(); //ui->spinBox_downPen_size->value();
    rgbColor[0] = settings->value("pen/down/red", 0).toInt(); //ui->spinBox_downPen_red->value();
    rgbColor[1] = settings->value("pen/down/green", 0).toInt(); //ui->spinBox_downPen_green->value();
    rgbColor[2] = settings->value("pen/down/blue", 0).toInt(); //ui->spinBox_downPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize);

    // Set upPen
    penSize = settings->value("pen/up/size", 1).toInt(); //ui->spinBox_upPen_size->value();
    rgbColor[0] = settings->value("pen/up/red", 1).toInt(); //ui->spinBox_upPen_red->value();
    rgbColor[1] = settings->value("pen/up/green", 1).toInt(); //ui->spinBox_upPen_green->value();
    rgbColor[2] = settings->value("pen/up/blue", 1).toInt(); //ui->spinBox_upPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    upPen.setColor(penColor);
    upPen.setWidth(penSize);
}

void MainWindow::do_drawView()
{
    // Set up new graphics view.
    plotScene.clear();

    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();

    // Draw origin
    QPen originPen;
    originPen.setColor(QColor(150, 150, 150));
    originPen.setWidth(2);
    plotScene.addLine(0, 0, xDpi, 0, originPen);
    plotScene.addLine(0, 0, 0, -yDpi, originPen);

    do_updatePens();

    // scale is the value set by our user
    //double scale = ui->doubleSpinBox_objScale->value();
    double scale = 1.0;
    // Factor is the conversion from HP Graphic Unit to pixels
    double xFactor = (xDpi / 1016.0 * scale);
    double yFactor = (yDpi / 1016.0 * scale);

    QList<QLine> lines_down;
    lines_down.clear();
    QList<QLine> lines_up;
    lines_up.clear();

    for (int i = 0; i < objList.length(); i++)
    {
        // Get a list of qlines
        objList[i].gen_line_lists();
        lines_down = objList[i].lineListDown;
        lines_up = objList[i].lineListUp;

        // Transform qlines to be upright
        for (int i = 0; i < lines_down.length(); i++)
        {
            int x, y;
            x = lines_down[i].x1();
            y = lines_down[i].y1();
            x = x*xFactor;
            y = y*(-1)*yFactor;
            lines_down[i].setP1(QPoint(x, y));
            x = lines_down[i].x2();
            x = x*xFactor;
            y = lines_down[i].y2();
            y = y*(-1)*yFactor;
            lines_down[i].setP2(QPoint(x, y));
        }

        for (int i = 0; i < lines_up.length(); i++)
        {
            int x, y;
            x = lines_up[i].x1();
            x = x*xFactor;
            y = lines_up[i].y1();
            y = y*(-1)*yFactor;
            lines_up[i].setP1(QPoint(x, y));
            x = lines_up[i].x2();
            x = x*xFactor;
            y = lines_up[i].y2();
            y = y*(-1)*yFactor;
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

    // Draw origin text
    QGraphicsTextItem * label = plotScene.addText("Front of Plotter");
    label->setRotation(90);
    QRectF labelRect = label->boundingRect();
    label->setY(label->y() - labelRect.width());
    plotScene.addText("(0,0)");
    QString scaleText = "Scale: " + QString::number(scale);
    QGraphicsTextItem * scaleTextItem = plotScene.addText(scaleText);
    QRectF scaleTextItemRect = scaleTextItem->boundingRect();
    scaleTextItem->setY(scaleTextItem->y() + scaleTextItemRect.height());

    // Set scene rectangle to match new items
    plotScene.setSceneRect(plotScene.itemsBoundingRect());
    //plotScene.addRect(plotScene.sceneRect(), downPen);

    // Set scene to view
    ui->graphicsView_view->setSceneRect(plotScene.sceneRect());
    ui->graphicsView_view->show();
}

void MainWindow::do_loadFile()
{
    if (inputFile.isOpen())
    {
        inputFile.close();
    }
    QString filePath = ui->lineEdit_filePath->text();
    if (filePath.isEmpty())
    {
        return;
    }
    inputFile.setFileName(filePath);
    inputFile.open(QIODevice::ReadOnly);
    objList.clear();
    //ui->textBrowser_read->clear();

    QString buffer = "";
    QTextStream fstream(&inputFile);
    buffer = fstream.readAll();
    objList.push_back(hpgl_obj(buffer));

    settings->setValue("MainWindow/filePath", filePath);

    do_drawView();
}

void MainWindow::do_plot()
{
    // Variables
    int cutterSpeed = settings->value("cutter/speed", 80).toInt(); //ui->spinBox_cutterSpeed->value(); // in mm/s
    hpgl_obj obj;
    QString printThis;
    int cmdCount;
    QString command;
    double time;
    QProcess process;

    qDebug() << "Plotting file!";
    if (serialBuffer.isNull() || !serialBuffer->isOpen() || objList.isEmpty())
    {
        ui->textBrowser_console->append(timeStamp() + "Can't plot!");
        return;
    }
    for (int i = 0; i < objList.count(); i++)
    {
        obj = objList.at(i);
        cmdCount = obj.cmdCount();
        for (int cmd_index = 0; cmd_index < cmdCount; cmd_index++)
        {
            printThis = obj.cmdPrint(cmd_index);
            if (printThis == "OOB")
            {
                ui->textBrowser_console->append("ERROR: Object Out Of Bounds! Cannot Plot! D:");
                //ui->textBrowser_console->append("(try the auto translation button)");
                ui->textBrowser_console->append("(An X or Y value is less than zero)");
                return;
            }
            serialBuffer->write(printThis.toStdString().c_str());
            if (settings->value("cutter/incremental", 1).toInt())
            {
                serialBuffer->flush();
                command = "sleep ";
                time = obj.cmdMM(cmd_index) / cutterSpeed;
                if (time == 0)
                    continue;
                command += QString::number(time);
                qDebug() << "Starting sleep command: sleep " << time;
                process.start(command);
                process.waitForFinished(60000); // Waits for up to 60s
                qDebug() << "Done with sleep command";
            }
        }
    }
    qDebug() << "Done plotting.";
}

void MainWindow::handle_objectTransform()
{
    QTransform Tscale, Trotate, Ttranslate;
    double scale = ui->doubleSpinBox_objScale->value();
    double rotation = ui->doubleSpinBox_objRotation->value();
    int translateX = ui->spinBox_objTranslationX->value();
    int translateY = ui->spinBox_objTranslationY->value();

    Tscale.scale(scale, scale);
    Trotate.rotate(rotation);
    Ttranslate.translate(translateX, translateY);

    //qDebug() << "MATRIX: " << transform;

    for (int i = 0; i < objList.count(); i++)
    {
        objList[i].cmdTransformScale = Tscale;
        objList[i].cmdTransformRotate = Trotate;
        objList[i].cmdTransformTranslate = Ttranslate;
    }
    do_drawView();
}































