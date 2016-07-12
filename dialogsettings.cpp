#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    // Instantiate settings object
    init_localplot_settings();
    settings = new QSettings();

    // Initialize interface
    ui->comboBox_baud->insertItems(0, QStringList() << "2400" << "4800"
                                   << "9600" << "19200" << "38400"
                                   << "57600" << "115200");
    ui->comboBox_baud->setCurrentIndex(2);
    ui->comboBox_bytesize->insertItems(0, QStringList() << "8" << "7"
                                       << "6" << "5");
    ui->comboBox_parity->insertItems(0, QStringList() << "None" << "Odd"
                                     << "Even" << "Mark" << "Space");
    ui->comboBox_stopbits->insertItems(0, QStringList() << "1" << "1.5" << "2");
    do_refreshSerialList();

    // Load saved settings
    ui->spinBox_downPen_size->setValue(settings->value("pen/down/size", 2).toInt());
    ui->spinBox_downPen_red->setValue(settings->value("pen/down/red", 100).toInt());
    ui->spinBox_downPen_green->setValue(settings->value("pen/down/green", 150).toInt());
    ui->spinBox_downPen_blue->setValue(settings->value("pen/down/blue", 200).toInt());
    ui->spinBox_upPen_size->setValue(settings->value("pen/up/size", 1).toInt());
    ui->spinBox_upPen_red->setValue(settings->value("pen/up/red", 250).toInt());
    ui->spinBox_upPen_green->setValue(settings->value("pen/up/green", 150).toInt());
    ui->spinBox_upPen_blue->setValue(settings->value("pen/up/blue", 150).toInt());

    for (int i = 0; i < ui->comboBox_serialPort->count(); i++)
    {
        if (settings->value("serial/port") == ui->comboBox_serialPort->itemData(i))
        {
            ui->comboBox_serialPort->setCurrentIndex(i);
            break;
        }
    }

    connect(ui->pushButton_serialRefresh, SIGNAL(clicked()), this, SLOT(do_refreshSerialList()));

    // Update settings on UI change
    connect(ui->spinBox_downPen_size, SIGNAL(valueChanged(int)), this, SLOT(update_penDown()));
    connect(ui->spinBox_downPen_red, SIGNAL(valueChanged(int)), this, SLOT(update_penDown()));
    connect(ui->spinBox_downPen_green, SIGNAL(valueChanged(int)), this, SLOT(update_penDown()));
    connect(ui->spinBox_downPen_blue, SIGNAL(valueChanged(int)), this, SLOT(update_penDown()));
    connect(ui->spinBox_upPen_size, SIGNAL(valueChanged(int)), this, SLOT(update_penUp()));
    connect(ui->spinBox_upPen_red, SIGNAL(valueChanged(int)), this, SLOT(update_penUp()));
    connect(ui->spinBox_upPen_green, SIGNAL(valueChanged(int)), this, SLOT(update_penUp()));
    connect(ui->spinBox_upPen_blue, SIGNAL(valueChanged(int)), this, SLOT(update_penUp()));

    connect(ui->comboBox_serialPort, SIGNAL(currentIndexChanged(int)), this, SLOT(update_serialDevice()));
    connect(ui->checkBox_cutterSpeed, SIGNAL(toggled(bool)), this, SLOT(update_cutterSpeed(bool)));

    connect(ui->checkBox_cutterSpeed, SIGNAL(stateChanged(int)), this, SLOT(update_cutter()));
    connect(ui->spinBox_cutterSpeed, SIGNAL(valueChanged(int)), this, SLOT(update_cutter()));

    ui->graphicsView_penDownDemo->setScene(&penDownDemoScene);
    ui->graphicsView_penUpDemo->setScene(&penUpDemoScene);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(close()));
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::update_cutter()
{
    settings->beginGroup("cutter");
    settings->setValue("incremental", ui->checkBox_cutterSpeed->isChecked());
    settings->setValue("speed", ui->spinBox_cutterSpeed->value());
    settings->endGroup();
    ui->spinBox_cutterSpeed->setEnabled(ui->checkBox_cutterSpeed->isChecked());
}

void DialogSettings::update_serialDevice()
{
    settings->beginGroup("serial");
    settings->setValue("port/name", ui->comboBox_serialPort->currentText());
    settings->setValue("port/index", ui->comboBox_serialPort->currentIndex());
    settings->setValue("parity", ui->comboBox_parity->currentData());
    settings->setValue("baud", ui->comboBox_baud->currentData());
    settings->setValue("bytesize", ui->comboBox_bytesize->currentData());
    settings->setValue("stopbits", ui->comboBox_stopbits->currentData());
    settings->setValue("xonxoff", ui->radioButton_XonXoff->isChecked());
    settings->setValue("rtscts", ui->radioButton_RtsCts->isChecked());
    settings->setValue("dsrdtr", ui->radioButton_DsrDtr->isChecked());
    settings->endGroup();
}

void DialogSettings::do_refreshSerialList()
{
    QList<QSerialPortInfo> _ports = serialPorts.availablePorts();
    int _index = _ports.count();
    ui->comboBox_serialPort->clear();
    for (int i = 0; i < _index; ++i)
    {
        ui->comboBox_serialPort->insertItem(i, _ports.at(i).portName());
    }
}

void DialogSettings::do_drawDemoView()
{
    do_updatePens();
    penDownDemoScene.clear();
    penUpDemoScene.clear();
    penDownDemoScene.addLine(0, 0, 28, 0, downPen);
    penUpDemoScene.addLine(0, 0, 28, 0, upPen);
    ui->graphicsView_penDownDemo->show();
    ui->graphicsView_penUpDemo->show();
}

void DialogSettings::do_updatePens()
{
    // Variables
    int rgbColor[3];
    int penSize;
    QColor penColor;

    // Set downPen
    penSize = settings->value("pen/down/size", 2).toInt(); //ui->spinBox_downPen_size->value();
    rgbColor[0] = settings->value("pen/down/red", 0).toInt(); //ui->spinBox_downPen_red->value();
    rgbColor[1] = settings->value("pen/down/green", 0).toInt(); //ui->spinBox_downPen_green->value();
    rgbColor[2] = settings->value("pen/down/blue", 0).toInt(); //ui->spinBox_downPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize);

    // Set upPen
    penSize = settings->value("pen/up/size", 1).toInt(); //ui->spinBox_upPen_size->value();
    rgbColor[0] = settings->value("pen/up/red", 1).toInt(); //ui->spinBox_upPen_red->value();
    rgbColor[1] = settings->value("pen/up/green", 1).toInt(); //ui->spinBox_upPen_green->value();
    rgbColor[2] = settings->value("pen/up/blue", 1).toInt(); //ui->spinBox_upPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    upPen.setColor(penColor);
    upPen.setWidth(penSize);
}
