#ifndef ARDUINOCOMM_H
#define ARDUINOCOMM_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

enum ArduinoDisplay
{
    A_STATUS,
    A_ENC1,
    A_ENC2,
    A_BUTTONS
};
enum ArduinoStatusParm
{
    P_CONFIG,
    P_BANKNUM,
    P_BANKLABEL
};


class ArduinoComm : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoComm(QString portId, QObject *parent = nullptr);
    ~ArduinoComm();


    void updateButtonLabel(int buttonNum, QString label);
    void updateEncoderLabel(int encDisplay, int encNum, QString label);
    void updateStatusConfig(QString config);
    void updateStatusBank(int banknum, QString banklabel);

    void connectToArduino();
    bool isArduinoHere();

signals:
    void connectionEstablished();

private:
    QSerialPort* serPort;
    QString serPortID = ""; //"HIDPCHIDKD";  //FIXXXME make this configurable in settings

    bool arduinoConnected = false;

    void setSerialPortname(QString portID);
    void openSerialPort();
    void closeSerialPort();
    void writeSerial(QByteArray data);
    void readSerialData();
    void processReadData(QString data);

    void sendCommand(QString command);


public slots:
    void handleError(QSerialPort::SerialPortError error);
    void handleConnectionEstablished();

};

#endif // ARDUINOCOMM_H
