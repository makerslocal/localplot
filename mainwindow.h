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

    void handle_serialOpened();
    void handle_serialClosed();
    void handle_serialConnectBtn();
    void handle_selectFileBtn();

signals:


private:
    Ui::MainWindow *ui;
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;
    QFile inputFile;
    QVector<QString> cmdList;
    QGraphicsScene plotScene;

};

#endif // MAINWINDOW_H
