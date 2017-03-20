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

#include "settings.h"
#include "extplot.h"
#include "extloadfile.h"
#include "exteta.h"
#include "hpglgraphicsview.h"
#include "hpgllistmodel.h"

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

    // UI
    void handle_selectFileBtn();
    void handle_deleteFileBtn();
    void handle_rotateLeftBtn();
    void handle_rotateRightBtn();
    void handle_flipXbtn();
    void handle_flipYbtn();
    void handle_plotStarted();
    void handle_plotFinished();
    void handle_plottingPercent(int percent);
    void handle_newConsoleText(QString text, QColor textColor);
    void handle_newConsoleText(QString text);
    void handle_zoomChanged(QString text);
    void handle_listViewClick();
    void handle_plotSceneSelectionChanged();
    void handle_plottingEta(double eta);

    // Item transformations
    void rotateSelectedItems(qreal rotation);
    void scaleSelectedItems(qreal x, qreal y);

    // View Zooming
    void sceneScaleWidth();
    void sceneScale11();
    void sceneScaleContain();
    void sceneScaleContainSelected();

    // View/Scene
    void sceneSetup();
    void get_pen(QPen *_pen, QString _name);
    void sceneSetSceneRect();
    void sceneConstrainItems();
    void addPolygon(QPersistentModelIndex index, QPolygonF poly);
    void newFileToScene(QPersistentModelIndex _index);

    // plotter thread
    void do_plot();
    void do_cancelPlot();
    void do_procEta();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QFrame * statusBarDivider();
    QPersistentModelIndex createHpglFile(file_uid _file);
    QLineF get_widthLine();

    Ui::MainWindow *ui;
    QGraphicsScene plotScene;
    hpglListModel hpglModel;
    QGraphicsLineItem * widthLine;

    // Status bar
    QProgressBar * progressBar_plotting;
    QLabel * label_eta;
    QLabel * label_status;
    QLabel * label_zoom;
};

#endif // MAINWINDOW_H
