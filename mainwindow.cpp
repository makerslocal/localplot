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
    hpglModel = new hpglListModel(this);

    // Connect UI actions
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(handle_selectFileBtn()));
    connect(ui->actionDelete, SIGNAL(triggered(bool)), this, SLOT(handle_deleteFileBtn()));
    connect(ui->actionDuplicate, SIGNAL(triggered(bool)), this, SLOT(handle_duplicateFileBtn()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(do_openDialogAbout()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(do_openDialogSettings()));
    connect(ui->actionZoom_Actual, SIGNAL(triggered(bool)), this, SLOT(sceneZoomActual()));
    connect(ui->actionZoom_Vinyl, SIGNAL(triggered(bool)), this, SLOT(sceneZoomVinyl()));
    connect(ui->actionZoom_Items, SIGNAL(triggered(bool)), this, SLOT(sceneZoomItems()));
    connect(ui->actionZoom_Selected, SIGNAL(triggered(bool)), this, SLOT(sceneZoomSelected()));
    connect(ui->actionRotate_Left, SIGNAL(triggered(bool)), this, SLOT(handle_rotateLeftBtn()));
    connect(ui->actionRotate_Right, SIGNAL(triggered(bool)), this, SLOT(handle_rotateRightBtn()));
    connect(ui->actionFlip_Horizontal, SIGNAL(triggered(bool)), this, SLOT(handle_flipXbtn()));
    connect(ui->actionFlip_Vertical, SIGNAL(triggered(bool)), this, SLOT(handle_flipYbtn()));
    connect(ui->actionAuto_Arrange, SIGNAL(triggered(bool)), this, SLOT(do_binpack()));
    connect(ui->actionPlot, SIGNAL(triggered(bool)), this, SLOT(do_plot()));
    connect(ui->actionJog, SIGNAL(triggered(bool)), this, SLOT(do_jog()));
    connect(ui->actionZoom_In, SIGNAL(triggered(bool)), this, SLOT(sceneZoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered(bool)), this, SLOT(sceneZoomOut()));

    // Connect UI buttons to actions
    connect(ui->pushButton_fileSelect, SIGNAL(clicked(bool)), ui->actionLoad_File, SLOT(trigger()));
    connect(ui->pushButton_doPlot, SIGNAL(clicked(bool)), ui->actionPlot, SLOT(trigger()));
    connect(ui->pushButton_jogPerimeter, SIGNAL(clicked(bool)), ui->actionJog, SLOT(trigger()));
    connect(ui->pushButton_fileRemove, SIGNAL(clicked(bool)), ui->actionDelete, SLOT(trigger()));
    connect(ui->pushButton_duplicateFile, SIGNAL(clicked(bool)), ui->actionDuplicate, SLOT(trigger()));

    // Connect other UI elements
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(handle_splitterMoved()));
    connect(&plotScene, SIGNAL(changed(QList<QRectF>)), this, SLOT(sceneConstrainItems()));
    connect(ui->graphicsView_view, SIGNAL(mouseReleased()), this, SLOT(sceneSetSceneRect()));
    connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(handle_listViewClick()));
    connect(&plotScene, SIGNAL(selectionChanged()), this, SLOT(handle_plotSceneSelectionChanged()));
    connect(ui->graphicsView_view, SIGNAL(zoomDelta(int)), this, SLOT(sceneZoomDelta(int)));

    // Connect everything else
    connect(hpglModel, SIGNAL(newPolygon(QPersistentModelIndex,QPolygonF)), this, SLOT(addPolygon(QPersistentModelIndex,QPolygonF)));
    connect(hpglModel, SIGNAL(newFileToScene(QPersistentModelIndex)), this, SLOT(newFileToScene(QPersistentModelIndex)));

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
    if (settings.contains("mainwindow/splitter/geometry"))
    {
        ui->splitter->restoreGeometry(settings.value("mainwindow/splitter/geometry").toByteArray());
    }
    if (settings.contains("mainwindow/splitter/state"))
    {
        ui->splitter->restoreState(settings.value("mainwindow/splitter/state").toByteArray());
    }

    // Setup statusbar
    label_eta = new QLabel;
    label_status = new QLabel;
    label_zoom = new QLabel;
    label_eta->setText("ETA: NA");
    label_status->setText("Status label created.");
    label_zoom->setText("Zoom: NA");
    statusBar()->addPermanentWidget(label_status);
    statusBar()->addPermanentWidget(statusBarDivider());
    statusBar()->addPermanentWidget(label_eta);
    statusBar()->addPermanentWidget(statusBarDivider());
    statusBar()->addPermanentWidget(label_zoom);

    ui->graphicsView_view->setScene(&plotScene);
    ui->listView->setModel(hpglModel);

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

void MainWindow::handle_splitterMoved()
{
    QSettings settings;
    settings.setValue("mainwindow/splitter/state", ui->splitter->saveState());
    settings.setValue("mainwindow/splitter/geometry", ui->splitter->saveGeometry());
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
    sceneSetGrid();
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
    qDebug() << "ETA: " << eta;
    QString labelText = "ETA: ";
    int minutes = (eta / 60);
    if (minutes)
    {
        labelText += QString::number(minutes);
        labelText += "m ";
    }
    labelText += QString::number(eta - (minutes*60),'g',2);
    labelText += "s";
    label_eta->setText(labelText);
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

void MainWindow::handle_cancelBtn()
{
    emit please_plotter_cancelPlot();
}

void MainWindow::do_plot()
{
    ExtPlot * worker;

    worker = new ExtPlot(hpglModel);

    // Create progress window
    DialogProgress * newwindow;
    newwindow = new DialogProgress(this);
    newwindow->setWindowTitle("Plotting Progress");
    newwindow->enableHookCheckbox();

    // Create plotting process in new thread
    QThread * workerThread = new QThread;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), this, SLOT(runFinishedCommand()));
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));

    // Connect progress window
    connect(newwindow, SIGNAL(do_cancel()), worker, SLOT(cancel()));
    connect(worker, SIGNAL(finished()), newwindow, SLOT(close()));
    connect(worker, SIGNAL(progress(int)), newwindow, SLOT(handle_updateProgress(int)));

    // Start
    workerThread->start();
    newwindow->exec();
}

void MainWindow::do_jog()
{
    ExtPlot * worker;

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

    // Create progress window
    DialogProgress * newwindow;
    newwindow = new DialogProgress(this);
    newwindow->setWindowTitle("Jogging Progress");

    // Create plotting process in new thread
    QThread * workerThread = new QThread;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));

    // Connect progress window
    connect(newwindow, SIGNAL(do_cancel()), worker, SLOT(cancel()));
    connect(worker, SIGNAL(finished()), newwindow, SLOT(close()));
    connect(worker, SIGNAL(progress(int)), newwindow, SLOT(handle_updateProgress(int)));

    // Start
    workerThread->start();
    newwindow->exec();
}

void MainWindow::runFinishedCommand()
{
    QSettings settings;
    QProcess * proc = new QProcess(this);
    bool doRun = settings.value("hook/finished", SETDEF_HOOK_FINISHED).toBool();
    QString procCmd = settings.value("hook/finished/path", SETDEF_HOOK_FINISHED_PATH).toString();
    if (procCmd.isEmpty() || (!doRun) )
    {
        qDebug() << "Not running a post hook.";
        return;
    }
    qDebug() << "Running post hook: " << procCmd;
    proc->start(procCmd);
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

    // Create progress window
    DialogProgress * newwindow;
    newwindow = new DialogProgress(this);
    newwindow->setWindowTitle("Loading File Progress");

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
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    if (settings.value("device/cutoutboxes", SETDEF_DEVICE_CUTOUTBOXES).toBool())
    {
        connect(worker, SIGNAL(finished(QPersistentModelIndex)), this, SLOT(createCutoutBox(QPersistentModelIndex)));
    }

    // Connect progress window
    connect(newwindow, SIGNAL(do_cancel()), worker, SLOT(cancel()));
    connect(worker, SIGNAL(finished(QPersistentModelIndex)), newwindow, SLOT(close()));
    connect(worker, SIGNAL(progress(int)), newwindow, SLOT(handle_updateProgress(int)));

    // Start
    workerThread->start();
    newwindow->exec();
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
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    workerThread->start();
}

void MainWindow::do_binpack()
{
    // Create progress window
    DialogProgress * newwindow;
    newwindow = new DialogProgress(this);
    newwindow->setWindowTitle("Arranging Progress");

    // Process in new thread
    QThread * workerThread = new QThread;
    ExtBinPack * worker = new ExtBinPack(hpglModel);
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(packedRect(QPersistentModelIndex,QRectF)), this, SLOT(handle_packedRect(QPersistentModelIndex,QRectF)));
    connect(worker, SIGNAL(statusUpdate(QString,QColor)), this, SLOT(handle_newConsoleText(QString,QColor)));
    connect(this, SIGNAL(please_plotter_cancelPlot()), worker, SLOT(cancel()));

    // Connect progress window
    connect(newwindow, SIGNAL(do_cancel()), worker, SLOT(cancel()));
    connect(worker, SIGNAL(finished()), newwindow, SLOT(close()));
    connect(worker, SIGNAL(progress(int)), newwindow, SLOT(handle_updateProgress(int)));

    // Start
    workerThread->start();
    newwindow->exec();
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

void MainWindow::createCutoutBox(QPersistentModelIndex _index)
{
    QSettings settings;
    QGraphicsItemGroup * itemGroup = NULL;
    QVector<QGraphicsPolygonItem *> * items;

    QMutex * mutex;
    hpglModel->dataItemsGroup(_index, mutex, itemGroup, items);
    QMutexLocker rowLocker(mutex);

    if (itemGroup == NULL)
    {
        qDebug() << "Error: itemgroup is null.";
        return;
    }
    if (items == NULL)
    {
        qDebug() << "Error: items is null.";
        return;
    }

    double padding = settings.value("device/cutoutboxes/padding", SETDEF_DEVICE_CUTOUTBOXES_PADDING).toDouble();
    if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
    {
        padding = padding * 2.54;
    }
    padding = padding * 1016.0;
    QRectF cutoutRect = itemGroup->boundingRect();
    cutoutRect = cutoutRect.marginsAdded(QMarginsF(padding, padding, padding, padding));
    cutoutRect.moveTo(0, 0);

//    itemGroup->moveBy(padding, padding);
    for (int i = 0; i < items->length(); ++i)
    {
        items->at(i)->moveBy(padding, padding);
    }

    rowLocker.unlock();
    addPolygon(_index, static_cast<QPolygonF>(cutoutRect));
    rowLocker.relock();
}

void MainWindow::newFileToScene(QPersistentModelIndex _index)
{
    QGraphicsItemGroup * itemGroup = NULL;
    ui->listView->setCurrentIndex(_index);

    QMutex * mutex;
    hpglModel->dataGroup(_index, mutex, itemGroup);
    QMutexLocker rowLocker(mutex);

    if (itemGroup == NULL)
    {
        qDebug() << "Error: itemgroup is null in newFileToScene().";
        return;
    }

    plotScene.addItem(itemGroup);
    rowLocker.unlock();

    hpglModel->setGroupFlag(_index, QGraphicsItem::ItemIsMovable, true);
    hpglModel->setGroupFlag(_index, QGraphicsItem::ItemIsSelectable, true);

    ui->listView->selectionModel()->clearSelection();
    handle_listViewClick();
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

void MainWindow::sceneZoomVinyl()
{
    ui->graphicsView_view->fitInView(
        plotScene.sceneRect(),
        Qt::KeepAspectRatio);
    sceneSetGrid();
    handle_newConsoleText("Scene scale set to view all", Qt::darkGreen);
    handle_zoomChanged("Vinyl width");
}

void MainWindow::sceneSetGrid()
{
    QTransform _transform = ui->graphicsView_view->transform();
    // physicalDpi is the number of pixels in an inch
    int xDpi = ui->graphicsView_view->physicalDpiX();
    int yDpi = ui->graphicsView_view->physicalDpiY();
    QSettings settings;
    if (!settings.value("mainwindow/grid", SETDEF_MAINWINDOW_GRID).toBool())
    {
        ui->graphicsView_view->setBackgroundBrush(Qt::NoBrush);
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
        gridX = (((xDpi*size)/2.54) / mx);
        gridY = (((yDpi*size)/2.54) / my);
    }
    else
    {
        gridX = ((xDpi*size) / mx);
        gridY = ((yDpi*size) / my);
    }

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

    gridBrush.setTransform((_transform.inverted()));

    ui->graphicsView_view->setBackgroundBrush(gridBrush);
//    plotScene.setBackgroundBrush(gridBrush);
}

void MainWindow::sceneZoomActual()
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
    sceneSetGrid();

    handle_newConsoleText("Scene scale set to 1:1", Qt::darkGreen);
    handle_zoomChanged("Actual size");
}

void MainWindow::sceneZoomItems()
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
    sceneSetGrid();

    handle_newConsoleText("Scene scale set to contain items", Qt::darkGreen);
    handle_zoomChanged("Show all items");
}

void MainWindow::sceneZoomSelected()
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
    sceneSetGrid();

    handle_newConsoleText("Scene scale set to contain items", Qt::darkGreen);
    handle_zoomChanged("Show all items");
}

void MainWindow::sceneZoomDelta(int delta)
{
    QTransform oldtransform = ui->graphicsView_view->transform();
    QTransform scaleTransform;
    scaleTransform.scale((1.0+(5.0/delta)), (1.0+(5.0/delta)));
    ui->graphicsView_view->setTransform(oldtransform * scaleTransform);
    sceneSetGrid();
    handle_newConsoleText("Scene scale set to scroll wheel zoom.");
    handle_zoomChanged("Custom");
}

void MainWindow::sceneZoomOut()
{
    sceneZoomDelta(-30);
}

void MainWindow::sceneZoomIn()
{
    sceneZoomDelta(30);
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
    sceneZoomActual();
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


































