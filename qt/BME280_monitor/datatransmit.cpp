#include "datatransmit.h"
#include "ui_datatransmit.h"

DataTransmit::DataTransmit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataTransmit)
{
    ui->setupUi(this);

    fillOversampling(ui->oversampling_temperature);
    fillOversampling(ui->oversampling_pressure);
    fillOversampling(ui->oversampling_humidity);

    connect(ui->readData, &QPushButton::clicked, this, &DataTransmit::slot_sendData);
    connect(ui->writeData, &QPushButton::clicked, this, &DataTransmit::slot_sendData);
    connect(ui->reset, &QPushButton::clicked, this, &DataTransmit::slot_sendData);

    connect(&_autoReadTimer, &QTimer::timeout, this, &DataTransmit::slot_autoRead);
}

DataTransmit::~DataTransmit()
{
    delete ui;
}

void DataTransmit::setTemperature(double temperature)
{
    ui->indicator_temperature->setText(ui->oversampling_temperature->currentIndex() > 0 ? QString::number(temperature) + " C°" : "Skipped");
}

void DataTransmit::setPressure(double pressure)
{
    pressure = pressure / 133.3224;
    ui->indicator_pressure->setText(ui->oversampling_pressure->currentIndex() > 0 ? QString::number(pressure) + " mm Hg" : "Skippes");
}

void DataTransmit::setHumitidy(double humidity)
{
    ui->indicator_humidity->setText(ui->oversampling_humidity->currentIndex() > 0 ? QString::number(humidity) + " %" : "Skipped");
}

void DataTransmit::setConfigRegister(uint8_t F2, uint8_t F4, uint8_t F5)
{
    ui->oversampling_humidity->setCurrentIndex(F2);
    ui->oversampling_temperature->setCurrentIndex(F4 >> 5);
    ui->oversampling_pressure->setCurrentIndex( (F4 & 0x1F) >> 2);
    uint8_t mode = F4 & 0x03;
    ui->mode->setCurrentIndex( mode == 3 ? 2 : mode);
    ui->config_standby->setCurrentIndex(F5 >> 5);
    ui->config_filter->setCurrentIndex( (F5 & 0x1F) >> 2);
}

void DataTransmit::fillOversampling(QComboBox *combobox)
{
    combobox->addItems({"Skipped", "Oversampling x1", "Oversampling x2", "Oversampling x4", "Oversampling x8", "Oversampling x16"});
}

QByteArray DataTransmit::calculateBytesOfTransfer() const
{
    uint8_t F2 = ui->oversampling_humidity->currentIndex();
    uint8_t F4 =  ui->oversampling_temperature->currentIndex() << 5;
    F4 |= ui->oversampling_pressure->currentIndex() << 2;

    if(ui->mode->currentIndex() == 2)
        F4 |= 3;
    else
        F4 |= ui->mode->currentIndex();

    uint8_t F5 = ui->config_standby->currentIndex() << 5;
    F5 |= ui->config_filter->currentIndex() << 2;

    QByteArray arr;
    arr.append(0xF2);
    arr.append(F2);
    arr.append(0xF5);
    arr.append(F5);
    arr.append(0xF4);
    arr.append(F4);

    return arr;
}

void DataTransmit::setEnableGroupBox(bool state)
{
    ui->groupBox_oversamplings->setEnabled(state);
    ui->groupBox_settings->setEnabled(state);
    ui->readData->setEnabled(state);
    ui->writeData->setEnabled(state);
    ui->reset->setEnabled(state);
}

void DataTransmit::slot_sendData()
{
    QByteArray arr;

    if(sender()->objectName() == "writeData")
        arr = calculateBytesOfTransfer();
    else if(sender()->objectName() == "readData")
    {
        arr.append((int)0);
        arr.append((int)0);
    }
    else if(sender()->objectName() == "reset")
    {
        arr.append(0xE0);
        arr.append(0xB6);
    }

    emit signal_sendData(arr);
}

void DataTransmit::slot_autoRead()
{
    QByteArray arr;
    arr.append((int)0);
    arr.append((int)0);
    emit signal_sendData(arr);
}

void DataTransmit::on_autoread_stateChanged(int state)
{
    if(state == Qt::Checked)
    {
        _autoReadTimer.start(500);
        setEnableGroupBox(false);
    }
    else
    {
        _autoReadTimer.stop();
        setEnableGroupBox(true);
    }
}
