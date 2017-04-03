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

    connect(ui->pushButton_close, SIGNAL(clicked(bool)), this, SIGNAL(please_close()));

    QPixmap _image(":/image/images/logo.png");
    logoScene = new QGraphicsScene;
    ui->graphicsView->setScene(logoScene);
    logoScene->addPixmap(_image);
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
