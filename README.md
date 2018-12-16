# inputconsole
project to design a hard- and software to control video/photography editing software (or other)

Input hardware is an Arduino Pro Micro which will look to the computer like a joystick.
To convert the inputs to keyboard commands, Antimicro is used.

The hardware will include some displays to show the current mapping of buttons and encoders.
Info about the current config will be sent from Antimicro to the hardware through the serial port.
So Antimicro will have to get new functions to send these config back to the hardware.
