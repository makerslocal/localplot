#include "dialogprogress.h"
#include "ui_dialogprogress.h"

DialogProgress::DialogProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProgress)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(handle_abortBtn(QAbstractButton*)));
}

DialogProgress::~DialogProgress()
{
    delete ui;
}

void DialogProgress::handle_updateProgress(int percent)
{
    ui->progressBar->setValue(percent);
}

void DialogProgress::handle_abortBtn(QAbstractButton * btn)
{
    qDebug() << btn->text();
    if (btn->text() == "Abort")
    {
        qDebug() << "btn";
    }
    emit do_cancel();
}
