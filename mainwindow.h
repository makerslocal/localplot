/**
 * Localplot - Main UI thread header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "hpgl.h"
#include "settings.h"
#include "ancilla.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString timeStamp();
    int get_nextInt(QString input, int *index);
    QGraphicsScene penDownDemoScene;
    QGraphicsScene penUpDemoScene;
    QPen downPen;
    QPen upPen;

/*
 * please_  for signals to threads
 */
signals:
    void please_plotter_openSerial();
    void please_plotter_closeSerial();
    void please_plotter_doPlot(QList<hpgl> * _objList);
    void please_plotter_cancelPlot();

/*
 * do_      for ui action
 * update_  for settings change
 * handle_  for ui update
 */
private slots:
    void do_loadFile();
    void do_drawView();
    void do_updatePens();
    void do_openDialogAbout();
    void do_openDialogSettings();

    // plotter thread
//    void do_openSerial();
//    void do_closeSerial();
    void do_plot();
    void do_cancelPlot();
    void handle_ancillaThreadStart();
    void handle_ancillaThreadQuit();

    void update_filePath();

    void handle_serialOpened();
    void handle_serialClosed();
    void handle_serialConnectBtn();
    void handle_selectFileBtn();
    void handle_objectTransform();
    void handle_plotStarted();
    void handle_plotCancelled();
//    void handle_autoTranslateBtn();
    void handle_plottingPercent(int percent);

private:
    Ui::MainWindow *ui;
    QList<hpgl> objList;
    QGraphicsScene plotScene;
    QPointer<QSettings> settings;
    AncillaryThread * ancilla;
};

#endif // MAINWINDOW_H
