#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Set up application-wide QSettings
    init_localplot_settings();

    QApplication a(argc, argv);
    MainWindow w;

    // Open MainWindow
    w.show();

    // Required setup to pass custom item in signal/slot.
    qRegisterMetaType<QVector<QGraphicsPolygonItem*>>("QVector<QGraphicsPolygonItem*>");

    return a.exec();
}
