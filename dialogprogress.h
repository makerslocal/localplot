#ifndef DIALOGPROGRESS_H
#define DIALOGPROGRESS_H

#include <QDialog>
#include <QtCore>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QAbstractButton>

namespace Ui {
class DialogProgress;
}

class DialogProgress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProgress(QWidget *parent = 0);
    ~DialogProgress();

signals:
    void do_cancel();

public slots:
    void handle_updateProgress(int percent);

private:
    Ui::DialogProgress *ui;
    void handle_abortBtn(QAbstractButton *btn);
};

#endif // DIALOGPROGRESS_H
