#include "arduinoconnector.h"


ArduinoConnector::ArduinoConnector(QMap<SDL_JoystickID, InputDevice *> *joysticks, AntiMicroSettings *settings, QObject *parent) :
    communicator(settings->value("Arduino/SerialPortId").toString(), this)
{
    this->joysticks = joysticks;
    this->settings = settings;

    this->arduinoGuid = settings->value("Arduino/JoystickGuid").toString();

    qDebug() << "======= Settings =========";
    qDebug() << "portid" << settings->value("Arduino/SerialPortId").toString();
    qDebug() << "guid" << settings->value("Arduino/JoystickGuid").toString();

    connect(&communicator, &ArduinoComm::connectionEstablished, this, &ArduinoConnector::arduinoConnected);
    communicator.connectToArduino();
    initArduino();
}

void ArduinoConnector::initArduino()
{
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();
        if (joystick)
        {
            //arduinoGuid = "03000000412300003780000001010000";
            qDebug() << "Name:" << joystick->getName() << "Setnumber" << joystick->getActiveSetNumber() << "sdlname:" << joystick->getSDLName() <<  "guidstring" << joystick->getGUIDString() << arduinoGuid;
            if (joystick->getGUIDString() == arduinoGuid)
            {
                qDebug() << "found Arduino guid:" << joystick->getGUIDString();
                arduino = joystick;
                connect(arduino, &Joystick::profileLoaded, this, &ArduinoConnector::updateArduino);
                connect(arduino, &Joystick::setChangeActivated, this, &ArduinoConnector::updateSet);
                //connect(arduino, &Joystick::profileLoaded, this, )
            }
        }
    }
}

void ArduinoConnector::updateEncoders()
{
    // FIXXXME read from settings
    int encoderStart1=0;
    int encoderStart2=8;
    int encoderTotal=4;
    if (communicator.isArduinoHere())
    {
        for (int i=0;i<encoderTotal;i++)
        {
            int buttonNo = i*2;
            communicator.updateEncoderLabel(A_ENC1, i, arduino->getActiveSetJoystick()->getJoyButton(buttonNo+encoderStart1)->getActionName());
            communicator.updateEncoderLabel(A_ENC2, i, arduino->getActiveSetJoystick()->getJoyButton(buttonNo+encoderStart2)->getActionName());
        }

    }
}

void ArduinoConnector::updateButtons()
{
    //FIXXXME read from settings
    int buttonsTotal = 32;
    int buttonsStart = 40;

    if (communicator.isArduinoHere())
    {
        for (int i = 0; i<buttonsTotal; i++)
            communicator.updateButtonLabel(i, arduino->getActiveSetJoystick()->getJoyButton(i+buttonsStart)->getActionName());
    }
}

void ArduinoConnector::updateSet(int setNum)
{
    QString setName = "";
    setName = arduino->getSetJoystick(setNum)->getName();
    qDebug() << "Set updated:" << setNum << "name:" << setName;
    if (communicator.isArduinoHere())
    {
        communicator.updateStatusBank(setNum, setName);
    }
    updateButtons();
    updateEncoders();


}

void ArduinoConnector::updateConfig()
{
    QString profile = "";
    profile = arduino->getProfileName();
    qDebug() << "in updateConfig" << "profilename: " << profile;
    if (communicator.isArduinoHere())
    {
        communicator.updateStatusConfig(profile);
    }
    int setNum = arduino->getActiveSetJoystick()->getIndex();
    updateSet(setNum);
}

void ArduinoConnector::updateArduino(InputDevice *inputDevice)
{
    if (inputDevice != arduino) {
        qDebug() << "profile updated is not arduino";
        return;
    }
    qDebug() << "updateArduino called";
    updateConfig();
}


/*
void ArduinoConnector::updateProfile(QString location)
{
    qDebug() << "updateProfile" << location << "profilename" << arduino->getProfileName();
    QString profileConfig = "";
    if (arduino->getProfileName() != "") profileConfig = arduino->getProfileName();
    if (location == "" && profileConfig == "")
    {
        profileConfig = "n/a";
    } else
    {
        profileConfig = ((location.section('/', -1)).section('.',0,0)).remove("arduino_");
    }
    qDebug() << "profileConfig" << profileConfig;

    if (communicator.isArduinoHere())
    {
        communicator.updateStatusConfig(profileConfig);
    }
}
*/

void ArduinoConnector::updateAction()
{
    qDebug() << "in updateAction";
    qDebug() << "Setnumber:" << arduino->getActiveSetNumber();
}

void ArduinoConnector::arduinoConnected()
{
    updateArduino(arduino);
}

void ArduinoConnector::onAutoProfileLoaded()
{
    qDebug() << "#### configChanged called! ####";
    qDebug() << "profilename" << arduino->getProfileName();
    qDebug() << "Action of Button 0" << arduino->getActiveSetJoystick()->getJoyButton(48)->getActionName();

}

