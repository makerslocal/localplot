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

    // Setup listView and listModel
    ui->listView->setModel(&hpglModel);

    // Connect UI buttons
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(do_plot()));
    connect(ui->pushButton_fileRemove, SIGNAL(clicked(bool)), this, SLOT(handle_deleteFileBtn()));
    connect(ui->toolButton_rotateLeft, SIGNAL(clicked(bool)), this, SLOT(handle_rotateLeftBtn()));
    connect(ui->toolButton_rotateRight, SIGNAL(clicked(bool)), this, SLOT(handle_rotateRightBtn()));
    connect(ui->toolButton_flipX, SIGNAL(clicked(bool)), this, SLOT(handle_flipXbtn()));
    connect(ui->toolButton_flipY, SIGNAL(clicked(bool)), this, SLOT(handle_flipYbtn()));

    // Connect UI actions
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(do_openDialogAbout()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(do_openDialogSettings()));
    connect(ui->action1_1, SIGNAL(triggered(bool)), this, SLOT(sceneScale11()));
    connect(ui->actionAll, SIGNAL(triggered(bool)), this, SLOT(sceneScaleWidth()));
    connect(ui->actionItems, SIGNAL(triggered(bool)), this, SLOT(sceneScaleContain()));
    connect(ui->actionContain_Selected_Items, SIGNAL(triggered(bool)), this, SLOT(sceneScaleContainSelected()));

    // View/scene
    connect(&plotScene, SIGNAL(changed(QList<QRectF>)), this, SLOT(sceneConstrainItems()));
    connect(ui->graphicsView_view, SIGNAL(mouseReleased()), this, SLOT(sceneSetSceneRect()));
    connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(handle_listViewClick()));
    connect(&plotScene, SIGNAL(selectionChanged()), this, SLOT(handle_plotSceneSelectionChanged()));

//    connect(QGuiApplication::primaryScreen(), SIGNAL(physicalDotsPerInchChanged(qreal)),
//            this, SLOT(sceneSetup())); // Update view if the pixel DPI changes

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

    // Setup statusbar
    progressBar_plotting = new QProgressBar;
    label_eta = new QLabel;
    label_status = new QLabel;
    label_zoom = new QLabel;
    label_eta->setText("ETA: NA");
    label_status->setText("Status label created.");
    label_zoom->setText("Zoom: NA");
    statusBar()->addPermanentWidget(label_status);
    statusBar()->addPermanentWidget(statusBarDivider());
    statusBar()->addPermanentWidget(progressBar_plotting);
    statusBar()->addPermanentWidget(statusBarDivider());
    statusBar()->addPermanentWidget(label_eta);
    statusBar()->addPermanentWidget(statusBarDivider());
    statusBar()->addPermanentWidget(label_zoom);

    ui->graphicsView_view->setScene(&plotScene);

    sceneSetup();
}

MainWindow::~MainWindow()
{
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

QFrame * MainWindow::statusBarDivider()
{
    QFrame * line;
    line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QString timeStamp()
{
    return(QTime::currentTime().toString("[HH:mm ss.zzz]"));
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

void MainWindow::handle_plottingEta(double eta)
{
    label_eta->setText("ETA: " + QString::number(eta));
}

void MainWindow::handle_zoomChanged(QString text)
{
    label_zoom->setText("Zoom: " + text);
}

void MainWindow::handle_newConsoleText(QString text, QColor textColor)
{
    qDebug() << timeStamp() << text;
    label_status->setText(text);
    label_status->setStyleSheet("QLabel { color : "+textColor.name()+"; }");
}

void MainWindow::handle_newConsoleText(QString text)
{
    qDebug() << timeStamp() << text;
    label_status->setText(text);
    label_status->setStyleSheet("QLabel { color : #000; }");
}

void MainWindow::do_plot()
{
    if (ui->pushButton_doPlot->text() == "Plot!")
    {
        // Load file in new thread
        QThread * workerThread = new QThread;
        ExtPlot * worker = new ExtPlot(&hpglModel);
        worker->moveToThread(workerThread);
        connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
        connect(workerThread, SIGNAL(started()), this, SLOT(handle_plotStarted()));
        connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(workerThread, SIGNAL(finished()), this, SLOT(handle_plotFinished()));
        connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(worker, SIGNAL(progress(int)), this, SLOT(handle_plottingPercent(int)));
        connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
        connect(this, SIGNAL(please_plotter_cancelPlot()), worker, SLOT(cancel()));
        workerThread->start();
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
    progressBar_plotting->setValue(0);
}

void MainWindow::handle_plotFinished()
{
    ui->pushButton_doPlot->setText("Plot!");
    ui->pushButton_fileRemove->setEnabled(true);
    ui->pushButton_fileSelect->setEnabled(true);
    ui->graphicsView_view->setEnabled(true);
    ui->actionLoad_File->setEnabled(true);
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
        itemGroup = NULL;
        items = NULL;
        QMutex * mutex;
        hpglModel.dataItemsGroup(index, mutex, itemGroup, items);
        if (!mutex->tryLock())
        {
            qDebug() << "Mutex already locked, giving up.";
            return;
        }

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontainselected().";
            mutex->unlock();
            return;
        }
        if (items == NULL)
        {
            qDebug() << "Error: items is null in scenescalecontainselected().";
            mutex->unlock();
            return;
        }

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
        mutex->unlock();
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
        itemGroup = NULL;
        items = NULL;
        QMutex * mutex;
        hpglModel.dataItemsGroup(index, mutex, itemGroup, items);
        if (!mutex->tryLock())
        {
            qDebug() << "Mutex already locked, giving up.";
            return;
        }

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontainselected().";
            mutex->unlock();
            return;
        }
        if (items == NULL)
        {
            qDebug() << "Error: items is null in scenescalecontainselected().";
            mutex->unlock();
            return;
        }

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
        mutex->unlock();
    }
}

void MainWindow::handle_selectFileBtn()
{
    QSettings settings;
    QString filePath;
    QString startDir = settings.value("mainwindow/filePath", "").toString();

    filePath = QFileDialog::getOpenFileName(this,
        tr("Open File"), startDir, tr("HPGL Files (*.hpgl *.HPGL)"));

    if (filePath.isEmpty())
    {
        handle_newConsoleText("File open cancelled.", Qt::darkRed);
        return;
    }

    settings.setValue("mainwindow/filePath", filePath);

    // Load file in new thread
    QThread * workerThread = new QThread;
    ExtLoadFile * worker = new ExtLoadFile(&hpglModel);
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished(QPersistentModelIndex)), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished(QPersistentModelIndex)), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished(QPersistentModelIndex)), this, SLOT(newFileToScene(QPersistentModelIndex)));
    connect(worker, SIGNAL(finished(QPersistentModelIndex)), this, SLOT(do_procEta()));
    connect(worker, SIGNAL(newPolygon(QPersistentModelIndex,QPolygonF)),
            this, SLOT(addPolygon(QPersistentModelIndex,QPolygonF)));
    connect(worker, SIGNAL(progress(int)), this, SLOT(handle_plottingPercent(int)));
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    workerThread->start();
}

void MainWindow::do_procEta()
{
    // Load file in new thread
    QThread * workerThread = new QThread;
    ExtEta * worker = new ExtEta(&hpglModel);
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished(double)), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished(double)), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished(double)), this, SLOT(handle_plottingEta(double)));
    connect(worker, SIGNAL(progress(int)), this, SLOT(handle_plottingPercent(int)));
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    workerThread->start();
}

void MainWindow::newFileToScene(QPersistentModelIndex _index)
{
    QGraphicsItemGroup * itemGroup = NULL;
    ui->listView->setCurrentIndex(_index);

    QMutex * mutex;
    hpglModel.dataGroup(_index, mutex, itemGroup);
    QMutexLocker rowLocker(mutex);

    if (itemGroup == NULL)
    {
        qDebug() << "Error: itemgroup is null in newFileToScene().";
        return;
    }

    plotScene.addItem(itemGroup);
    rowLocker.unlock();

    hpglModel.setGroupFlag(_index, QGraphicsItem::ItemIsMovable, true);
    hpglModel.setGroupFlag(_index, QGraphicsItem::ItemIsSelectable, true);

    handle_listViewClick();
    sceneSetSceneRect();
//    emit please_plotter_procEta(&hpglModel);
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
        itemGroup = NULL;
        items = NULL;
        QMutex * mutex;
        hpglModel.dataItemsGroup(index, mutex, itemGroup, items);
        if (!mutex->tryLock())
        {
            qDebug() << "Mutex already locked, giving up.";
            return;
        }

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontainselected().";
            mutex->unlock();
            return;
        }
        if (items == NULL)
        {
            qDebug() << "Error: items is null in scenescalecontainselected().";
            mutex->unlock();
            return;
        }

        for (int i2 = items->length()-1; i2 >= 0; --i2)
        {
            plotScene.removeItem(items->at(i2));
        }
        plotScene.destroyItemGroup(itemGroup);
        mutex->unlock();
        hpglModel.removeRow(list[i].row());
    }
}

void MainWindow::handle_rotateLeftBtn()
{
    rotateSelectedItems(90);
}

void MainWindow::handle_rotateRightBtn()
{
    rotateSelectedItems(-90);
}

void MainWindow::handle_flipXbtn()
{
    scaleSelectedItems(-1, 1);
}

void MainWindow::handle_flipYbtn()
{
    scaleSelectedItems(1, -1);
}

void MainWindow::handle_plottingPercent(int percent)
{
    progressBar_plotting->setValue(percent);
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
    handle_zoomChanged("Vinyl width");
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
    handle_zoomChanged("Actual size");
}

void MainWindow::sceneScaleContain()
{
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    // Transforms
    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QRectF newrect;

    newrect.setX(0);
    newrect.setY(0);
    newrect.setWidth(0);
    newrect.setHeight(0);

    for (int i = 0; i < hpglModel.rowCount(); ++i)
    {
        index = hpglModel.index(i);
        QMutex * mutex;
        itemGroup = NULL;
        hpglModel.dataGroup(index, mutex, itemGroup);
        QMutexLocker rowLocker(mutex);

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontain().";
            return;
        }

        QRectF compRect = itemGroup->boundingRect();
        QPointF compPoint = itemGroup->pos();

        if ((compPoint.x()+compRect.width()) > newrect.width())
        {
            newrect.setWidth(compPoint.x()+compRect.width());
        }
        if ((compPoint.y()+compRect.height()) > newrect.height())
        {
            newrect.setHeight(compPoint.y()+compRect.height());
        }
    }

    ui->graphicsView_view->fitInView(newrect, Qt::KeepAspectRatio);

    handle_newConsoleText("Scene scale set to contain items", Qt::darkGreen);
    handle_zoomChanged("Show all items");
}

void MainWindow::sceneScaleContainSelected()
{
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    // Transforms
    QTransform hpglToPx, itemToScene, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    itemToScene.scale(1016.0/xDpi, 1016.0/yDpi);
    viewFlip.scale(1, -1);

    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QRectF newrect;
    QModelIndexList selectedItems = ui->listView->selectionModel()->selectedIndexes();

    newrect.setX(0);
    newrect.setY(0);
    newrect.setWidth(0);
    newrect.setHeight(0);

    for (int i = 0; i < selectedItems.length(); ++i)
    {
        index = selectedItems.at(i);
        QMutex * mutex;
        itemGroup = NULL;
        hpglModel.dataGroup(index, mutex, itemGroup);
        QMutexLocker rowLocker(mutex);

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in scenescalecontainselected().";
            return;
        }

        QRectF compRect = itemGroup->boundingRect();
        QPointF compPoint = itemGroup->pos();

        if ((compPoint.x()+compRect.width()) > newrect.width())
        {
            newrect.setWidth(compPoint.x()+compRect.width());
        }
        if ((compPoint.y()+compRect.height()) > newrect.height())
        {
            newrect.setHeight(compPoint.y()+compRect.height());
        }
    }

    ui->graphicsView_view->fitInView(newrect, Qt::KeepAspectRatio);

    handle_newConsoleText("Scene scale set to contain items", Qt::darkGreen);
    handle_zoomChanged("Show all items");
}

void MainWindow::rotateSelectedItems(qreal rotation)
{
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QTransform transform;
    qreal translateWidth, translateheight;

    ui->listView->selectionModel()->clearSelection();

    for (int i = 0; i < hpglModel.rowCount(); ++i)
    {
        index = hpglModel.index(i);
        itemGroup = NULL;
        QMutex * mutex;
        hpglModel.dataGroup(index, mutex, itemGroup);
        QMutexLocker rowLocker(mutex);

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in rotateselecteditems().";
            return;
        }

        if (itemGroup->isSelected())
        {
            translateWidth = itemGroup->boundingRect().width();
            translateheight = itemGroup->boundingRect().height();
            transform.translate(translateWidth/2.0, translateheight/2.0);
            transform.rotate(rotation);
            transform.translate(-translateWidth/2.0, -translateheight/2.0);
            itemGroup->setTransform(itemGroup->transform() * transform);
        }
    }
}

void MainWindow::scaleSelectedItems(qreal x, qreal y)
{
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QTransform transform;
    qreal translateWidth, translateheight;

    ui->listView->selectionModel()->clearSelection();

    for (int i = 0; i < hpglModel.rowCount(); ++i)
    {
        index = hpglModel.index(i);
        itemGroup = NULL;
        QMutex * mutex;
        hpglModel.dataGroup(index, mutex, itemGroup);
        QMutexLocker rowLocker(mutex);

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup null in scaleselecteditems().";
            return;
        }

        if (itemGroup->isSelected())
        {
            translateWidth = itemGroup->boundingRect().width();
            translateheight = itemGroup->boundingRect().height();
            transform.translate(translateWidth/2.0, translateheight/2.0);
            transform.scale(x, y);
            transform.translate(-translateWidth/2.0, -translateheight/2.0);
            itemGroup->setTransform(itemGroup->transform() * transform);
        }
    }
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

    // Set scene to view
    ui->graphicsView_view->show();

    sceneSetSceneRect();
    sceneScale11();
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
    QPointF topLeft = widthLine->mapToScene(widthLine->line().p2());
    QPointF bottomLeft = widthLine->mapToScene(widthLine->line().p1());

    hpglModel.constrainItems(bottomLeft, topLeft);
}

void MainWindow::addPolygon(QPersistentModelIndex index, QPolygonF poly)
{
    // Variables
    QPen pen;

    // Set downPen
    get_pen(&pen, "down");

    QGraphicsPolygonItem * gpoly = plotScene.addPolygon(poly, pen);

    hpglModel.addPolygon(index, gpoly);
}


































