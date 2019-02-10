# VideoInputconsole
This project was created to design hard- and software similar to the available colorcorrection panels like Tangent Element or similar to control video/photography editing software (or other).

## Hardware:
* 1 Arduino Pro Micro
* 4 128x32 pixel SSD1306 OLED displays
* 1 128x64 pixel SSD1306 OLED display
* 4 MCP23017 i2c I/O expander
* 1 TCA9548A 1-to-8 I2C Multiplexer Breakout
* 1 ShuttleProV2 input hardware

## Software:
* selfwritten Arduino code
* modified Antimicro

The complete project is build on the Arduino Pro Micro, because it shows up to the OS as HID-compatible Joystick.
This way, all inputs can be mapped to keyboard commands through Antimicro without any problems.
To overcome the Micro's shortcoming on I/O-pins, all inputs and displays are connected on the i2c-bus.

4 mcp23017 provide 64 additional inputs which are used as followed:
* 16: 8 digital encoders with 2 inputs each for CW/CCW rotation inputs
* 16: 16 momentary buttons
* 8: one momentary button in each encoders (mostly used to reset a configuration)
* 12: numpad with 0-9 and DEL and ENTER on it (to input timecode,...)
* 1: "Set" button, to switch between different sets in Antimicro.
That's 53 total inputs used with some inputs left.

As with Tangent's input consoles, a "Shift" button is implemented, which gives every input a 2nd function when pressed.
This button is directly connected to a digital input on the Arduino, since it only changes Arduino's internal button status and does not send any command to the PC.

With 16 buttons and 8 encoders there are 24 functions available at once.
When using the Shiftbutton, another 24 functions are available, that's 48 in total.
But that's only the first set, since Antimicro gives you 8 sets to configure, that's 384 functions per configuration.
That should be enough for every software I want to control.
(In case, it's not, there are still the 8 push-functions in each encoder, which adds another 64 total functions ;))

Since it's quite hard to memorize all functions in each set by heart, 4 OLED displays show the current configuration of each encoder and each button.
Configuration changes or changes of the current set will be send from Antimicro to the Arduino through the serial port in realtime.
