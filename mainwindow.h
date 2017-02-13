#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QVector>
#include <QList>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPen>
#include <QScreen>
#include <QCoreApplication>
#include <QSettings>
#include <QThread>
#include <QProcess>

#include "hpgl_obj.h"
#include "settings.h"
#include "plotter.h"

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
    void please_plotter_doPlot(QList<hpgl_obj> _objList);

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

    void update_filePath();

    void handle_serialOpened();
    void handle_serialClosed();
    void handle_serialConnectBtn();
    void handle_selectFileBtn();
    void handle_objectTransform();
//    void handle_autoTranslateBtn();

private:
    Ui::MainWindow *ui;
    QFile inputFile;
    QList<hpgl_obj> objList;
    QGraphicsScene plotScene;
    QSettings * settings;
    Plotter * plotter;
    QThread threadPlotter;
};

#endif // MAINWINDOW_H
