#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
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

public slots:
    void do_openSerial();
    void do_closeSerial();
    void do_loadFile();
    void do_plot();
    void do_drawView();
    void do_updatePens();

    void update_penDown();
    void update_penUp();
    void update_filePath();
//    void update_serialDevice();

    void handle_serialOpened();
    void handle_serialClosed();
    void handle_serialConnectBtn();
    void handle_selectFileBtn();
    void handle_objectTransform();
//    void handle_autoTranslateBtn();

signals:
    //

private slots:
//    void open();
    void open_dialogAbout();
    void open_dialogSettings();

private:
    Ui::MainWindow *ui;
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;
    QFile inputFile;
    QList<hpgl_obj> objList;
    QGraphicsScene plotScene;
    QSettings * settings;
};

#endif // MAINWINDOW_H
