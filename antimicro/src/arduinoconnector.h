#ifndef ARDUINOCONNECTOR_H
#define ARDUINOCONNECTOR_H

#include <QObject>
#include <QtDebug>
#include <QMap>
#include <QMapIterator>
#include <QSerialPort>

#include "inputdevice.h"
#include "setjoystick.h"
#include "joystick.h"

#include "arduinocomm.h"
#include "antimicrosettings.h"


#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_platform.h>
#else
#include <SDL/SDL_joystick.h>
typedef Sint32 SDL_JoystickID;
#endif


class ArduinoConnector : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoConnector(QMap<SDL_JoystickID, InputDevice*> *joysticks, AntiMicroSettings *settings, QObject *parent = nullptr);
    ArduinoComm communicator;
public slots:
    void onAutoProfileLoaded();

protected slots:
    void updateAction();
    void arduinoConnected();

private:
    QString arduinoGuid = ""; //"03000000412300003780000001010000"; // FIXXXME, make configurable
    const QString mySDLName = "Arduino LLC Arduino Micro";

    QMap<SDL_JoystickID, InputDevice*> *joysticks;
    AntiMicroSettings *settings;
    InputDevice *arduino;

    SDL_JoystickID arduinoSDLid;

    void updateArduino(InputDevice *inputDevice);
    void initArduino();
    void updateConfig();
    void updateSet(int setNum);
    void updateButtons();
    void updateEncoders();
signals:


};

#endif // ARDUINOCONNECTOR_H
