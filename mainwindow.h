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

#include "hpgl.h"
#include "settings.h"
#include "ancilla.h"
#include "etc.h"
#include "hpglgraphicsview.h"

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
    void please_plotter_doPlot(QVector<hpgl_file *> *);
    void please_plotter_cancelPlot();
    void please_plotter_loadFile(file_uid _file);

/*
 * do_      for ui/proc action
 * update_  for settings change
 * handle_  for ui update
 */
private slots:
    void do_loadFile(file_uid _file);
    void do_openDialogAbout();
    void do_openDialogSettings();

    // UI
    void handle_selectFileBtn();
    void handle_deleteFileBtn();
    void handle_plotStarted();
    void handle_plotCancelled();
    void handle_plotFinished();
    void handle_plottingPercent(int percent);
    void handle_listViewClick();
    void handle_plotSceneSelectionChanged();

    // View/Scene
    void sceneSetup();
    void get_pen(QPen *_pen, QString _name);
    void deleteHpglFile(hpgl_file *_hpgl);
    void sceneSetSceneRect();
    void sceneConstrainItems();
    void addPolygon(file_uid _file, QPolygonF poly);
    hpgl_file *createHpglFile(file_uid _file);

    // plotter thread
    void do_plot();
    void do_cancelPlot();
    void handle_ancillaThreadStart();
    void handle_ancillaThreadQuit();
    void handle_ancillaThreadStatus(QString _consoleText);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QGraphicsScene plotScene;
    QThread ancillaryThreadInstance;
    QPointer<AncillaryThread> ancilla;
    QVector<hpgl_file *> hpglList;
    QStringListModel * listModel;
    QGraphicsLineItem * widthLine;
};

#endif // MAINWINDOW_H
