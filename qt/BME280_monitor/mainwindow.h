#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTableWidget>
#include <memory>
#include "datatransmit.h"

#define COLUMN_NUMBERS 10
#define BME280_MAX_ARRAY_SIZE 56

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();
    void handleError(QSerialPort::SerialPortError error);

    void slot_readData();
    void slot_sendData(QByteArray arr);

private:
    void initActionsConnections();
    void createTable();

    void fillInLine(uint rowNumber, uint8_t byte);

    void showStatusMessage(const QString &message) const;
    void setStatus(uint8_t byte);

    uint8_t cellValueToByte(int row) const;
    uint16_t cellsValuesToBytes(int msb_row, int lsb_row) const;

    double calculateTemperature();
    double calculatePressure() const;
    double calculateHumidity() const;

    Ui::MainWindow *m_ui = nullptr;
    std::unique_ptr<QLabel> _status;
    std::unique_ptr<SettingsDialog> _settings;
    std::unique_ptr<QSerialPort> _serial;
    std::unique_ptr<QTableWidget> _table;
    std::unique_ptr<DataTransmit> _dataTransmit;
    QByteArray _acquired_data;

    int32_t _t_fine;
};

#endif // MAINWINDOW_H
