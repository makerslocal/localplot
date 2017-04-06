/**
 * DialogSettings - Configure localplot
 * Christopher Bero <bigbero@gmail.com>
 */
#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    QSettings settings;

    ui->setupUi(this);

    // Set up the drawing pens
    upPen.setStyle(Qt::DotLine);
    do_updatePens();

    // Restore saved window geometry
    if (settings.contains("dialogsettings/geometry"))
    {
        restoreGeometry(settings.value("dialogsettings/geometry").toByteArray());
    }

    // Initialize interface
    ui->comboBox_baud->clear();
    ui->comboBox_baud->insertItem(0, "2400", 2400);
    ui->comboBox_baud->insertItem(1, "4800", 4800);
    ui->comboBox_baud->insertItem(2, "9600", 9600);
    ui->comboBox_baud->insertItem(3, "19200", 19200);
    ui->comboBox_baud->insertItem(4, "38400", 38400);
    ui->comboBox_baud->insertItem(5, "57600", 57600);
    ui->comboBox_baud->insertItem(6, "115200", 115200);
    ui->comboBox_baud->setCurrentIndex(2);

    ui->comboBox_bytesize->clear();
    ui->comboBox_bytesize->insertItem(0, "8", 8);
    ui->comboBox_bytesize->insertItem(1, "7", 7);
    ui->comboBox_bytesize->insertItem(2, "6", 6);
    ui->comboBox_bytesize->insertItem(3, "5", 5);
    ui->comboBox_bytesize->setCurrentIndex(0);

    ui->comboBox_parity->clear();
    ui->comboBox_parity->insertItem(0, "None", "none");
    ui->comboBox_parity->insertItem(1, "Odd", "odd");
    ui->comboBox_parity->insertItem(2, "Even", "even");
    ui->comboBox_parity->insertItem(3, "Mark", "mark");
    ui->comboBox_parity->insertItem(4, "Space", "space");
    ui->comboBox_parity->setCurrentIndex(0);

    ui->comboBox_stopbits->clear();
    ui->comboBox_stopbits->insertItem(0, "1", 1);
    ui->comboBox_stopbits->insertItem(1, "1.5", 3);
    ui->comboBox_stopbits->insertItem(2, "2", 2);
    ui->comboBox_stopbits->setCurrentIndex(0);

    ui->comboBox_deviceWidthType->clear();
    for (int i = 0; i < deviceWidth_t::SIZE_OF_ENUM; ++i)
    {
        ui->comboBox_deviceWidthType->insertItem(i, deviceWidth_names[i], i);
    }
    ui->comboBox_deviceWidthType->setCurrentIndex(0);

    do_refreshSerialList();

    // Load saved settings
    ui->spinBox_downPen_size->setValue(settings.value("pen/down/size", SETDEF_PEN_DOWN_SIZE).toInt());
    ui->spinBox_downPen_red->setValue(settings.value("pen/down/red", SETDEF_PEN_DOWN_RED).toInt());
    ui->spinBox_downPen_green->setValue(settings.value("pen/down/green", SETDEF_PEN_DOWN_GREEN).toInt());
    ui->spinBox_downPen_blue->setValue(settings.value("pen/down/blue", SETDEF_PEN_DOWN_BLUE).toInt());
    ui->spinBox_upPen_size->setValue(settings.value("pen/up/size", SETDEF_PEN_UP_SIZE).toInt());
    ui->spinBox_upPen_red->setValue(settings.value("pen/up/red", SETDEF_PEN_UP_RED).toInt());
    ui->spinBox_upPen_green->setValue(settings.value("pen/up/green", SETDEF_PEN_UP_GREEN).toInt());
    ui->spinBox_upPen_blue->setValue(settings.value("pen/up/blue", SETDEF_PEN_DOWN_BLUE).toInt());
    ui->checkBox_deviceIncrementalOutput->setChecked(settings.value("device/incremental", SETDEF_DEVICE_INCREMENTAL).toBool());
    ui->spinBox_deviceCutSpeed->setValue(settings.value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt());
    ui->spinBox_deviceTravelSpeed->setValue(settings.value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt());
    ui->spinBox_deviceWidth->setValue(settings.value("device/width", SETDEF_DEVICE_WIDTH).toInt());
    ui->comboBox_deviceWidthType->setCurrentIndex(settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt());
    ui->tabWidget->setCurrentIndex(settings.value("dialogsettings/index", SETDEF_DIALLOGSETTINGS_INDEX).toInt());

    oldCutoutBoxes = settings.value("device/cutoutboxes", SETDEF_DEVICE_CUTOUTBOXES).toBool();
    ui->checkBox_enableCutoutBoxes->setChecked(oldCutoutBoxes);

    ui->checkBox_viewGridEnabled->setChecked(settings.value("mainwindow/grid", SETDEF_MAINWINDOW_GRID).toBool());
    ui->spinBox_viewGridSize->setValue(settings.value("mainwindow/grid/size", SETDEF_MAINWINDOW_GRID_SIZE).toInt());
    ui->doubleSpinBox_cutoutBoxesPadding->setValue(settings.value("device/cutoutboxes/padding", SETDEF_DEVICE_CUTOUTBOXES_PADDING).toDouble());

    ui->checkBox_hookFinishedEnabled->setChecked(settings.value("hook/finished", SETDEF_HOOK_FINISHED).toBool());
    ui->lineEdit_hookFinishedPath->setText(settings.value("hook/finished/path", SETDEF_HOOK_FINISHED_PATH).toString());

    // load import script paths
    ui->lineEdit_importSvgPath->setText(settings.value("import/svg/path", SETDEF_IMPORT_SVG_PATH).toString());
    ui->lineEdit_importDxfPath->setText(settings.value("import/dxf/path", SETDEF_IMPORT_DXF_PATH).toString());

    if (settings.value("serial/xonxoff", SETDEF_SERIAL_XONOFF).toBool())
    {
        ui->radioButton_XonXoff->setChecked(true);
        ui->radioButton_RtsCts->setChecked(false);
        ui->radioButton_flowControlNone->setChecked(false);
    }
    else if (settings.value("serial/rtscts", SETDEF_SERIAL_RTSCTS).toBool())
    {
        ui->radioButton_XonXoff->setChecked(false);
        ui->radioButton_RtsCts->setChecked(true);
        ui->radioButton_flowControlNone->setChecked(false);
    }
    else
    {
        ui->radioButton_XonXoff->setChecked(false);
        ui->radioButton_RtsCts->setChecked(false);
        ui->radioButton_flowControlNone->setChecked(true);
    }
    for (int index = 0; index < ui->comboBox_baud->count(); index++)
    {
        int value = ui->comboBox_baud->itemData(index).toInt();
        if (value == settings.value("serial/baud", SETDEF_SERIAL_BAUD).toInt())
        {
            ui->comboBox_baud->setCurrentIndex(index);
            break;
        }
    }
    for (int index = 0; index < ui->comboBox_bytesize->count(); index++)
    {
        int value = ui->comboBox_bytesize->itemData(index).toInt();
        if (value == settings.value("serial/bytesize", SETDEF_SERIAL_BYTESIZE).toInt())
        {
            ui->comboBox_bytesize->setCurrentIndex(index);
            break;
        }
    }
    for (int index = 0; index < ui->comboBox_stopbits->count(); index++)
    {
        int value = ui->comboBox_stopbits->itemData(index).toInt();
        if (value == settings.value("serial/stopbits", SETDEF_SERIAL_STOPBITS).toInt())
        {
            ui->comboBox_stopbits->setCurrentIndex(index);
            break;
        }
    }
    for (int index = 0; index < ui->comboBox_parity->count(); index++)
    {
        QString value = ui->comboBox_parity->itemData(index).toString();
        if (value == settings.value("serial/parity", SETDEF_SERIAL_PARITY).toString())
        {
            ui->comboBox_parity->setCurrentIndex(index);
            break;
        }
    }
    for (int index = 0; index < ui->comboBox_serialPort->count(); index++)
    {
        QString value = ui->comboBox_serialPort->itemData(index).toString();
        if (value == settings.value("serial/port", SETDEF_SERIAL_STOPBITS).toString())
        {
            ui->comboBox_serialPort->setCurrentIndex(index);
            break;
        }
    }

    connect(ui->pushButton_serialRefresh, SIGNAL(clicked()), this, SLOT(do_refreshSerialList()));
    connect(ui->comboBox_serialPort, SIGNAL(activated(int)), this, SLOT(do_writeLineEditSerialPort()));

    // Update UI on settings change
    connect(ui->spinBox_downPen_size, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_downPen_red, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_downPen_green, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_downPen_blue, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_upPen_size, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_upPen_red, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_upPen_green, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));
    connect(ui->spinBox_upPen_blue, SIGNAL(valueChanged(int)), this, SLOT(do_drawDemoView()));

    ui->graphicsView_penDownDemo->setScene(&penDownDemoScene);
    ui->graphicsView_penUpDemo->setScene(&penUpDemoScene);

    connect(ui->pushButton_settingsClear, SIGNAL(clicked(bool)), this, SLOT(do_settingsClear()));
    connect(ui->pushButton_settingsPrint, SIGNAL(clicked(bool)), this, SLOT(do_settingsPrint()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(do_saveAndClose())); // Save settings
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close())); // Discard settings

    connect(ui->comboBox_deviceWidthType, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_unitsChanged()));

    handle_unitsChanged();

    do_drawDemoView();
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("dialogsettings/geometry", saveGeometry());
    QDialog::closeEvent(event);
}

void DialogSettings::handle_unitsChanged()
{
    int currentIndex;
    currentIndex = ui->comboBox_deviceWidthType->currentIndex();
    QString labelText;

    if (currentIndex == deviceWidth_t::INCH)
    {
        labelText = "Inches";
    }
    else if (currentIndex == deviceWidth_t::CM)
    {
        labelText = "Centimeters";
    }
    else
    {
        labelText = "[Error]";
    }
    ui->label_deviceWidth->setText(labelText);
    ui->label_cutoutBoxPadding->setText(labelText);
    ui->label_viewGridUnits->setText(labelText);
}

void DialogSettings::do_settingsClear()
{
    QSettings settings;
    qDebug() << "Clearing all settings.";
    do_settingsPrint();
    settings.clear();
    qDebug() << "... Done.";
    this->close();
}

void DialogSettings::do_settingsPrint()
{
    QSettings settings;
    qDebug() << "Printing all settings: ";
    QStringList allKeys = settings.allKeys();
    for (int index = 0; index < allKeys.size(); index++)
    {
        qDebug() << allKeys.at(index) << ":\t" << settings.value(allKeys.at(index));
    }
}

void DialogSettings::do_saveAndClose()
{
    QSettings settings;
    settings.beginGroup("pen");
    {
        settings.setValue("down/size", ui->spinBox_downPen_size->value());
        settings.setValue("down/red", ui->spinBox_downPen_red->value());
        settings.setValue("down/green", ui->spinBox_downPen_green->value());
        settings.setValue("down/blue", ui->spinBox_downPen_blue->value());
        settings.setValue("up/size", ui->spinBox_upPen_size->value());
        settings.setValue("up/red", ui->spinBox_upPen_red->value());
        settings.setValue("up/green", ui->spinBox_upPen_green->value());
        settings.setValue("up/blue", ui->spinBox_upPen_blue->value());
    }
    settings.endGroup();

    settings.beginGroup("serial");
    {
        if (ui->comboBox_serialPort->currentData().toString() == ui->lineEdit_serialPort->text())
        {
            settings.setValue("port", ui->comboBox_serialPort->currentData());
        }
        else
        {
            settings.setValue("port", ui->lineEdit_serialPort->text());
        }
        settings.setValue("parity", ui->comboBox_parity->currentData());
        settings.setValue("baud", ui->comboBox_baud->currentData());
        settings.setValue("bytesize", ui->comboBox_bytesize->currentData());
        settings.setValue("stopbits", ui->comboBox_stopbits->currentData());
        settings.setValue("xonxoff", ui->radioButton_XonXoff->isChecked());
        settings.setValue("rtscts", ui->radioButton_RtsCts->isChecked());
    }
    settings.endGroup();

    settings.beginGroup("device");
    {
        settings.setValue("incremental", ui->checkBox_deviceIncrementalOutput->isChecked());
        settings.setValue("speed/cut", ui->spinBox_deviceCutSpeed->value());
        settings.setValue("speed/travel", ui->spinBox_deviceTravelSpeed->value());
        settings.setValue("width", ui->spinBox_deviceWidth->value());
        settings.setValue("width/type", ui->comboBox_deviceWidthType->currentData().toInt());
        settings.setValue("cutoutboxes", ui->checkBox_enableCutoutBoxes->isChecked());
        settings.setValue("cutoutboxes/padding", ui->doubleSpinBox_cutoutBoxesPadding->value());
        // Signal cutoutbox toggle if necessary
        bool newCutoutBoxes = settings.value("cutoutboxes", SETDEF_DEVICE_CUTOUTBOXES).toBool();
        if (oldCutoutBoxes != newCutoutBoxes)
        {
            emit toggleCutoutBoxes(newCutoutBoxes);
        }
    }
    settings.endGroup();

    settings.beginGroup("mainwindow");
    {
        settings.setValue("grid", ui->checkBox_viewGridEnabled->isChecked());
        settings.setValue("grid/size", ui->spinBox_viewGridSize->value());
    }
    settings.endGroup();

    settings.beginGroup("import");
    {
        settings.setValue("svg/path", ui->lineEdit_importSvgPath->text());
        settings.setValue("dxf/path", ui->lineEdit_importDxfPath->text());
    }
    settings.endGroup();

    settings.setValue("hook/finished", ui->checkBox_hookFinishedEnabled->isChecked());
    settings.setValue("hook/finished/path", ui->lineEdit_hookFinishedPath->text());

    settings.setValue("dialogsettings/index", ui->tabWidget->currentIndex());

    close();
}

void DialogSettings::do_refreshSerialList()
{
    QSettings settings;
    QList<QSerialPortInfo> _ports = serialPorts.availablePorts();
    ui->comboBox_serialPort->clear();

    ui->lineEdit_serialPort->setText(settings.value("serial/port", SETDEF_SERIAL_PORT).toString());

    for (int i = 0; i < _ports.count(); ++i)
    {
        ui->comboBox_serialPort->insertItem(i, _ports.at(i).portName(), _ports.at(i).systemLocation());
        if (_ports.at(i).systemLocation() == settings.value("serial/port", SETDEF_SERIAL_PORT).toString())
        {
            ui->comboBox_baud->setCurrentIndex(i);
        }
    }
}

void DialogSettings::do_writeLineEditSerialPort()
{
    ui->lineEdit_serialPort->setText(ui->comboBox_serialPort->currentData().toString());
}

void DialogSettings::do_updatePens()
{
    // Variables
    int rgbColor[3];
    int penSize;
    QColor penColor;

    // Set downPen
    penSize = ui->spinBox_downPen_size->value();
    rgbColor[0] = ui->spinBox_downPen_red->value();
    rgbColor[1] = ui->spinBox_downPen_green->value();
    rgbColor[2] = ui->spinBox_downPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    downPen.setColor(penColor);
    downPen.setWidth(penSize);

    // Set upPen
    penSize = ui->spinBox_upPen_size->value();
    rgbColor[0] = ui->spinBox_upPen_red->value();
    rgbColor[1] = ui->spinBox_upPen_green->value();
    rgbColor[2] = ui->spinBox_upPen_blue->value();
    penColor = QColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    upPen.setColor(penColor);
    upPen.setWidth(penSize);
}

void DialogSettings::do_drawDemoView()
{
    do_updatePens();
    penDownDemoScene.clear();
    penUpDemoScene.clear();
    penDownDemoScene.addLine(0, 0, 75, 0, downPen);
    penUpDemoScene.addLine(0, 0, 75, 0, upPen);
    ui->graphicsView_penDownDemo->show();
    ui->graphicsView_penUpDemo->show();
}


























