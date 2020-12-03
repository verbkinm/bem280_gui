#ifndef DATATRANSMIT_H
#define DATATRANSMIT_H

#include <QWidget>
#include <QComboBox>
#include <QTimer>

namespace Ui {
class DataTransmit;
}

class DataTransmit : public QWidget
{
    Q_OBJECT

public:
    explicit DataTransmit(QWidget *parent = nullptr);
    ~DataTransmit();

    void setTemperature(double temperature);
    void setPressure(double pressure);
    void setHumitidy(double humidity);

    void setConfigRegister(uint8_t F2, uint8_t F4, uint8_t F5);

private:
    Ui::DataTransmit *ui;

    void fillOversampling(QComboBox* combobox);
    QByteArray calculateBytesOfTransfer() const;
    void setEnableGroupBox(bool state);

    QTimer _autoReadTimer;

private slots:
    void slot_sendData();
    void slot_autoRead();

    void on_autoread_stateChanged(int state);

signals:
    void signal_sendData(QByteArray);
};

#endif // DATATRANSMIT_H
