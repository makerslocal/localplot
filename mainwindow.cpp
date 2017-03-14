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
    ui->listView->setModel(&hpglModel);
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
    connect(ancilla, SIGNAL(hpglParsingDone()), this, SLOT(handle_listViewClick()));
    connect(ancilla, SIGNAL(statusUpdate(QString)), this, SLOT(handle_newConsoleText(QString)));
    connect(ancilla, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    connect(ancilla, SIGNAL(newPolygon(QPersistentModelIndex, QPolygonF)),
            this, SLOT(addPolygon(QPersistentModelIndex, QPolygonF)));
    connect(ancilla, SIGNAL(plottingCancelled()), this, SLOT(handle_plotCancelled()));
    connect(ancilla, SIGNAL(plottingDone()), this, SLOT(handle_plotFinished()));
    connect(ancilla, SIGNAL(plottingStarted()), this, SLOT(handle_plotStarted()));
    connect(this, SIGNAL(please_plotter_doPlot(hpglListModel *)),
            ancilla, SLOT(do_beginPlot(hpglListModel *)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), ancilla, SLOT(do_cancelPlot()));
    connect(this, SIGNAL(please_plotter_loadFile(const QPersistentModelIndex, const hpglListModel *)), ancilla, SLOT(do_loadFile(const QPersistentModelIndex, const hpglListModel *)));

    // View/scene
    connect(&plotScene, SIGNAL(changed(QList<QRectF>)), this, SLOT(sceneConstrainItems()));
    connect(ui->graphicsView_view, SIGNAL(mouseReleased()), this, SLOT(sceneSetSceneRect()));

//    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalDotsPerInchChanged(qreal)),
//            this, SLOT(sceneSetup())); // Update view if the pixel DPI changes

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

    for (int i = (hpglModel.rowCount() - 1); i >= 0; --i)
    {
        hpglModel.removeRow(i);
    }
    plotScene.clear();
    plotScene.deleteLater();

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

void MainWindow::handle_newConsoleText(QString text, QColor textColor)
{
    QColor originalColor = ui->textBrowser_console->textColor();
    ui->textBrowser_console->append(timeStamp());
    ui->textBrowser_console->setTextColor(textColor);
    ui->textBrowser_console->append("- " + text);
    ui->textBrowser_console->setTextColor(originalColor);
}

void MainWindow::handle_newConsoleText(QString text)
{
    ui->textBrowser_console->append(timeStamp() + "\n- " + text);
}

void MainWindow::handle_ancillaThreadStart()
{
    handle_newConsoleText("Ancillary thread started \\o/", Qt::darkGreen);
}

void MainWindow::handle_ancillaThreadQuit()
{
    handle_newConsoleText("Ancillary thread stopped.");
}

void MainWindow::do_plot()
{
    if (ui->pushButton_doPlot->text() == "Plot!")
    {
        emit please_plotter_doPlot(&hpglModel);
    }
    else
    {
        emit please_plotter_cancelPlot();
    }

}

void MainWindow::do_cancelPlot()
{
    emit please_plotter_cancelPlot();
}

void MainWindow::handle_plotStarted()
{
    ui->pushButton_doPlot->setText("Cancel");
    ui->pushButton_fileRemove->setEnabled(false);
    ui->pushButton_fileSelect->setEnabled(false);
    ui->graphicsView_view->setEnabled(false);
    ui->actionLoad_File->setEnabled(false);
    ui->progressBar_plotting->setValue(0);
    handle_newConsoleText("Plotting started.");
}

void MainWindow::handle_plotCancelled()
{
    ui->pushButton_doPlot->setText("Plot!");
    ui->pushButton_fileRemove->setEnabled(true);
    ui->pushButton_fileSelect->setEnabled(true);
    ui->graphicsView_view->setEnabled(true);
    ui->actionLoad_File->setEnabled(true);
    handle_newConsoleText("Plotting cancelled.");
}

void MainWindow::handle_plotFinished()
{
    ui->pushButton_doPlot->setText("Plot!");
    ui->pushButton_fileRemove->setEnabled(true);
    ui->pushButton_fileSelect->setEnabled(true);
    ui->graphicsView_view->setEnabled(true);
    ui->actionLoad_File->setEnabled(true);
    handle_newConsoleText("Plotting Done.");
}

void MainWindow::setItemSelected(QGraphicsItemGroup * group, bool selected)
{
    QModelIndex index;
    QVector<QGraphicsPolygonItem *> items;
    QPen _pen;

    index = group->data(QMODELINDEX_KEY).value<QModelIndex>();
    items = hpglModel.data(index, hpglUserRoles::role_hpgl_items).value<QVector<QGraphicsPolygonItem*>>();
    get_pen(&_pen, "down");

    group->setSelected(selected);
    group->setZValue(selected ? 1 : -1);

    _pen.setColor(_pen.color().lighter(120));
}

void MainWindow::handle_plotSceneSelectionChanged()
{
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QVector<QGraphicsPolygonItem *> * items;
    QPen _selectedPen;

    ui->listView->selectionModel()->clearSelection();

    for (int i = 0; i < hpglModel.rowCount(); ++i)
    {
        index = hpglModel.index(i);
        itemGroup = hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                .value<QGraphicsItemGroup*>();
        items = hpglModel.data(index, hpglUserRoles::role_hpgl_items)
                .value<QVector<QGraphicsPolygonItem*>*>();

        get_pen(&_selectedPen, "down");

        if (itemGroup->isSelected())
        {
            ui->listView->selectionModel()->select(index, QItemSelectionModel::Select);
            itemGroup->setZValue(1);
            _selectedPen.setColor(_selectedPen.color().lighter(120));
        }
        else
        {
            ui->listView->selectionModel()->select(index, QItemSelectionModel::Deselect);
            itemGroup->setZValue(-1);
        }

        for (int i3 = 0; i3 < items->length(); ++i3)
        {
            (*items)[i3]->setPen(_selectedPen);
        }
    }
}

void MainWindow::handle_listViewClick()
{
    QModelIndexList list;
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QVector<QGraphicsPolygonItem *> * items;
    QPen _selectedPen;

    list = ui->listView->selectionModel()->selectedIndexes();
    QItemSelectionModel * selectionmodel = ui->listView->selectionModel();

    for (int i = 0; i < hpglModel.rowCount(); ++i)
    {
        index = hpglModel.index(i);
        itemGroup = hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                .value<QGraphicsItemGroup*>();
        items = hpglModel.data(index, hpglUserRoles::role_hpgl_items)
                .value<QVector<QGraphicsPolygonItem*>*>();

        get_pen(&_selectedPen, "down");

        if (selectionmodel->isSelected(index))
        {
            itemGroup->setSelected(true);
            itemGroup->setZValue(1);
            _selectedPen.setColor(_selectedPen.color().lighter(120));
        }
        else
        {
            itemGroup->setSelected(false);
            itemGroup->setZValue(-1);
        }

        for (int i3 = 0; i3 < items->length(); ++i3)
        {
            (*items)[i3]->setPen(_selectedPen);
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

    if (_file.path == "")
    {
        handle_newConsoleText("File open cancelled.", Qt::darkRed);
        return;
    }

    settings.setValue("mainwindow/filePath", _file.path);

    _file.filename = _file.path.split("/").last();

    if (hpglModel.rowCount() == 0)
    {
        _file.uid = 0;
    }
    else
    {
        QModelIndex index = hpglModel.index(hpglModel.rowCount()-1);
        _file.uid = hpglModel.data(index, hpglUserRoles::role_uid).toInt() + 1;
    }

    do_loadFile(_file);
}

void MainWindow::handle_deleteFileBtn()
{
    QModelIndexList list;
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QVector<QGraphicsPolygonItem *> * items;

    list = ui->listView->selectionModel()->selectedIndexes();

    for (int i = list.length()-1; i >= 0; --i)
    {
        index = list.at(i);
        itemGroup = hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                .value<QGraphicsItemGroup*>();
        items = hpglModel.data(index, hpglUserRoles::role_hpgl_items)
                .value<QVector<QGraphicsPolygonItem*>*>();
        for (int i2 = items->length()-1; i2 >= 0; --i2)
        {
            plotScene.removeItem(items->at(i2));
        }
        plotScene.destroyItemGroup(itemGroup);
//        plotScene.removeItem(group);
        hpglModel.removeRow(list[i].row());
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
        plotScene.sceneRect(),
        Qt::KeepAspectRatio);
    handle_newConsoleText("Scene scale set to view all", Qt::darkGreen);
    return;
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

    handle_newConsoleText("Scene scale set to 1:1", Qt::darkGreen);
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

QPersistentModelIndex MainWindow::createHpglFile(file_uid _file)
{
//    qDebug() << "Creating hpgl_file for: " << _file.path << ", " << _file.uid;

    // Add to listView
    int numRows = hpglModel.rowCount();
    if (!hpglModel.insertRows(numRows, 1))
    {
        qDebug() << "Insert row failed.";
        return(QModelIndex());
    }

    QPersistentModelIndex index = QPersistentModelIndex(hpglModel.index(numRows));
    if (!hpglModel.setData(index, QVariant::fromValue(_file), hpglUserRoles::role_name))
    {
        qDebug() << "Setdata failed.";
        return(QModelIndex());
    }

    QGraphicsItemGroup * newGroup = hpglModel.data(index, hpglUserRoles::role_hpgl_items_group).value<QGraphicsItemGroup*>();
    plotScene.addItem(newGroup);
    hpglModel.setGroupFlag(index, QGraphicsItem::ItemIsMovable, true);
    hpglModel.setGroupFlag(index, QGraphicsItem::ItemIsSelectable, true);
    ui->listView->setCurrentIndex(index);

    return(index);
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

    for (int i = 0; i < hpglModel.rowCount(); ++i)
    {
        QModelIndex index = hpglModel.index(i);
        modCount = 0;

        QPointF pos = hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                .value<QGraphicsItemGroup*>()->pos();

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
        else if ((pos.y() + hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                  .value<QGraphicsItemGroup*>()->boundingRect().height()) >
                 _point.y())
        {
            ++modCount;
            pos.setY(_point.y() - hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                     .value<QGraphicsItemGroup*>()->boundingRect().height());
        }


        if (modCount)
        {
            hpglModel.data(index, hpglUserRoles::role_hpgl_items_group)
                            .value<QGraphicsItemGroup*>()->setPos(pos);
        }
    }
}

void MainWindow::do_loadFile(file_uid _file)
{
    QSettings settings;

    settings.setValue("mainwindow/filePath", _file.path);

    QPersistentModelIndex index = createHpglFile(_file);

    emit please_plotter_loadFile(index, &hpglModel);
}

void MainWindow::addPolygon(QPersistentModelIndex index, QPolygonF poly)
{
    // Variables
    QPen pen;

    // Set downPen
    get_pen(&pen, "down");

    hpglModel.setData(index, QVariant::fromValue(plotScene.addPolygon(poly, pen)), hpglUserRoles::role_hpgl_items);
}


































