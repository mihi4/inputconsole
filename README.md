# inputconsole
This project was created to design hard- and software similar to the available colorcorrection panels like Tangent Element or similar to control video/photography editing software (or other).

The hardware consists of:
* 1 Arduino Pro Micro
* 4 128x32 pixel SSD1306 OLED displays
* 1 128x64 pixel SSD1306 OLED display
* 4 MCP23017 i2c I/O expander
* 1 TCA9548A 1-to-8 I2C Multiplexer Breakout

* 1 ShuttleProV2 input hardware


Input hardware is an Arduino Pro Micro which will look to the computer like a joystick.
To convert the inputs to keyboard commands, Antimicro is used.


The hardware will include some displays to show the current mapping of buttons and encoders.
Info about the current config will be sent from Antimicro to the hardware through the serial port.
So Antimicro will have to get new functions to send these config back to the hardware.
