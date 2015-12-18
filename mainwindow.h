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

public slots:
    void do_refreshSerialList();
    void do_openSerial();
    void do_closeSerial();

    void handle_serialOpened();
    void handle_serialClosed();
    void handle_serialConnectBtn();

signals:


private:
    Ui::MainWindow *ui;
    QSerialPortInfo serialPorts;
    QPointer<QSerialPort> serialBuffer;

};

#endif // MAINWINDOW_H
