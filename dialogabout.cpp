/**
 * DialogAbout - UI for localplot information and contributors
 * Christopher Bero <bigbero@gmail.com>
 */
#include "dialogabout.h"
#include "ui_dialogabout.h"

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    qDebug() << "I'm alive!";
    connect(ui->pushButton_close, SIGNAL(clicked(bool)), this, SIGNAL(please_close()));
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
