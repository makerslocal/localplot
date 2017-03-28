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

    // Connect UI buttons
    connect(ui->pushButton_fileSelect, SIGNAL(clicked()), this, SLOT(handle_selectFileBtn()));
    connect(ui->pushButton_doPlot, SIGNAL(clicked()), this, SLOT(handle_plotFileBtn()));
    connect(ui->pushButton_jogPerimeter, SIGNAL(clicked(bool)), this, SLOT(handle_jogPerimeterBtn()));
    connect(ui->pushButton_cancel, SIGNAL(clicked(bool)), this, SLOT(handle_cancelBtn()));
    connect(ui->pushButton_fileRemove, SIGNAL(clicked(bool)), this, SLOT(handle_deleteFileBtn()));
    connect(ui->pushButton_duplicateFile, SIGNAL(clicked(bool)), this, SLOT(handle_duplicateFileBtn()));
    connect(ui->toolButton_rotateLeft, SIGNAL(clicked(bool)), this, SLOT(handle_rotateLeftBtn()));
    connect(ui->toolButton_rotateRight, SIGNAL(clicked(bool)), this, SLOT(handle_rotateRightBtn()));
    connect(ui->toolButton_flipX, SIGNAL(clicked(bool)), this, SLOT(handle_flipXbtn()));
    connect(ui->toolButton_flipY, SIGNAL(clicked(bool)), this, SLOT(handle_flipYbtn()));
    connect(ui->toolButton_autoArrange, SIGNAL(clicked(bool)), this, SLOT(do_binpack()));

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
    hpglModel = new hpglListModel(this);
    // Setup listView and listModel
    ui->listView->setModel(hpglModel);

    connect(hpglModel, SIGNAL(newPolygon(QPersistentModelIndex,QPolygonF)), this, SLOT(addPolygon(QPersistentModelIndex,QPolygonF)));
    connect(hpglModel, SIGNAL(newFileToScene(QPersistentModelIndex)), this, SLOT(newFileToScene(QPersistentModelIndex)));

    sceneSetup();
}

MainWindow::~MainWindow()
{
    for (int i = (hpglModel->rowCount() - 1); i >= 0; --i)
    {
        hpglModel->removeRow(i);
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

void MainWindow::handle_jogPerimeterBtn()
{
    do_plot(true);
}

void MainWindow::handle_plotFileBtn()
{
    do_plot(false);
}

void MainWindow::handle_cancelBtn()
{
    emit please_plotter_cancelPlot();
}

void MainWindow::do_plot(bool jogPerimeter)
{
    ExtPlot * worker;
    if (jogPerimeter)
    {
        QRectF perimeter;
        QModelIndex index;
        QGraphicsItemGroup * itemGroup;
        qreal x1, y1, x2, y2;

        for (int i = 0; i < hpglModel->rowCount(); ++i)
        {
            index = hpglModel->index(i);
            itemGroup = NULL;
            QMutex * mutex;
            hpglModel->dataGroup(index, mutex, itemGroup);
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

            itemGroup->mapRectToScene(itemGroup->boundingRect()).getCoords(&x1, &y1, &x2, &y2);
            if (x1 < perimeter.x())
            {
                perimeter.setX(x1);
            }
            if (y1 < perimeter.y())
            {
                perimeter.setY(y1);
            }
            if (x2 > (perimeter.x() + perimeter.width()))
            {
                perimeter.setWidth(x2 - perimeter.width());
            }
            if (y2 > (perimeter.y() + perimeter.height()))
            {
                perimeter.setHeight(y2 - perimeter.y());
            }
            if (perimeter.x() < 0)
            {
                perimeter.setX(0);
            }
            if (perimeter.y() < 0)
            {
                perimeter.setY(0);
            }
            mutex->unlock();
        }
        worker = new ExtPlot(hpglModel, perimeter);
    }
    else
    {
        worker = new ExtPlot(hpglModel);
    }

    // Process in new thread
    QThread * workerThread = new QThread;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(started()), this, SLOT(handle_extStarted()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(workerThread, SIGNAL(finished()), this, SLOT(handle_extFinished()));
    connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(progress(int)), this, SLOT(handle_plottingPercent(int)));
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), worker, SLOT(cancel()));
    workerThread->start();
}

void MainWindow::do_binpack()
{
    if (ui->pushButton_doPlot->text() == "Plot!")
    {
        // Process in new thread
        QThread * workerThread = new QThread;
        ExtBinPack * worker = new ExtBinPack(hpglModel);
        worker->moveToThread(workerThread);
        connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
        connect(workerThread, SIGNAL(started()), this, SLOT(handle_extStarted()));
        connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(workerThread, SIGNAL(finished()), this, SLOT(handle_extFinished()));
        connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(worker, SIGNAL(packedRect(QPersistentModelIndex,QRectF)), this, SLOT(handle_packedRect(QPersistentModelIndex,QRectF)));
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

void MainWindow::handle_packedRect(QPersistentModelIndex index, QRectF rect)
{
    QGraphicsItemGroup * itemGroup;
    itemGroup = NULL;
    QMutex * mutex;
    QSettings settings;

    hpglModel->dataGroup(index, mutex, itemGroup);
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

    double padding = settings.value("device/cutoutboxes/padding", SETDEF_DEVICE_CUTOUTBOXES_PADDING).toDouble();
    if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
    {
        padding = padding * 2.54;
    }
    padding = padding * 1016.0;

    if (static_cast<int>(itemGroup->sceneBoundingRect().width()) != static_cast<int>(rect.width()-(padding))
            && static_cast<int>(itemGroup->sceneBoundingRect().width()) == static_cast<int>(rect.height()-(padding)))
    {
        int translateWidth, translateheight;
        QTransform transform;
        translateWidth = itemGroup->boundingRect().width();
        translateheight = itemGroup->boundingRect().height();
        transform.translate(translateWidth/2.0, translateheight/2.0);
        transform.rotate(90);
        transform.translate(-translateWidth/2.0, -translateheight/2.0);
        itemGroup->setTransform(itemGroup->transform() * transform);
        mutex->unlock();
        sceneConstrainItems();
        mutex->lock();
    }
    QPointF posFinal, posItem, posScene;
    QRectF grect;
    posFinal = QPointF(rect.x(), rect.y());
    posItem = itemGroup->pos();
    grect = itemGroup->sceneBoundingRect();
    posScene = grect.topLeft();
    qDebug() << posFinal << posItem << posScene;
    posItem.setX(posItem.x() + (posFinal.x() - posScene.x()));
    posItem.setY(posItem.y() + (posFinal.y() - posScene.y()));
    qDebug() << posItem;
    itemGroup->setPos(posItem);

    mutex->unlock();
}

void MainWindow::do_cancelPlot()
{
    emit please_plotter_cancelPlot();
}

void MainWindow::handle_duplicateFileBtn()
{
    hpglModel->duplicateSelectedRows();
}

void MainWindow::do_enableUI(bool enabled)
{
    ui->pushButton_fileRemove->setEnabled(enabled);
    ui->pushButton_fileSelect->setEnabled(enabled);
    ui->graphicsView_view->setEnabled(enabled);
    ui->actionLoad_File->setEnabled(enabled);
//    progressBar_plotting->setValue(0);
    ui->pushButton_doPlot->setEnabled(enabled);
    ui->pushButton_jogPerimeter->setEnabled(enabled);
    ui->pushButton_cancel->setEnabled(!enabled);
}

void MainWindow::handle_extStarted()
{
    do_enableUI(false);
}

void MainWindow::handle_extFinished()
{
    do_enableUI(true);
}

void MainWindow::handle_plotSceneSelectionChanged()
{
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QVector<QGraphicsPolygonItem *> * items;
    QPen _selectedPen;

    ui->listView->selectionModel()->clearSelection();

    for (int i = 0; i < hpglModel->rowCount(); ++i)
    {
        index = hpglModel->index(i);
        itemGroup = NULL;
        items = NULL;
        QMutex * mutex;
        hpglModel->dataItemsGroup(index, mutex, itemGroup, items);
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

    for (int i = 0; i < hpglModel->rowCount(); ++i)
    {
        index = hpglModel->index(i);
        itemGroup = NULL;
        items = NULL;
        QMutex * mutex;
        hpglModel->dataItemsGroup(index, mutex, itemGroup, items);
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
    ExtLoadFile * worker = new ExtLoadFile(hpglModel);
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
    ExtEta * worker = new ExtEta(hpglModel);
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
    QSettings settings;

    QMutex * mutex;
    hpglModel->dataGroup(_index, mutex, itemGroup);
    QMutexLocker rowLocker(mutex);

    if (itemGroup == NULL)
    {
        qDebug() << "Error: itemgroup is null in newFileToScene().";
        return;
    }

    if (settings.value("device/cutoutboxes", SETDEF_DEVICE_CUTOUTBOXES).toBool())
    {
        double padding = settings.value("device/cutoutboxes/padding", SETDEF_DEVICE_CUTOUTBOXES_PADDING).toDouble();
        if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
        {
            padding = padding * 2.54;
        }
        padding = padding * 1016.0;
        QRectF cutoutRect = itemGroup->boundingRect();
        cutoutRect = cutoutRect.marginsAdded(QMarginsF(padding, padding, padding, padding));
        cutoutRect.moveTo(0, 0);
        itemGroup->moveBy(padding, padding);
        rowLocker.unlock();
        addPolygon(_index, static_cast<QPolygonF>(cutoutRect));
        rowLocker.relock();
    }

    plotScene.addItem(itemGroup);
    rowLocker.unlock();

    hpglModel->setGroupFlag(_index, QGraphicsItem::ItemIsMovable, true);
    hpglModel->setGroupFlag(_index, QGraphicsItem::ItemIsSelectable, true);

    itemGroup->setSelected(true);
    rowLocker.unlock();

    handle_plotSceneSelectionChanged();
    sceneConstrainItems();
    sceneSetSceneRect();
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
        hpglModel->dataItemsGroup(index, mutex, itemGroup, items);
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
        hpglModel->removeRow(list[i].row());
    }
}

void MainWindow::handle_rotateLeftBtn()
{
    hpglModel->rotateSelectedItems(90);
}

void MainWindow::handle_rotateRightBtn()
{
    hpglModel->rotateSelectedItems(-90);
}

void MainWindow::handle_flipXbtn()
{
    hpglModel->scaleSelectedItems(-1, 1);
}

void MainWindow::handle_flipYbtn()
{
    hpglModel->scaleSelectedItems(1, -1);
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
    return (QLineF(0, 0, 0, (1016.0 * length)));
}

void MainWindow::sceneScaleWidth()
{
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    ui->graphicsView_view->fitInView(
        plotScene.sceneRect(),
        Qt::KeepAspectRatio);
    sceneSetGrid(xDpi, yDpi, ui->graphicsView_view->transform());
    handle_newConsoleText("Scene scale set to view all", Qt::darkGreen);
    handle_zoomChanged("Vinyl width");
}

void MainWindow::sceneSetGrid(int xDpi, int yDpi, QTransform _transform)
{
    QSettings settings;
    if (!settings.value("mainwindow/grid", SETDEF_MAINWINDOW_GRID).toBool())
    {
        return;
    }
    int size = settings.value("mainwindow/grid/size", SETDEF_MAINWINDOW_GRID_SIZE).toInt();

    QTransform hpglToPx;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);

    // m11 and m22 are horizontal and vertical scaling
    qreal mx, my;
    mx = hpglToPx.m11() / qAbs(_transform.m11());
    my = hpglToPx.m22() / qAbs(_transform.m22());

    int gridX, gridY;
    if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
    {
        gridX = ((xDpi*size*2.54) / mx);
        gridY = ((yDpi*size*2.54) / my);
    }
    else
    {
        gridX = ((xDpi*size) / mx);
        gridY = ((yDpi*size) / my);
    }

    qDebug() << "grid: " << gridX << gridY << mx << my;
    QImage grid(gridX, gridY, QImage::Format_RGB32);
    QRgb value;

    value = qRgb(150, 200, 150);
    for (int x = 0; x < gridX; ++x)
    {
        for (int y = 0; y < gridY; ++y)
        {
            grid.setPixelColor(QPoint(x, y), Qt::white);
        }
    }
    for (int i = 0; i < gridX; ++i)
    {
        grid.setPixel(i, 0, value);
    }
    for (int i = 0; i < gridY; ++i)
    {
        grid.setPixel(0, i, value);
    }

    QBrush gridBrush(grid);

    qDebug() << hpglToPx << _transform;
    gridBrush.setTransform((_transform.inverted()));

    ui->graphicsView_view->setBackgroundBrush(gridBrush);
//    plotScene.setBackgroundBrush(gridBrush);
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
    sceneSetGrid(xDpi, yDpi, ui->graphicsView_view->transform());

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

    for (int i = 0; i < hpglModel->rowCount(); ++i)
    {
        index = hpglModel->index(i);
        QMutex * mutex;
        itemGroup = NULL;
        hpglModel->dataGroup(index, mutex, itemGroup);
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
    sceneSetGrid(xDpi, yDpi, ui->graphicsView_view->transform());

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
        hpglModel->dataGroup(index, mutex, itemGroup);
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
    sceneSetGrid(xDpi, yDpi, ui->graphicsView_view->transform());

    handle_newConsoleText("Scene scale set to contain items", Qt::darkGreen);
    handle_zoomChanged("Show all items");
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

    hpglModel->constrainItems(bottomLeft, topLeft);
}

void MainWindow::addPolygon(QPersistentModelIndex index, QPolygonF poly)
{
    // Variables
    QPen pen;

    // Set downPen
    get_pen(&pen, "down");

    QGraphicsPolygonItem * gpoly = plotScene.addPolygon(poly, pen);

    hpglModel->addPolygon(index, gpoly);
}


































