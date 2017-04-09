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
#include <QImage>
#include <QBrush>
#include <QPixmap>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <qtconcurrentrun.h>
#include <QGraphicsDropShadowEffect>

#include <qmath.h>
#include <unistd.h>

#include "settings.h"
#include "ext/plot.h"
#include "ext/loadfile.h"
#include "ext/eta.h"
#include "hpglgraphicsview.h"
#include "hpgllistmodel.h"
#include "ext/binpack.h"
#include "dialog/dialogprogress.h"

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
    static QLineF get_widthLine();

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

    void runFinishedCommand();

    // UI
    void handle_selectFileBtn();
    void handle_deleteFileBtn();
    void handle_rotateLeftBtn();
    void handle_rotateRightBtn();
    void handle_duplicateFileBtn();
    void handle_cancelBtn();
    void handle_flipXbtn();
    void handle_flipYbtn();
    void handle_newConsoleText(QString text, QColor textColor);
    void handle_newConsoleText(QString text);
    void handle_zoomChanged(QString text);
    void handle_vinylLengthChanged(int length);
    void handle_listViewClick();
    void handle_plotSceneSelectionChanged();
    void handle_plottingEta(double eta);
    void handle_splitterMoved();

    // View/Scene
    void sceneSetup();
    void get_pen(QPen *_pen, QString _name);
    void sceneSetSceneRect(QRectF rect = QRectF());
    void sceneConstrainItems();
    void addPolygon(QPersistentModelIndex index, QPolygonF poly);
    void newFileToScene(QPersistentModelIndex _index);
    void handle_packedRect(QPersistentModelIndex index, QRectF rect);
    void handle_cutoutBoxesToggle(bool checked);
    void setGrid();

    // plotter thread
    void do_plot();
    void do_jog();
    void do_cancelPlot();
    void do_procEta();
    void do_binpack();

    // URLs
    void handle_openSourceCode();
    void handle_openBugTracker();
    void handle_openWiki();

    // imports
    void checkImportScripts();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QFrame * statusBarDivider();
    QPersistentModelIndex createHpglFile(file_uid _file);

    Ui::MainWindow *ui;
    QGraphicsScene plotScene;
    hpglListModel * hpglModel;
    QGraphicsLineItem * widthLine;
    QGraphicsRectItem * vinyl;

    // Status bar
    QLabel * label_eta;
    QLabel * label_status;
    QLabel * label_zoom;
    QLabel * label_length;
};

#endif // MAINWINDOW_H
