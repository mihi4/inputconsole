#include "arduinocomm.h"
#include <QtDebug>
#include <QMessageBox>

ArduinoComm::ArduinoComm(QString portId, QObject *parent) : QObject(parent),
  serPort(new QSerialPort(this))
{
    qDebug() << "parameter portId" << portId;
    this->serPortID = portId;
    //openSerialPort("/dev/ttyACM0");
    openSerialPort();
    connect(serPort, &QSerialPort::errorOccurred, this, &ArduinoComm::handleError);
    connect(serPort, &QSerialPort::readyRead, this, &ArduinoComm::readSerialData);
    connect(this, &ArduinoComm::connectionEstablished, this, &ArduinoComm::handleConnectionEstablished);
}

ArduinoComm::~ArduinoComm()
{
    disconnect(serPort, &QSerialPort::errorOccurred, this, &ArduinoComm::handleError);
    disconnect(serPort, &QSerialPort::readyRead, this, &ArduinoComm::readSerialData);
    closeSerialPort();
}

void ArduinoComm::setSerialPortname(QString portId)
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    qDebug() << "Total number of ports available: " << serialPortInfos.count();

    const QString blankString = "N/A";
    QString description;
    QString manufacturer;
    QString serialNumber;
    QString portName;

    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
        serialNumber = serialPortInfo.serialNumber();
        portName = serialPortInfo.portName();
        if (serialNumber == portId)
        {
            qDebug() << "found port with ID" << portId;
            serPort->setPortName(portName);
            return;
        }
    }
}

void ArduinoComm::openSerialPort()
{
    qDebug() << "opening Port";
    //serPort->setPortName(portname);
    setSerialPortname(serPortID);
    serPort->setBaudRate(QSerialPort::Baud115200);
    serPort->setDataBits(QSerialPort::Data8);
    serPort->setParity(QSerialPort::NoParity);
    serPort->setStopBits(QSerialPort::OneStop);
    serPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Connected";
    } else {
        qDebug() << "Error Connecting";
    }
}

void ArduinoComm::closeSerialPort()
{
    qDebug() << "closing Port";
    if (serPort->isOpen())
        serPort->close();
}

void ArduinoComm::sendCommand(QString command)
{
    //qDebug() << "in AComm sendCOmmand" << command;
    command.replace("\n", "");
    char newLine = 0x0A;

    QByteArray output = "";
    output.append(command);
    output.append(newLine);

    writeSerial(output);
}

void ArduinoComm::writeSerial(QByteArray data)
{
   // qDebug() << "writeSerial called";
    if (serPort->isOpen() && serPort->isWritable())
    {
    //    qDebug() << "sending command" << data;
        serPort->write(data);
    } else
    {
        qDebug() << "serport is not open";
        arduinoConnected = false;
    }
    //qDebug() << "writeSerial ending";
}

void ArduinoComm::readSerialData()
{
    const QByteArray data = serPort->readAll();
    processReadData(QString(data));
}
void ArduinoComm::processReadData(QString data)
{
    data.replace("\r\n", "");
    //qDebug() << "incoming data" << data;
    if (data == "yes") emit connectionEstablished();
    return;
}
void ArduinoComm::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        qDebug() << "error connecting:" << serPort->errorString();
        closeSerialPort();
    }
}

void ArduinoComm::handleConnectionEstablished()
{
    sendCommand("ok");
    arduinoConnected = true;
}

void ArduinoComm::connectToArduino()
{
    sendCommand("uhere");
}

bool ArduinoComm::isArduinoHere()
{
    return arduinoConnected;
}


/*
 * Commands to Arduino
 */

void ArduinoComm::updateStatusConfig(QString config)
{
    QString command = QString::number(A_STATUS);
    command.append(",");
    command.append(QString::number(P_CONFIG));
    command.append(",");
    command.append(config);

    qDebug() << "updateStatusConfig command:" << command;
    sendCommand(command);
}

void ArduinoComm::updateStatusBank(int banknum, QString banklabel)
{
    QString command = QString::number(A_STATUS);
    command.append(",");
    command.append(QString::number(P_BANKNUM));
    command.append(",");
    command.append(QString::number(banknum));

    qDebug() << "updatestatusBank command:" << command;
    sendCommand(command);

    command = QString::number(A_STATUS);
    command.append(",");
    command.append(QString::number(P_BANKLABEL));
    command.append(",");
    command.append(banklabel);

    qDebug() << "updatestatusBank command:" << command;
    sendCommand(command);
}

void ArduinoComm::updateEncoderLabel(int encDisplay, int encNum, QString label)
{
    QString command = QString::number(encDisplay);
    command.append(",");
    command.append(QString::number(encNum));
    command.append(",");
    command.append(label);

    qDebug() << "updateEncoderLabel command:" << command;
    sendCommand(command);

}

void ArduinoComm::updateButtonLabel(int buttonNum, QString label)
{
    QString command = QString::number(A_BUTTONS);
    command.append(",");
    command.append(QString::number(buttonNum));
    command.append(",");
    command.append(label);

    qDebug() << "updateButtonLabel command:" << command;
    sendCommand(command);
}
