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

#include "hpgl.h"
#include "settings.h"
#include "ancilla.h"
#include "etc.h"

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
    void please_plotter_doPlot(const QVector<QGraphicsPolygonItem *>);
    void please_plotter_cancelPlot();
    void please_plotter_loadFile(QString filePath);

/*
 * do_      for ui action
 * update_  for settings change
 * handle_  for ui update
 */
private slots:
    void do_loadFile(QString filePath);
    void do_drawView();
    void do_updatePens();
    void do_openDialogAbout();
    void do_openDialogSettings();

    // plotter thread
    void do_plot();
    void do_cancelPlot();
    void handle_ancillaThreadStart();
    void handle_ancillaThreadQuit();
    void sceneClearHpgl();
    void sceneSetSceneRect();
    void handle_ancillaThreadStatus(QString _consoleText);

    void update_filePath();

    void handle_selectFileBtn();
    void handle_plotStarted();
    void handle_plotCancelled();
    void handle_plotFinished();
    void handle_plottingPercent(int percent);

    void addPolygon(QPolygonF poly);

private:
    Ui::MainWindow *ui;
    QGraphicsScene plotScene;
    QPen downPen;
    QPen upPen;
    QThread ancillaryThreadInstance;
    QPointer<AncillaryThread> ancilla;
    QVector<QGraphicsPolygonItem *> hpgl_items;
    QTimer drawTimer; // Measures performance of drawView()
};

#endif // MAINWINDOW_H
