/**
 * Localplot - Main UI thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QFileDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsTextItem>
#include <QPushButton>
#include <QGraphicsItem>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QListView>
#include <QModelIndex>
#include <QModelIndexList>
#include <QStringListModel>
#include <QMainWindow>
#include <QTextBrowser>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QLabel>
#include <QFrame>
#include <QSplitter>
#include <QGraphicsLineItem>
#include <QToolButton>
#include <QImage>
#include <QBrush>
#include <QPixmap>
#include <QProcess>

#include <qmath.h>

#include "settings.h"
#include "extplot.h"
#include "extloadfile.h"
#include "exteta.h"
#include "hpglgraphicsview.h"
#include "hpgllistmodel.h"
#include "extbinpack.h"
#include "dialogprogress.h"

QString timeStamp();

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static QLineF get_widthLine();

/*
 * please_  for signals to threads
 */
signals:
    void please_plotter_openSerial();
    void please_plotter_closeSerial();
    void please_plotter_doPlot(hpglListModel *);
    void please_plotter_cancelPlot();
    void please_plotter_loadFile(const QPersistentModelIndex, const hpglListModel *);
    void please_plotter_procEta(hpglListModel *);

/*
 * do_      for ui/proc action
 * update_  for settings change
 * handle_  for ui update
 */
private slots:
    void do_openDialogAbout();
    void do_openDialogSettings();

    void runFinishedCommand();

    // UI
    void handle_selectFileBtn();
    void handle_deleteFileBtn();
    void handle_rotateLeftBtn();
    void handle_rotateRightBtn();
    void handle_duplicateFileBtn();
    void handle_cancelBtn();
    void handle_flipXbtn();
    void handle_flipYbtn();
    void handle_newConsoleText(QString text, QColor textColor);
    void handle_newConsoleText(QString text);
    void handle_zoomChanged(QString text);
    void handle_listViewClick();
    void handle_plotSceneSelectionChanged();
    void handle_plottingEta(double eta);
    void handle_splitterMoved();

    // View Zooming
    void sceneZoomVinyl();
    void sceneZoomActual();
    void sceneZoomItems();
    void sceneZoomSelected();
    void sceneSetGrid();

    // View/Scene
    void sceneSetup();
    void get_pen(QPen *_pen, QString _name);
    void sceneSetSceneRect();
    void sceneConstrainItems();
    void addPolygon(QPersistentModelIndex index, QPolygonF poly);
    void newFileToScene(QPersistentModelIndex _index);
    void createCutoutBox(QPersistentModelIndex _index);
    void handle_packedRect(QPersistentModelIndex index, QRectF rect);

    // plotter thread
    void do_plot();
    void do_jog();
    void do_cancelPlot();
    void do_procEta();
    void do_binpack();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QFrame * statusBarDivider();
    QPersistentModelIndex createHpglFile(file_uid _file);

    Ui::MainWindow *ui;
    QGraphicsScene plotScene;
    hpglListModel * hpglModel;
    QGraphicsLineItem * widthLine;

    // Status bar
    QLabel * label_eta;
    QLabel * label_status;
    QLabel * label_zoom;
};

#endif // MAINWINDOW_H
