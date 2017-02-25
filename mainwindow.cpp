/**
 * Localplot - Almost useful!
 * Christopher Bero <bigbero@gmail.com>
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialogabout.h"
#include "ui_dialogabout.h"

#include "dialogsettings.h"
#include "ui_dialogsettings.h"

/**
 * HPGL reference: http://cstep.luberth.com/HPGL.pdf
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

/**
 * @brief MainWindow::MainWindow
 * Instantiate main UI window.
 * @param parent - Qt specific
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QSettings settings;

    ui->setupUi(this);

    // Declare worker thread
    ancilla = new AncillaryThread;

    // Connect UI actions
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(do_openDialogAbout()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(do_openDialogSettings()));

    // Connect thread
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    connect(ancilla, SIGNAL(plottingProgress(int)), this, SLOT(handle_plottingPercent(int)));
    connect(ancilla, &AncillaryThread::started, this, &MainWindow::handle_ancillaThreadStart);
    connect(ancilla, &AncillaryThread::finished, this, &MainWindow::handle_ancillaThreadQuit);
    connect(ancilla, SIGNAL(hpglParsingDone()), this, SLOT(sceneSetSceneRect()));
    connect(ancilla, SIGNAL(statusUpdate(QString)), this, SLOT(handle_ancillaThreadStatus(QString)));
    connect(ancilla, SIGNAL(newPolygon(QPolygonF)), this, SLOT(addPolygon(QPolygonF)));
    connect(this, SIGNAL(please_plotter_doPlot(QList<hpgl>*)), ancilla, SLOT(do_beginPlot(QList<hpgl>*)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), ancilla, SLOT(do_cancelPlot()));
    connect(this, SIGNAL(please_plotter_loadFile(QString)), ancilla, SLOT(load_file(QString)));

    // Set up the drawing pens
    upPen.setStyle(Qt::DotLine);
    do_updatePens();

    connect(ui->lineEdit_filePath, SIGNAL(editingFinished()), this, SLOT(update_filePath()));

    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalDotsPerInchChanged(qreal)),
            this, SLOT(do_drawView())); // Update view if the pixel DPI changes

    ui->graphicsView_view->setScene(&plotScene);

    ui->lineEdit_filePath->setText(
                settings.value("mainwindow/filePath",
                                SETDEF_MAINWINDOW_FILEPATH).toString());

    do_drawView();

    // Kickstart worker thread
    ancilla->start();
}

MainWindow::~MainWindow()
{
    ancilla->quit();
    ancilla->wait();

    delete ui;
}

/*******************************************************************************
 * Child windows
 ******************************************************************************/

/**
 * @brief MainWindow::do_openDialogAbout
 * Creates a window with information about the program and authors.
 */
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

/**
 * @brief MainWindow::do_openDialogSettings
 * Creates a window to modify QSettings
 */
void MainWindow::do_openDialogSettings()
{
    DialogSettings * newwindow;
    newwindow = new DialogSettings(this);
    newwindow->setWindowTitle("localplot settings");
    newwindow->exec();
}

/*******************************************************************************
 * UI Slots
 ******************************************************************************/

/**
 * @brief MainWindow::update_filePath
 * Stores the current file in QSettings
 */
void MainWindow::update_filePath()
{
    QSettings settings;
    settings.setValue("mainwindow/filePath", ui->lineEdit_filePath->text());
}

void MainWindow::do_updatePens()
{
    // Variables
    QSettings settings;
    int rgbColor[3];
    int penSize;
    QColor penColor;

    // Set downPen
    penSize = settings.value("pen/down/size", SETDEF_PEN_DOWN_SIZE).toInt();
    rgbColor[0] = settings.value("pen/down/red", SETDEF_PEN_DOWN_RED).toInt();
    rgbColor[1] = settings.value("pen/down/green", SETDEF_PEN_DOWN_GREEN).toInt();
    rgbColor[2] = settings.value("pen/down/blue", SETDEF_PEN_DOWN_BLUE).toInt();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize);

    // Set upPen
    penSize = settings.value("pen/up/size", SETDEF_PEN_UP_SIZE).toInt();
    rgbColor[0] = settings.value("pen/up/red", SETDEF_PEN_UP_RED).toInt();
    rgbColor[1] = settings.value("pen/up/green", SETDEF_PEN_UP_GREEN).toInt();
    rgbColor[2] = settings.value("pen/up/blue", SETDEF_PEN_UP_BLUE).toInt();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    upPen.setColor(penColor);
    upPen.setWidth(penSize);
}

/*******************************************************************************
 * Worker thread slots
 ******************************************************************************/

void MainWindow::handle_ancillaThreadStart()
{
    ui->textBrowser_console->append(timeStamp() + "Ancillary thread started \\o/");
}

void MainWindow::handle_ancillaThreadQuit()
{
    ui->textBrowser_console->append(timeStamp() + "Ancillary thread stopped.");
}

void MainWindow::handle_ancillaThreadStatus(QString _consoleText)
{
    ui->textBrowser_console->append(timeStamp() + _consoleText);
}

void MainWindow::do_plot()
{
    emit please_plotter_doPlot();
}

void MainWindow::do_cancelPlot()
{
    emit please_plotter_cancelPlot();
}

void MainWindow::handle_plotStarted()
{
    disconnect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    ui->pushButton_doPlot->setText("Cancel");
    ui->progressBar_plotting->setValue(0);
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_cancelPlot()));
    ui->textBrowser_console->append(timeStamp() + "Plotting started.");
}

void MainWindow::handle_plotCancelled()
{
    disconnect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_cancelPlot()));
    ui->pushButton_doPlot->setText("Plot!");
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    ui->textBrowser_console->append(timeStamp() + "Plotting cancelled.");
}

void MainWindow::handle_plotFinished()
{
    disconnect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_cancelPlot()));
    ui->pushButton_doPlot->setText("Plot!");
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    ui->textBrowser_console->append(timeStamp() + "Plotting Done.");
}

void MainWindow::handle_selectFileBtn()
{
    QSettings settings;
    QString filePath;
    QString startDir = settings.value("mainwindow/filePath", "").toString();

    filePath = QFileDialog::getOpenFileName(this,
        tr("Open File"), startDir, tr("HPGL Files (*.hpgl *.HPGL)"));

    ui->lineEdit_filePath->setText(filePath);
    do_loadFile(filePath);
}

void MainWindow::handle_plottingPercent(int percent)
{
    ui->progressBar_plotting->setValue(percent);
}

/*******************************************************************************
 * Etcetera methods
 ******************************************************************************/

void MainWindow::do_drawView()
{
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();

    // Transforms
    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    // Set up new graphics view.
    plotScene.clear();

    // Draw origin
    QPen originPen;
    originPen.setColor(QColor(150, 150, 150));
    originPen.setWidth(2);
    plotScene.addLine(0, 0, xDpi, 0, originPen)->setTransform(itemToScene);
    plotScene.addLine(0, 0, 0, yDpi, originPen)->setTransform(itemToScene);

    do_updatePens();

    // Draw origin text
    QGraphicsTextItem * label = plotScene.addText("Front of Plotter");
    label->setTransform(itemToScene * viewFlip);
    label->setRotation(-90);
    label->moveBy(-1 * label->mapRectToScene(label->boundingRect()).width(), 0);

    QGraphicsTextItem * originText = plotScene.addText("(0,0)");
    originText->setTransform(itemToScene * viewFlip);
//    originText->moveBy(0, -1 * originText->mapRectToScene(originText->boundingRect()).height());

    ui->graphicsView_view->setTransform(hpglToPx * viewFlip);

    // Set scene to view
    ui->graphicsView_view->show();
}

void MainWindow::sceneClearHpgl()
{
    for (int i = 0; i < hpgl_items.count(); i++)
    {
        plotScene.removeItem(hpgl_items[i]);
    }
    hpgl_items.clear();
    sceneSetSceneRect();
}

void MainWindow::sceneSetSceneRect()
{
    plotScene.setSceneRect(plotScene.itemsBoundingRect());
}

void MainWindow::do_loadFile(QString filePath)
{
    QSettings settings;

    settings.setValue("mainwindow/filePath", filePath);

    sceneClearHpgl();

    emit please_plotter_loadFile(filePath);
}

void MainWindow::addPolygon(QPolygonF poly)
{
    // Variables
    QSettings settings;
    int rgbColor[3];
    int penSize;
    QColor penColor;
    QPen downPen;
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    int avgDpi = (xDpi + yDpi) / 2.0;

    // Set downPen
    penSize = settings.value("pen/down/size", SETDEF_PEN_DOWN_SIZE).toInt();
    rgbColor[0] = settings.value("pen/down/red", SETDEF_PEN_DOWN_RED).toInt();
    rgbColor[1] = settings.value("pen/down/green", SETDEF_PEN_DOWN_GREEN).toInt();
    rgbColor[2] = settings.value("pen/down/blue", SETDEF_PEN_DOWN_BLUE).toInt();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize * (1016.0 / avgDpi));

    hpgl_items.push_back(plotScene.addPolygon(poly, downPen));
}





























