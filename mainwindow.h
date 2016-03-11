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

#include "hpgl_obj.h"

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

public slots:
    void do_refreshSerialList();
    void do_openSerial();
    void do_closeSerial();
    void do_loadFile();
    void do_plot();
    void do_drawView();
    void do_updatePens();

    void handle_serialOpened();
    void handle_serialClosed();
    void handle_serialConnectBtn();
    void handle_selectFileBtn();

signals:
    //

private:
    Ui::MainWindow *ui;
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;
    QFile inputFile;
    QList<hpgl_obj> objList;
    QGraphicsScene plotScene;
    QPen downPen;
    QPen upPen;
    QScreen * screen;
};

#endif // MAINWINDOW_H
