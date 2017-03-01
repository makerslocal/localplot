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
    ancilla->moveToThread(&ancillaryThreadInstance);

    hpgl_items_group = NULL;
    plotScene.setObjectName("plotScene");

    // Setup listView and listModel
    listModel = new QStringListModel(this);
    ui->listView->setModel(listModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Connect UI actions
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(do_openDialogAbout()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(do_openDialogSettings()));

    // Connect thread
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    connect(ancilla, SIGNAL(plottingProgress(int)), this, SLOT(handle_plottingPercent(int)));
    connect(&ancillaryThreadInstance, SIGNAL(started()), this, SLOT(handle_ancillaThreadStart()));
    connect(&ancillaryThreadInstance, SIGNAL(finished()), this, SLOT(handle_ancillaThreadQuit()));
    connect(&ancillaryThreadInstance, SIGNAL(finished()), ancilla, SLOT(deleteLater()));
    connect(ancilla, SIGNAL(hpglParsingDone()), this, SLOT(sceneSetSceneRect()));
    connect(ancilla, SIGNAL(hpglParsingDone()), this, SLOT(handle_groupingItems()));
    connect(ancilla, SIGNAL(statusUpdate(QString)), this, SLOT(handle_ancillaThreadStatus(QString)));
    connect(ancilla, SIGNAL(newPolygon(QPolygonF)), this, SLOT(addPolygon(QPolygonF)));
    connect(this, SIGNAL(please_plotter_doPlot(const QVector<QGraphicsPolygonItem *>)),
            ancilla, SLOT(do_beginPlot(const QVector<QGraphicsPolygonItem *>)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), ancilla, SLOT(do_cancelPlot()));
    connect(this, SIGNAL(please_plotter_loadFile(QString)), ancilla, SLOT(do_loadFile(QString)));

    // View/scene
    connect(&plotScene, SIGNAL(changed(QList<QRectF>)), this, SLOT(sceneConstrainItems()));
    connect(ui->graphicsView_view, SIGNAL(mouseReleased()), this, SLOT(sceneSetSceneRect()));

    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalDotsPerInchChanged(qreal)),
            this, SLOT(sceneSetup())); // Update view if the pixel DPI changes

    ui->graphicsView_view->setScene(&plotScene);

    sceneSetup();

    ui->pushButton_doPlot->setEnabled(true);

    // Kickstart worker thread
    ancillaryThreadInstance.start();
}

MainWindow::~MainWindow()
{
    ancillaryThreadInstance.quit();
    ancillaryThreadInstance.wait();

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

void MainWindow::get_pen(QPen * _pen, QString _name)
{
    // Variables
    QSettings settings;
    int rgbColor[3];
    int penSize;
    QColor penColor;

    // Set downPen
    settings.beginGroup("pen/"+_name);
    {
        penSize = settings.value("size", SETDEF_PEN_DOWN_SIZE).toInt();
        rgbColor[0] = settings.value("red", SETDEF_PEN_DOWN_RED).toInt();
        rgbColor[1] = settings.value("green", SETDEF_PEN_DOWN_GREEN).toInt();
        rgbColor[2] = settings.value("blue", SETDEF_PEN_DOWN_BLUE).toInt();
        penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
        _pen->setColor(penColor);
        _pen->setWidth(penSize);
    }
    settings.endGroup();
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
    emit please_plotter_doPlot(hpgl_items);
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

void MainWindow::handle_groupingItems()
{
//    qDebug() << "Grouping items.";
//    hpgl_items_group = new QGraphicsItemGroup;
//    plotScene.addItem(hpgl_items_group);
//    for (int i = 0; i < hpgl_items.count(); ++i)
//    {
//        hpgl_items_group->addToGroup(static_cast<QGraphicsItem*>(hpgl_items[i]));
//    }
    hpgl_items_group->setFlag(QGraphicsItem::ItemIsMovable, true);
//    plotScene.installEventFilter(this);
//    plotScene.removeItem(static_cast<QGraphicsItem*>(hpgl_items[1]));
}

void MainWindow::handle_selectFileBtn()
{
    QSettings settings;
    QString filePath;
    QString startDir = settings.value("mainwindow/filePath", "").toString();

    filePath = QFileDialog::getOpenFileName(this,
        tr("Open File"), startDir, tr("HPGL Files (*.hpgl *.HPGL)"));

    settings.setValue("mainwindow/filePath", filePath);

    QList<QGraphicsItem *> allThings = plotScene.items();
    for (int i = 0; i < allThings.length(); i++)
    {
        allThings[i]->setSelected(false);
    }

    listModel->insertRows(0, 1);
    int row = listModel->rowCount();
    QModelIndex index = listModel->index(row-1);
    listModel->setData(index, filePath, Qt::DisplayRole);

    do_loadFile(filePath);
}

void MainWindow::handle_plottingPercent(int percent)
{
    ui->progressBar_plotting->setValue(percent);
}

/*******************************************************************************
 * Etcetera methods
 ******************************************************************************/

void MainWindow::sceneSetup()
{
    QPen pen;

    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();

    // Transforms
    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    // Auto-position scene to bottom left of view.
    ui->graphicsView_view->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // Set up new graphics view.
    plotScene.clear();

    // Create item group
    hpgl_items_group = new QGraphicsItemGroup;
    plotScene.addItem(hpgl_items_group);
    hpgl_items_group->setFlag(QGraphicsItem::ItemIsMovable, true);

    // Draw origin
    pen.setColor(QColor(150, 150, 150));
    pen.setWidth(2);
    plotScene.addLine(0, 0, xDpi, 0, pen)->setTransform(itemToScene);
    plotScene.addLine(0, 0, 0, yDpi, pen)->setTransform(itemToScene);

    // Draw origin text
    QGraphicsTextItem * label = plotScene.addText("Front of Plotter");
    label->setTransform(itemToScene * viewFlip);
    label->setRotation(-90);
    label->moveBy(-1 * label->mapRectToScene(label->boundingRect()).width(), 0);

    QGraphicsTextItem * originText = plotScene.addText("(0,0)");
    originText->setTransform(itemToScene * viewFlip);

    ui->graphicsView_view->setTransform(hpglToPx * viewFlip);

    // Set scene to view
    ui->graphicsView_view->show();
}

void MainWindow::sceneClearHpgl()
{
    qDebug() << "Clearing hpgl from scene.";
    for (int i = 0; i < hpgl_items.count(); i++)
    {
        plotScene.removeItem(hpgl_items[i]);
    }
    hpgl_items.clear();
    sceneSetSceneRect();
}

void MainWindow::sceneSetSceneRect()
{
    qreal marginX, marginY;
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();

    // Transforms
    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    // Quarter inch margins
    marginX = (xDpi / 4.0) * (1016.0 / xDpi);
    marginY = (yDpi / 4.0) * (1016.0 / yDpi);

    qDebug() << "Setting scene rect.";

    plotScene.setSceneRect(plotScene.itemsBoundingRect().marginsAdded(QMarginsF(marginX, marginY, marginX, marginY)));
    QRectF _rect = plotScene.sceneRect();
    QRectF _ViewRect = ui->graphicsView_view->rect();

    _ViewRect = itemToScene.mapRect(_ViewRect);

    qDebug() << "Graphics view: " << ui->graphicsView_view->rect().width() << "," << ui->graphicsView_view->rect().height();
    qDebug() << "Graphics view: " << ui->graphicsView_view->size().width() << "," << ui->graphicsView_view->size().height();
    qDebug() << "Scene size: " << plotScene.sceneRect().width() << "," << plotScene.sceneRect().height();

//    plotScene.addRect(ui->graphicsView_view->rect())->setTransform(itemToScene);

    if (_rect.width() < _ViewRect.width())
    {
        _rect.setWidth(ui->graphicsView_view->rect().width());
        qDebug() << "Increasing scene width." << _rect.width();
        plotScene.setSceneRect(_rect);
    }
    if (_rect.height() < ui->graphicsView_view->rect().height())
    {
        _rect.setHeight(ui->graphicsView_view->rect().height());
        qDebug() << "Increasing scene height: " << _rect.height();
        plotScene.setSceneRect(_rect);
    }
}

void MainWindow::sceneConstrainItems()
{
    if (hpgl_items_group == NULL)
    {
        return;
    }
    QPointF pos = hpgl_items_group->pos();
    if (pos.x() < 0)
    {
        hpgl_items_group->setPos(0, pos.y());
        pos.setX(0);
    }
    if (pos.y() < 0)
    {
        hpgl_items_group->setPos(pos.x(), 0);
    }
}

void MainWindow::do_loadFile(QString filePath)
{
    QSettings settings;

    settings.setValue("mainwindow/filePath", filePath);

    emit please_plotter_loadFile(filePath);
}

void MainWindow::addPolygon(QPolygonF poly)
{
    // Variables
    QPen pen;
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    int avgDpi = (xDpi + yDpi) / 2.0;

    // Set downPen
    get_pen(&pen, "down");
    pen.setWidth(pen.widthF() * (1016.0 / avgDpi));

    hpgl_items.push_back(plotScene.addPolygon(poly, pen));
    hpgl_items_group->addToGroup(static_cast<QGraphicsItem*>(hpgl_items.last()));
//    hpgl_items.last()->setFlag(QGraphicsItem::ItemIsSelectable, true);
//    hpgl_items.last()->setSelected(true);
}

//bool MainWindow::eventFilter(QObject *obj, QEvent *event)
//{
//    if (obj == &plotScene) {
//        if (event->type() == QEvent::GraphicsSceneMousePress){//QEvent::GraphicsSceneMouseRelease) {
//            qDebug() << "filter obj: " << obj->objectName() << event->type();
//            QMouseEvent * mouseEvent = static_cast<QMouseEvent*>(event);
//            qDebug() << mouseEvent->buttons() << "button: " << mouseEvent->button() << (mouseEvent->button() == Qt::LeftButton);
//            if (!(mouseEvent->buttons() & Qt::LeftButton))
//            {
//                sceneSetSceneRect();
//            }
//        }
//    }
//    // pass the event on to the parent class
//    return QMainWindow::eventFilter(obj, event);
//}


































