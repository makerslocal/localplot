#ifndef DIALOGPROGRESS_H
#define DIALOGPROGRESS_H

#include <QDialog>
#include <QtCore>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QCheckBox>

#include "settings.h"

namespace Ui {
class DialogProgress;
}

class DialogProgress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProgress(QWidget *parent = 0);
    ~DialogProgress();
    void enableHookCheckbox();

signals:
    void do_cancel();

public slots:
    void handle_updateProgress(int percent);

private slots:
    void handle_postHookCheckboxChanged(bool checked);
    void handle_abortBtn(QAbstractButton *btn);

private:
    Ui::DialogProgress *ui;
};

#endif // DIALOGPROGRESS_H
