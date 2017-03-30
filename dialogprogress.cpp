#include "dialogprogress.h"
#include "ui_dialogprogress.h"

DialogProgress::DialogProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProgress)
{
    QSettings settings;
    ui->setupUi(this);

    ui->checkBox_hookFinishedEnabled->setChecked(settings.value("hook/finished", SETDEF_HOOK_FINISHED).toBool());

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(handle_abortBtn(QAbstractButton*)));
    connect(ui->checkBox_hookFinishedEnabled, SIGNAL(toggled(bool)), this, SLOT(handle_postHookCheckboxChanged(bool)));
}

DialogProgress::~DialogProgress()
{
    delete ui;
}

void DialogProgress::enableHookCheckbox()
{
    ui->checkBox_hookFinishedEnabled->setEnabled(true);
}

void DialogProgress::handle_postHookCheckboxChanged(bool checked)
{
    QSettings settings;
    settings.setValue("hook/finished", checked);
}

void DialogProgress::handle_updateProgress(int percent)
{
    ui->progressBar->setValue(percent);
}

void DialogProgress::handle_abortBtn(QAbstractButton * btn)
{
    if (btn->text() == "Abort")
    {
        emit do_cancel();
    }
}
