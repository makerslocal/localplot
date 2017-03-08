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

    // Setup listView and listModel
    listModel = new QStringListModel(this);
    ui->listView->setModel(listModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Connect UI actions
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(do_openDialogAbout()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(do_openDialogSettings()));
    connect(ui->action1_1, SIGNAL(triggered(bool)), this, SLOT(sceneScale11()));
    connect(ui->actionAll, SIGNAL(triggered(bool)), this, SLOT(sceneScaleWidth()));
    connect(ui->pushButton_fileRemove, SIGNAL(clicked(bool)), this, SLOT(handle_deleteFileBtn()));
    connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(handle_listViewClick()));
    connect(&plotScene, SIGNAL(selectionChanged()), this, SLOT(handle_plotSceneSelectionChanged()));

    // Connect thread
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    connect(ancilla, SIGNAL(plottingProgress(int)), this, SLOT(handle_plottingPercent(int)));
    connect(&ancillaryThreadInstance, SIGNAL(started()), this, SLOT(handle_ancillaThreadStart()));
    connect(&ancillaryThreadInstance, SIGNAL(finished()), this, SLOT(handle_ancillaThreadQuit()));
    connect(&ancillaryThreadInstance, SIGNAL(finished()), ancilla, SLOT(deleteLater()));
    connect(ancilla, SIGNAL(hpglParsingDone()), this, SLOT(sceneSetSceneRect()));
    connect(ancilla, SIGNAL(statusUpdate(QString)), this, SLOT(handle_ancillaThreadStatus(QString)));
    connect(ancilla, SIGNAL(newPolygon(file_uid, QPolygonF)), this, SLOT(addPolygon(file_uid, QPolygonF)));
    connect(this, SIGNAL(please_plotter_doPlot(QVector<hpgl_file *> *)),
            ancilla, SLOT(do_beginPlot(QVector<hpgl_file *> *)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), ancilla, SLOT(do_cancelPlot()));
    connect(this, SIGNAL(please_plotter_loadFile(file_uid)), ancilla, SLOT(do_loadFile(file_uid)));

    // View/scene
    connect(&plotScene, SIGNAL(changed(QList<QRectF>)), this, SLOT(sceneConstrainItems()));
    connect(ui->graphicsView_view, SIGNAL(mouseReleased()), this, SLOT(sceneSetSceneRect()));

    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalDotsPerInchChanged(qreal)),
            this, SLOT(sceneSetup())); // Update view if the pixel DPI changes

    ui->graphicsView_view->setScene(&plotScene);

    sceneSetup();

    ui->pushButton_doPlot->setEnabled(true);
    ui->pushButton_fileRemove->setEnabled(true);

    // Restore saved window geometry
    if (settings.contains("mainwindow/geometry"))
    {
        restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
    }
    if (settings.contains("mainwindow/windowState"))
    {
        restoreState(settings.value("mainwindow/windowState").toByteArray());
    }

    // Kickstart worker thread
    ancillaryThreadInstance.start();
}

MainWindow::~MainWindow()
{
    ancillaryThreadInstance.quit();
    ancillaryThreadInstance.wait();

    for (int i = (hpglList.length() - 1); i >= 0; --i)
    {
        deleteHpglFile(hpglList[i]);
    }

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/windowState", saveState());
    QMainWindow::closeEvent(event);
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
    widthLine->setLine(get_widthLine());
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

    _pen->setCosmetic(true);
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
    emit please_plotter_doPlot(&hpglList);
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

void MainWindow::handle_plotSceneSelectionChanged()
{
    QList<QGraphicsItem*> list;
    bool selectedFlag = false;

    list = plotScene.selectedItems();

    ui->listView->selectionModel()->clearSelection();

    for (int i = 0; i < hpglList.length(); ++i)
    {
        selectedFlag = false;
        for (int i2 = 0 ; i2 < list.length(); ++i2)
        {
            if (list.at(i2) == hpglList.at(i)->hpgl_items_group)
            {
                QModelIndex index;
                index = listModel->index(i);
                ui->listView->selectionModel()->select(index, QItemSelectionModel::Select);
                QPen _selectedPen;
                get_pen(&_selectedPen, "down");
                _selectedPen.setColor(_selectedPen.color().lighter(120));
                for (int i3 = 0; i3 < hpglList[i]->hpgl_items.length(); ++i3)
                {
                    hpglList[i]->hpgl_items[i3]->setPen(_selectedPen);
                }
                selectedFlag = true;
                break;
            }
        }
        if (selectedFlag == false)
        {
            hpglList[i]->hpgl_items_group->setSelected(false);
            QPen _selectedPen;
            get_pen(&_selectedPen, "down");
            for (int i3 = 0; i3 < hpglList[i]->hpgl_items.length(); ++i3)
            {
                hpglList[i]->hpgl_items[i3]->setPen(_selectedPen);
            }
        }
    }
}

void MainWindow::handle_listViewClick()
{
    QModelIndexList list;
    bool selectedFlag = false;

    list = ui->listView->selectionModel()->selectedIndexes();

    for (int i = 0; i < hpglList.length(); ++i)
    {
        selectedFlag = false;
        for (int i2 = 0 ; i2 < list.length(); ++i2)
        {
            if (list.at(i2).data(Qt::DisplayRole).toString() ==
                    (hpglList.at(i)->name.path+", "+QString::number(hpglList.at(i)->name.uid)))
            {
                hpglList[i]->hpgl_items_group->setSelected(true);
                QPen _selectedPen;
                get_pen(&_selectedPen, "down");
                _selectedPen.setColor(_selectedPen.color().lighter(120));
                for (int i3 = 0; i3 < hpglList[i]->hpgl_items.length(); ++i3)
                {
                    hpglList[i]->hpgl_items[i3]->setPen(_selectedPen);
                }
                selectedFlag = true;
                break;
            }
        }
        if (selectedFlag == false)
        {
            hpglList[i]->hpgl_items_group->setSelected(false);
            QPen _selectedPen;
            get_pen(&_selectedPen, "down");
            for (int i3 = 0; i3 < hpglList[i]->hpgl_items.length(); ++i3)
            {
                hpglList[i]->hpgl_items[i3]->setPen(_selectedPen);
            }
        }
    }
}

void MainWindow::handle_selectFileBtn()
{
    QSettings settings;
    file_uid _file;
    QString startDir = settings.value("mainwindow/filePath", "").toString();

    _file.path = QFileDialog::getOpenFileName(this,
        tr("Open File"), startDir, tr("HPGL Files (*.hpgl *.HPGL)"));

    settings.setValue("mainwindow/filePath", _file.path);

    if (hpglList.isEmpty())
    {
        _file.uid = 0;
    }
    else
    {
        _file.uid = hpglList.last()->name.uid + 1;
    }

    do_loadFile(_file);
}

void MainWindow::handle_deleteFileBtn()
{
    QModelIndexList list;

    list = ui->listView->selectionModel()->selectedIndexes();

    for (int i = (hpglList.length() - 1); i >= 0 ; --i)
    {
        if (hpglList.at(i)->hpgl_items_group->isSelected())
        {
            deleteHpglFile(hpglList.at(i));
        }
    }
}

void MainWindow::handle_plottingPercent(int percent)
{
    ui->progressBar_plotting->setValue(percent);
}

/*******************************************************************************
 * Etcetera methods
 ******************************************************************************/

QLineF MainWindow::get_widthLine()
{
    // physicalDpi is the number of pixels in an inch
    int yDpi = ui->graphicsView_view->physicalDpiY();
    QSettings settings;
    double length;
    if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::INCH)
    {
        qDebug() << "Device width in inches.";
        length = settings.value("device/width", SETDEF_DEVICE_WIDTH).toInt();
    }
    else if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
    {
        qDebug() << "Device width in cm.";
        length = settings.value("device/width", SETDEF_DEVICE_WIDTH).toInt() * 2.54;
    }
    else
    {
        qDebug() << "Default switch statement reached for device width! D:";
        length = SETDEF_DEVICE_WIDTH;
    }
    return (QLineF(0, 0, 0, (1016.0*length)));
}

void MainWindow::sceneScaleWidth()
{
    ui->graphicsView_view->fitInView(
        plotScene.itemsBoundingRect(),
        Qt::KeepAspectRatio);
    return;
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();

    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    QTransform scenetowidth;

    scenetowidth.scale(1, 1);
    ui->graphicsView_view->setTransform(viewFlip);
}

void MainWindow::sceneScale11()
{
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    // Transforms
    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    ui->graphicsView_view->setTransform(hpglToPx * viewFlip);
}

void MainWindow::sceneSetup()
{
    QPen pen;
    QSettings settings;

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

    // Draw origin
    pen.setCosmetic(true);
    pen.setColor(QColor(150, 150, 150));
    pen.setWidth(2);
    plotScene.addLine(0, 0, xDpi, 0, pen)->setTransform(itemToScene);

    // Width line
    widthLine = plotScene.addLine(get_widthLine(), pen);

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

    sceneSetSceneRect();
}

void MainWindow::deleteHpglFile(hpgl_file * _hpgl)
{
    file_uid _file = _hpgl->name;
    QModelIndex modelIndex;

    // Remove from listView
    for (int i = 0; i < listModel->rowCount(); ++i)
    {
        modelIndex = listModel->index(i);
//        qDebug() << "listview deletion: " << listModel->data(modelIndex, Qt::DisplayRole).toString();
//        qDebug() << " vs: " << (_file.path+", "+QString::number(_file.uid));
        if (listModel->data(modelIndex, Qt::DisplayRole).toString() == (_file.path+", "+QString::number(_file.uid)))
        {
            qDebug() << "Removing listview row: " << i;
            listModel->removeRow(i);
            break;
        }
    }

    for (int i = 0; i < hpglList.length(); ++i)
    {
        if (hpglList[i]->name == _file)
        {
            // Remove from graphicsView
            for (int i2 = 0; i2 < hpglList[i]->hpgl_items.length(); ++i2)
            {
                hpglList[i]->hpgl_items_group->removeFromGroup(static_cast<QGraphicsItem*>(hpglList[i]->hpgl_items[i2]));
                plotScene.removeItem(hpglList[i]->hpgl_items[i2]);
            }
            delete hpglList[i]->hpgl_items_group;
            hpglList[i]->hpgl_items.clear();
            hpglList.remove(i);
            break;
        }
    }
}

hpgl_file * MainWindow::createHpglFile(file_uid _file)
{
    hpgl_file * newFile;

    qDebug() << "Creating hpgl_file for: " << _file.path << ", " << _file.uid;

    // Set up hpgl_file
    newFile = new hpgl_file;
    newFile->hpgl_items_group = new QGraphicsItemGroup;
    newFile->hpgl_items_group->setFlag(QGraphicsItem::ItemIsMovable, true);
    newFile->hpgl_items_group->setFlag(QGraphicsItem::ItemIsSelectable, true);
    newFile->name = _file;

    newFile->hpgl_items.clear();
    plotScene.addItem(newFile->hpgl_items_group);

    // Add to listView
    int numRows = listModel->rowCount();
    listModel->insertRows(numRows, 1);
    QModelIndex index = listModel->index(numRows);
    listModel->setData(index, newFile->name.path+", "+QString::number(newFile->name.uid), Qt::DisplayRole);
    ui->listView->setCurrentIndex(index);

    hpglList.push_back(newFile);
    return(hpglList.last());
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

    QRectF srect = plotScene.itemsBoundingRect();

    if (srect.x() < 0)
    {
        srect.setX(0);
    }
    if (srect.y() < 0)
    {
        srect.setY(0);
    }

    plotScene.setSceneRect(srect.marginsAdded(QMarginsF(marginX, marginY, marginX, marginY)));

    return;

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
    int modCount;
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    QTransform sceneToItem;
    sceneToItem.scale(xDpi/1016.0, yDpi/1016.0);

    for (int i = 0; i < hpglList.length(); ++i)
    {
        modCount = 0;
        if (hpglList.at(i)->hpgl_items_group == NULL)
        {
            qDebug() << "Serious issue somewhere with hpglList. hpgl_items_group is missing!";
            return;
        }
        QPointF pos = hpglList.at(i)->hpgl_items_group->pos();
        if (pos.x() < 0)
        {
            ++modCount;
            pos.setX(0);
        }

        QPointF _point = widthLine->mapToScene(widthLine->line().p2());
        if (pos.y() < 0)
        {
            ++modCount;
            pos.setY(0);
        }
        else if ((pos.y() + hpglList.at(i)->hpgl_items_group->boundingRect().height()) >
                 _point.y())
        {
            ++modCount;
            pos.setY(_point.y() - hpglList.at(i)->hpgl_items_group->boundingRect().height());
        }


        if (modCount)
        {
            hpglList[i]->hpgl_items_group->setPos(pos);
        }
    }
}

void MainWindow::do_loadFile(file_uid _file)
{
    QSettings settings;

    settings.setValue("mainwindow/filePath", _file.path);

    createHpglFile(_file);

    emit please_plotter_loadFile(_file);
}

void MainWindow::addPolygon(file_uid _file, QPolygonF poly)
{
    // Variables
    QPen pen;
    // physicalDpi is the number of pixels in an inch
//    int xDpi = ui->graphicsView_view->physicalDpiX();
//    int yDpi = ui->graphicsView_view->physicalDpiY();
//    int avgDpi = (xDpi + yDpi) / 2.0;

    // Set downPen
    get_pen(&pen, "down");
//    pen.setWidth(pen.widthF() * (1016.0 / avgDpi));

    for (int i = 0; i < hpglList.length(); ++i)
    {
        if (_file == hpglList.at(i)->name)
        {
            hpglList[i]->hpgl_items.push_back(plotScene.addPolygon(poly, pen));
            hpglList[i]->hpgl_items_group->addToGroup(static_cast<QGraphicsItem*>(hpglList[i]->hpgl_items.last()));
            break;
        }
    }
}


































