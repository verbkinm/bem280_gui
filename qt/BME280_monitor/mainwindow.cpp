#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QHBoxLayout>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    _status(std::make_unique<QLabel>()),
    _settings(std::make_unique<SettingsDialog>()),
    _serial(std::make_unique<QSerialPort>(this)),
    _table(std::make_unique<QTableWidget>(BME280_MAX_ARRAY_SIZE, 11)),
    _dataTransmit(std::make_unique<DataTransmit>(this))
{
    m_ui->setupUi(this);

    createTable();
    QWidget* w = new QWidget();
    QHBoxLayout* l = new QHBoxLayout();

    l->addWidget(_table.get(), 1);
    l->addWidget(_dataTransmit.get());
    w->setLayout(l);
    this->setCentralWidget(w);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(_status.get());

    initActionsConnections();

    _acquired_data.clear();

    connect(_serial.get(), &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(_serial.get(), &QSerialPort::readyRead, this, &MainWindow::slot_readData);

    connect(_dataTransmit.get(), &DataTransmit::signal_sendData, this, &MainWindow::slot_sendData);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = _settings->settings();
    _serial->setPortName(p.name);
    _serial->setBaudRate(p.baudRate);
    _serial->setDataBits(p.dataBits);
    _serial->setParity(p.parity);
    _serial->setStopBits(p.stopBits);
    _serial->setFlowControl(p.flowControl);
    if (_serial->open(QIODevice::ReadWrite))
    {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        _dataTransmit->setEnabled(true);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), _serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (_serial->isOpen())
        _serial->close();

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
    _dataTransmit->setEnabled(false);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About BME280"),
                       tr("The <b>BME280</b> reading sensor data via uart."));
}

void MainWindow::slot_readData()
{
    _acquired_data.append(_serial->readAll());

    if(_acquired_data.length() != BME280_MAX_ARRAY_SIZE)
        return;

    uint8_t dataTable[BME280_MAX_ARRAY_SIZE];   //массив для отображения данных в порядке регистров как в документации

    //0xF2 - 0xF5
    for (uint8_t acquireDataIndex = 44, dataTableIndex = 11; acquireDataIndex < 48; acquireDataIndex++, dataTableIndex--)
        dataTable[dataTableIndex] = _acquired_data[acquireDataIndex];

    //0xF7 - 0xFE
    for (uint8_t acquireDataIndex = 48, dataTableIndex = 7; acquireDataIndex < BME280_MAX_ARRAY_SIZE; acquireDataIndex++, dataTableIndex--)
        dataTable[dataTableIndex] = _acquired_data[acquireDataIndex];

    //0xE1 - 0xF0
    for (uint8_t acquireDataIndex = 28, dataTableIndex = 12; acquireDataIndex < 44; acquireDataIndex++, dataTableIndex++)
        dataTable[dataTableIndex] = _acquired_data[acquireDataIndex];

    //0xE0, 0xD0
    dataTable[28] = _acquired_data[27];
    dataTable[29] = _acquired_data[26];

    //0x88 - 0xA1
    for (uint8_t acquireDataIndex = 0, dataTableIndex = 30; acquireDataIndex < 26; acquireDataIndex++, dataTableIndex++)
        dataTable[dataTableIndex] = _acquired_data[acquireDataIndex];

    //заполнение таблицы
    for (uint row = 0; row < BME280_MAX_ARRAY_SIZE; row++)
        fillInLine(row, dataTable[row]);

    _acquired_data.clear();

    _dataTransmit->setConfigRegister(cellValueToByte(11), cellValueToByte(9), cellValueToByte(8));
    _dataTransmit->setTemperature(calculateTemperature());
    _dataTransmit->setPressure(calculatePressure());
    _dataTransmit->setHumitidy(calculateHumidity());
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), _serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, _settings.get(), &SettingsDialog::show);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createTable()
{
    QStringList headerList;
    headerList << "Register Name" << "Address" << "bit7" << "bit6" << "bit5" << "bit4" << "bit3" << "bit2" << "bit1" << "bit0" << "Hex value";

    _table->setShowGrid(true);
    _table->horizontalHeader()->setSectionResizeMode(headerList.length()-1, QHeaderView::Stretch);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setEditTriggers(QTableWidget::NoEditTriggers);
    _table->setHorizontalHeaderLabels(headerList);

    for (int column = 0; column < _table->columnCount(); ++column)
    {
        for (int row = 0; row < _table->rowCount(); row++)
        {
            QTableWidgetItem* item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            _table->setItem(row, column, item);
        }
    }
    _table->resizeColumnsToContents();

    // растягиваем ширину столбцов до предела, если ширина всех столбцов меньше ширины тыблицы
    int widthColumns_TableLessonData = 0;
    for (int column = 0; column < _table->columnCount(); ++column)
        widthColumns_TableLessonData += _table->columnWidth(column);

    if(widthColumns_TableLessonData < _table->width())
    {
        for (int column = 0; column < _table->columnCount(); ++column)
        {
            _table->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Stretch);
            _table->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Stretch);
        }
    }

    // растягиваем высоту строк до предела, если высота всех строк меньше высоты тыблицы
    int heightRows_TableLessonData = 0;
    for (int row = 0; row < _table->rowCount(); ++row)
        heightRows_TableLessonData += _table->rowHeight(row);

    if(heightRows_TableLessonData < _table->height())
    {
        for (int row = 0; row < _table->rowCount(); ++row)
        {
            _table->verticalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
            _table->verticalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
        }
    }

    //черезстрочный фон
    for (int row = 0; row < _table->rowCount(); row += 2)
        for (int column = 0; column < _table->columnCount(); ++column)
        {
            auto item = _table->item(row,column);
            if(item != nullptr)
                item->setBackground(QBrush("#F5F5F5"));
        }

    _table->item(0, 0)->setText("hum_lsb");
    _table->item(1, 0)->setText("hum_msb");
    _table->item(2, 0)->setText("temp_xlsb");
    _table->item(3, 0)->setText("temp_lsb");
    _table->item(4, 0)->setText("temp_msb");
    _table->item(5, 0)->setText("press_xlsb");
    _table->item(6, 0)->setText("press_lsb");
    _table->item(7, 0)->setText("press_msb");
    _table->item(8, 0)->setText("config");
    _table->item(9, 0)->setText("ctrl_meas");
    _table->item(10, 0)->setText("status");
    _table->item(11, 0)->setText("ctrl_hum");

    uint8_t calib = 26;
    for (int row = 12; row < 28; ++row)
        _table->item(row, 0)->setText("calib" + QString::number(calib++));

    _table->item(28, 0)->setText("reset");
    _table->item(29, 0)->setText("id");

    calib = 0;
    for (int row = 30; row < BME280_MAX_ARRAY_SIZE; ++row)
        _table->item(row, 0)->setText("calib" + QString::number(calib++));

    //FE - F7
    uint16_t addr = 0xFE;
    for (int row = 0; row < 8; ++row)
        _table->item(row, 1)->setText("0x" + QString::number(addr--, 16).toUpper());

    //F5 - F2
    addr = 0xF5;
    for (int row = 8; row < 12; ++row)
        _table->item(row, 1)->setText("0x" + QString::number(addr--, 16).toUpper());

    //E1 - F0
    addr = 0xE1;
    for (int row = 12; row < 28; ++row)
        _table->item(row, 1)->setText("0x" + QString::number(addr++, 16).toUpper());

    _table->item(28, 1)->setText("0xE0");
    _table->item(29, 1)->setText("0xD0");

    //88 - A1
    addr = 0x88;
    for (int row = 30; row < BME280_MAX_ARRAY_SIZE; ++row)
        _table->item(row, 1)->setText("0x" + QString::number(addr++, 16).toUpper());
}

void MainWindow::fillInLine(uint rowNumber, uint8_t byte)
{
    uint shift = 0b10000000;
    for (int column = 2; column < 10; column++)
    {
        QTableWidgetItem* item = _table->item(rowNumber, column);
        uint8_t buff = (byte & shift);
        buff = (buff) ? 1 : 0;
        shift >>= 1;
        item->setText(QString::number(buff));
    }
    _table->item(rowNumber, 10)->setText("0x" + QString::number(byte, 16).toUpper());
}

void MainWindow::showStatusMessage(const QString &message) const
{
    _status->setText(message);
}

uint16_t MainWindow::cellsValuesToBytes(int msb_row, int lsb_row) const
{
    return ((int)cellValueToByte(msb_row) << 8) | cellValueToByte(lsb_row);
}

uint8_t MainWindow::cellValueToByte(int row) const
{
    return _table->item(row, COLUMN_NUMBERS)->text().toUShort(nullptr, 16);
}

double MainWindow::calculateTemperature()
{    
    uint8_t msb = cellValueToByte(4);
    uint8_t lsb = cellValueToByte(3);
    uint8_t xlsb = cellValueToByte(2);

    uint32_t adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);

    uint16_t dig_T1 = cellsValuesToBytes(31, 30);
    int16_t dig_T2 = cellsValuesToBytes(33, 32);
    int16_t dig_T3 = cellsValuesToBytes(35, 34);

    double var1, var2, temperature;
    var1 = (((double)adc_T) / 16384.0 - ((double)dig_T1) / 1024.0);
    var1 = var1 *((double)dig_T2);
    var2 = (((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0);
    var2 = (var2 * var2) * ((double)dig_T3);

    _t_fine = (int32_t)(var1 + var2);
    temperature = (var1 + var2) / 5120.0;

    double temperature_min = -40;
    double temperature_max = 85;

    if (temperature < temperature_min)
        temperature = temperature_min;
    else if (temperature > temperature_max)
        temperature = temperature_max;

    return temperature;
}

double MainWindow::calculatePressure() const
{
    uint8_t msb = cellValueToByte(7);
    uint8_t lsb = cellValueToByte(6);
    uint8_t xlsb = cellValueToByte(5);

    uint16_t dig_P1 = cellsValuesToBytes(37, 36);
    int16_t dig_P2 = cellsValuesToBytes(39, 38);
    int16_t dig_P3 = cellsValuesToBytes(41, 40);
    int16_t dig_P4 = cellsValuesToBytes(43, 42);
    int16_t dig_P5 = cellsValuesToBytes(45, 44);
    int16_t dig_P6 = cellsValuesToBytes(47, 46);
    int16_t dig_P7 = cellsValuesToBytes(49, 48);
    int16_t dig_P8 = cellsValuesToBytes(51, 50);
    int16_t dig_P9 = cellsValuesToBytes(53, 52);
    uint32_t adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);

    double var1, var2, pressure;
    var1 = ((double)_t_fine/2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);

    if (var1 == 0.0)
        return 0;

    pressure = 1048576.0 - (double)adc_P;
    pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * pressure * pressure / 2147483648.0;
    var2 = pressure * ((double)dig_P8) / 32768.0;
    pressure = pressure + (var1 + var2 + ((double)dig_P7)) / 16.0;

    double pressure_min = 30000.0;
    double pressure_max = 110000.0;

    if (pressure < pressure_min)
        pressure = pressure_min;
    else if (pressure > pressure_max)
        pressure = pressure_max;

    return pressure;
}

double MainWindow::calculateHumidity() const
{
    uint8_t msb = cellValueToByte(1);
    uint8_t lsb = cellValueToByte(0);

    uint8_t dig_H1 = cellValueToByte(55);
    int16_t dig_H2 = cellsValuesToBytes(13, 12);
    uint8_t dig_H3 = cellValueToByte(14);

    uint8_t dig_H4_msb = cellValueToByte(16) & 0x0F;
    uint8_t dig_H4_lsb = cellValueToByte(15);
    int16_t dig_H4 = dig_H4_lsb;
    dig_H4 = dig_H4 << 4;
    dig_H4 = dig_H4 | dig_H4_msb;

    uint8_t dig_H5_msb = cellValueToByte(17);
    uint8_t dig_H5_lsb = cellValueToByte(16) >> 4;
    int16_t dig_H5 = dig_H5_msb;
    dig_H5 <<= 4;
    dig_H5 = dig_H5 | dig_H5_lsb;

    int8_t dig_H6 = cellValueToByte(18);

    uint32_t adc_H = (msb << 8) | lsb;

    double var_H = (((double)_t_fine) - 76800.0);
    var_H = (adc_H - (((double)dig_H4) * 64.0 + ((double)dig_H5) / 16384.0 * var_H)) *
            (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) / 67108864.0 * var_H *
                                           (1.0 + ((double)dig_H3) / 67108864.0 * var_H)));
    var_H = var_H * (1.0 - ((double)dig_H1) * var_H / 524288.0);
    if (var_H > 100.0)
        var_H = 100.0;
    else if (var_H < 0.0)
        var_H = 0.0;

    return var_H;
}

void MainWindow::slot_sendData(QByteArray arr)
{
    if(!_serial->isOpen())
        return;

    _serial->write(arr);
}
