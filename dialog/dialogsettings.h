/**
 * DialogSettings - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QtCore>
#include <QSerialPortInfo>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

#include "settings.h"

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = 0);
    ~DialogSettings();

signals:
    void toggleCutoutBoxes(bool);

public slots:
    void do_refreshSerialList();
    void do_drawDemoView();
    void do_updatePens();
    void do_saveAndClose();
    void do_settingsClear();
    void do_settingsPrint();
    void do_writeLineEditSerialPort();

    void handle_unitsChanged();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::DialogSettings *ui;
    QSettings * settings;
    QSerialPortInfo serialPorts;
    QGraphicsScene penDownDemoScene;
    QGraphicsScene penUpDemoScene;
    QPen downPen;
    QPen upPen;
    bool oldCutoutBoxes;
};

#endif // DIALOGSETTINGS_H
