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

    // Declare threads
    plotter = new Plotter;
    plotter->moveToThread(&threadPlotter);

    // Instantiate settings object
    init_localplot_settings();
    settings = new QSettings();

    // Connect actions
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(do_openDialogAbout()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(do_openDialogSettings()));

    // Connect threads
    connect(ui->pushButton_serialConnect, SIGNAL(clicked()), this, SLOT(handle_serialConnectBtn()));
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    connect(plotter, SIGNAL(serialOpened()), this, SLOT(handle_serialOpened()));
    connect(plotter, SIGNAL(serialClosed()), this, SLOT(handle_serialClosed()));
    connect(this, SIGNAL(please_plotter_openSerial()), plotter, SLOT(do_openSerial()));
    connect(this, SIGNAL(please_plotter_closeSerial()), plotter, SLOT(do_closeSerial()));
    connect(this, SIGNAL(please_plotter_doPlot(QList<hpgl_obj>)), plotter, SLOT(do_beginPlot(QList<hpgl_obj>)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), plotter, SLOT(do_cancelPlot()));
    connect(plotter, SIGNAL(donePlotting()), this, SLOT(handle_plotCancelled()));
    connect(plotter, SIGNAL(startedPlotting()), this, SLOT(handle_plotStarted()));
    connect(plotter, SIGNAL(plottingProgress(int)), this, SLOT(handle_plottingPercent(int)));

    // Set up the drawing pens
    upPen.setStyle(Qt::DotLine);
    do_updatePens();

    connect(ui->lineEdit_filePath, SIGNAL(editingFinished()), this, SLOT(update_filePath()));

    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalSizeChanged(QSizeF)),
            this, SLOT(do_drawView())); // Update view if the pixel DPI changes

    connect(ui->doubleSpinBox_objScale, SIGNAL(valueChanged(double)),
            this, SLOT(handle_objectTransform())); // Update view if the scale changes
    connect(ui->doubleSpinBox_objRotation, SIGNAL(valueChanged(double)),
            this, SLOT(handle_objectTransform())); // Update view if the rotation changes
    connect(ui->spinBox_objTranslationX, SIGNAL(valueChanged(int)),
            this, SLOT(handle_objectTransform())); // Update view if the translation-X changes
    connect(ui->spinBox_objTranslationY, SIGNAL(valueChanged(int)),
            this, SLOT(handle_objectTransform())); // Update view if the translation-Y changes

    ui->graphicsView_view->setScene(&plotScene);

    ui->lineEdit_filePath->setText(settings->value("mainwindow/filePath", SETDEF_MAINWINDOW_FILEPATH).toString());

//    hpgl_obj obj_tmp("IN;SP0;PD10,10;PU100,100;IN");
//    QList<hpgl_obj> list_tmp;
//    list_tmp.push_back(hpgl_obj("IN;SP0;PD10,10;PU100,100;IN"));
//    qDebug() << "Time: " << list_tmp[0].time(3);

    do_drawView();

    // Kickstart threads
    threadPlotter.start();
    plotter->do_run(); // Auto start plotter
}

MainWindow::~MainWindow()
{
    plotter->do_closeSerial();

    threadPlotter.quit();
    threadPlotter.wait();

    delete settings;
    delete ui;
}

QString MainWindow::timeStamp()
{
    return(QTime::currentTime().toString("[HH:mm:ss:zzz] "));
}

void MainWindow::do_openDialogAbout()
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

void MainWindow::do_openDialogSettings()
{
    DialogSettings * newwindow;
    newwindow = new DialogSettings(this);
    newwindow->setWindowTitle("localplot settings");
    newwindow->exec();
}

void MainWindow::update_filePath()
{
    settings->setValue("mainwindow/filePath", ui->lineEdit_filePath->text());
}

void MainWindow::handle_serialConnectBtn()
{
    if (ui->pushButton_serialConnect->text() == "Connect")
    {
        emit please_plotter_openSerial(); //do_openSerial();
    }
    else
    {
        emit please_plotter_closeSerial(); //do_closeSerial();
    }
}

void MainWindow::handle_serialOpened()
{
//    disconnect(ui->pushButton_serialConnect, 0, 0, 0);
    ui->pushButton_doPlot->setEnabled(true);
    ui->textBrowser_console->append(timeStamp() + "Serial port opened x)");
    ui->pushButton_serialConnect->setText("Disconnect");
//    connect(ui->pushButton_serialConnect, SIGNAL(clicked()), plotter, SLOT(do_closeSerial()));
}

void MainWindow::handle_serialClosed()
{
//    disconnect(ui->pushButton_serialConnect, 0, 0, 0);
    ui->pushButton_doPlot->setEnabled(false);
    ui->textBrowser_console->append(timeStamp() + "Serial port closed :D");
    ui->pushButton_serialConnect->setText("Connect");
//    connect(ui->pushButton_serialConnect, SIGNAL(clicked()), plotter, SLOT(do_openSerial()));
}

void MainWindow::do_plot()
{
    qDebug() << "trying to plot.";
    emit please_plotter_doPlot(objList);
}

void MainWindow::do_cancelPlot()
{
    qDebug() << "Trying to cancel plot.";
    emit please_plotter_cancelPlot();
}

void MainWindow::handle_plotStarted()
{
    disconnect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    ui->pushButton_doPlot->setText("Cancel");
    ui->progressBar_plotting->setEnabled(true);
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_cancelPlot()));
}

void MainWindow::handle_plotCancelled()
{
    disconnect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_cancelPlot()));
    ui->pushButton_doPlot->setText("Plot!");
    ui->progressBar_plotting->setValue(0);
    ui->progressBar_plotting->setEnabled(false);
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
}

void MainWindow::handle_selectFileBtn()
{
    QString fileName;
    QString startDir = settings->value("mainwindow/filePath", "").toString();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), startDir, tr("HPGL Files (*.hpgl *.HPGL)"));

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

/*
 * Returns integer from section of string.
 */
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
    penSize = settings->value("pen/down/size", SETDEF_PEN_DOWN_SIZE).toInt();
    rgbColor[0] = settings->value("pen/down/red", SETDEF_PEN_DOWN_RED).toInt();
    rgbColor[1] = settings->value("pen/down/green", SETDEF_PEN_DOWN_GREEN).toInt();
    rgbColor[2] = settings->value("pen/down/blue", SETDEF_PEN_DOWN_BLUE).toInt();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize);

    // Set upPen
    penSize = settings->value("pen/up/size", SETDEF_PEN_UP_SIZE).toInt();
    rgbColor[0] = settings->value("pen/up/red", SETDEF_PEN_UP_RED).toInt();
    rgbColor[1] = settings->value("pen/up/green", SETDEF_PEN_UP_GREEN).toInt();
    rgbColor[2] = settings->value("pen/up/blue", SETDEF_PEN_UP_BLUE).toInt();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    upPen.setColor(penColor);
    upPen.setWidth(penSize);
}

void MainWindow::handle_plottingPercent(int percent)
{
//    ui->textBrowser_console->append(QString::number(percent)+"% complete.");
    ui->progressBar_plotting->setValue(percent);
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
    double scale = 1.0;
    // Factor is the conversion from HP Graphic Unit to pixels
    double xFactor = (xDpi / 1016.0 * scale);
    double yFactor = (yDpi / 1016.0 * scale);

    QList<QLine> lines_down;
    lines_down.clear();
    QList<QLine> lines_up;
    lines_up.clear();

    double time = 0;

    for (int i = 0; i < objList.length(); i++)
    {
        // Build ETA
        for (int i_cmd = 0; i_cmd < objList[i].cmdCount(); i_cmd++)
        {
            qDebug() << "debug: " << i << i_cmd;
            time += objList[i].time(i_cmd);
            qDebug() << "test time: " << time;
        }
        ui->textBrowser_console->append("Estimated cut time: "+QString::number(time)+" seconds.");

        // Get a list of qlines
        objList[i].gen_line_lists();
        lines_down = objList[i].lineListDown;
        lines_up = objList[i].lineListUp;

        // Transform qlines to be upright
        for (int i_down = 0; i_down < lines_down.length(); i_down++)
        {
            int x, y;
            x = lines_down[i_down].x1();
            y = lines_down[i_down].y1();
            x = x*xFactor;
            y = y*(-1)*yFactor;
            lines_down[i_down].setP1(QPoint(x, y));
            x = lines_down[i_down].x2();
            x = x*xFactor;
            y = lines_down[i_down].y2();
            y = y*(-1)*yFactor;
            lines_down[i_down].setP2(QPoint(x, y));
        }

        for (int i_up = 0; i_up < lines_up.length(); i_up++)
        {
            int x, y;
            x = lines_up[i_up].x1();
            x = x*xFactor;
            y = lines_up[i_up].y1();
            y = y*(-1)*yFactor;
            lines_up[i_up].setP1(QPoint(x, y));
            x = lines_up[i_up].x2();
            x = x*xFactor;
            y = lines_up[i_up].y2();
            y = y*(-1)*yFactor;
            lines_up[i_up].setP2(QPoint(x, y));
        }

        // Write qlines to the scene
        for (int i_write = 0; i_write < objList.length(); i_write++)
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
    QString scaleText = "View Scale: " + QString::number(scale);
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
    hpgl_obj tmp(buffer);
    objList.push_back(tmp);

    settings->setValue("mainwindow/filePath", filePath);

    do_drawView();
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































