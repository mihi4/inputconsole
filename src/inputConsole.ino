#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Adafruit_MCP23017.h"
#include <ssd1306.h>
#include <Joystick.h>

#define TCAADDR 0x70
#define DISP_INFO 1 // FIXXXME change for final hardware
#define DISP_ENC1 0
#define DISP_ENC2 2

#define MCP_ADDR_BUTTON1 1 //A0 5v, A1,A2 GND
//#define MCP_ADDR_BUTTON2 2 //A0 GND, A1 5V, A2 GND  
/*
addr 0 = A2 low , A1 low , A0 low  000
addr 1 = A2 low , A1 low , A0 high 001
addr 2 = A2 low , A1 high , A0 low  010
addr 3 = A2 low , A1 high , A0 high  011
addr 4 = A2 high , A1 low , A0 low  100
addr 5 = A2 high , A1 low , A0 high  101
addr 6 = A2 high , A1 high , A0 low  110
addr 7 = A2 high, A1 high, A0 high 111
 */

/*
 * display configuration
 */
String configName = "No App connected";
String connStatus = "off";
String bankNo = "0";

String inputString = ""; // input string from Serial

LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x3F for a 20 chars and 4 line display

// each encoder display has 8 labels, first 4 are normal, 2nd 4 are shifted
struct EncDisplay
{
  int tcaNum;
  String label[8];
};

EncDisplay encDisp[] = {
  {DISP_ENC1,{" enc1.1   ","   enc1.2 "," enc1.3   ","   enc1.4 "," ENC1.1   ","   ENC1.2 "," ENC1.3   ","   ENC1.4 "},},
  {DISP_ENC2,{" enc2.1   ","   enc2.2 "," enc2.3   ","   enc2.4 "," ENC2.1   ","   ENC2.2 "," ENC2.3   ","   ENC2.4 "},}
};

// array of button labels, first 12 are normal, 2nd 12 are shifted
String buttons[] = {
  "but 01","but 02","but 03","but 04","but 05","but 06","but 07","but 08","but 09","but 10","but 11", "but 12", 
  "shf 01","shf 02","shf 03","shf 04","shf 05","shf 06","shf 07","shf 08","shf 09","shf 10","shf 11","shf 12"
};

// constants for display configurations
const int statusX = 65; // xpos of antimicroc connection status 
const int statusY = 35; // ypos
const int bankX = 40; // xpos of bank number display
const int bankY = 23; // ypos
const int buttonsPerLine = 3; // number of buttons to display per line on LCD
const int buttonNum = 12; // total number of buttons to display
const int encNum = 4; // number of encoders to display on one OLED

// configuration of string length for displays
const int buttonLength = 6;
const int encLength = 10;
const int configLength = 20;
const int statusLength = 3;

/*
 * joystick configuration
 */
const int joystickbuttons = 72;
const int joystickEncoderStart = 4; // use buttons 0-4 
const int joystickMcp1Start = 36;
const int joystickMcp1Buttons = 12; // only 12 inputs used for buttons

Joystick_ Joystick = Joystick_(0x05, JOYSTICK_TYPE_JOYSTICK, joystickbuttons, 0, true, true, false, false, false, false, false, false, false, false, false);

/*
 * mcp configuration
 */
Adafruit_MCP23017 mcp0; // rotary encoders
Adafruit_MCP23017 mcp1; // buttons no1. free defineable
// Adafruit_MCP23017 mcp2; // buttons no1. numpad and < O >

// status of mcp registers
uint16_t buttonsMcp1Current = 0;
uint16_t buttonsMcp1Previous = 0;

/*
 * shift button configuration
 */
const int shiftButton = 7; // hardware input pin of shift button
boolean shiftEnabled = false;
boolean shiftChanged = false;
const char mcpButtonNum = 16;
const char shiftEncOffset = 16; // same for encoders

/*
 * test mode configuration
 */
boolean testMode=true; //after start display debug info about buttons and encoders
int testCounter = 0; // add/remove 1 for encoders

/*
 * encoder configuration
 */
boolean change=false;        // goes true when a change in the encoder state is detected
int encSelect[2] = {101, 0}; // stores the last encoder used and direction {encNo, 1=CW or 2=CCW}

const int encCount0 = 2;  // number of rotary encoders

// arrays to store the previous value of the encoders and buttons
unsigned char encoders0[encCount0];

// encoder pin connections to MCP23017
//    EncNo { Encoder pinA  GPAx, Encoder pinB  GPAy },
const int encPins0[encCount0][2] = {
  {0,1},   // enc:0 AA GPA0,GPA1 - pins 21/22 on MCP23017
  {2,3}    // enc:1 BB GPA3,GPA4 - pins 24/25 on MCP23017
};

/////////////////////////////////////////// common functions ///////////////////////////////////

void debug(String message)
{
  Serial.println(message.c_str());
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}


//*************************************** display functions *********************************
/*
 ******************* 
 * Draw Infodisplay 
 ******************* 
 */
void drawInfoBank()
{
  tcaselect(DISP_INFO);
  int startx = 40;  
  ssd1306_printFixed(40,bankY, bankNo.c_str() , STYLE_NORMAL);
}
void drawInfoStatus()
{
  tcaselect(DISP_INFO);
  if (connStatus == "off")   
  {
    ssd1306_printFixed(statusX,statusY,connStatus.c_str(), STYLE_ITALIC);
  }
  else
  {
    ssd1306_printFixed(statusX,statusY,connStatus.c_str(), STYLE_BOLD);
  }
}
void drawInfoConfig()
{
  tcaselect(DISP_INFO);
  ssd1306_printFixed(0,0, configName.c_str(), STYLE_NORMAL);
}
void drawInfoDisplay()
{
  tcaselect(DISP_INFO);
  
  drawInfoConfig();

  tcaselect(DISP_INFO);
  ssd1306_printFixed(0,bankY, "Bank: ", STYLE_NORMAL); 
  //ssd1306_drawLine(0,18,128,18);
  //ssd1306_drawLine(0,19,128,19);
  tcaselect(DISP_INFO);
  ssd1306_drawLine(0,11,128,11);
  ssd1306_drawLine(0,12,128,12);
  ssd1306_drawLine(0,13,128,13);
  tcaselect(DISP_INFO);
  ssd1306_printFixed(0,statusY,"Antimicro:", STYLE_NORMAL);
 
  drawInfoBank();
  drawInfoStatus();
}
void drawInfoDebug(String message)
{
  int msgLength = message.length();
  if (msgLength <= configLength)
  {
    for (int i = 0; i<configLength-msgLength; i++)
    {
      message += " ";
    }
  }
  else
  {
    message = message.substring(0,configLength);
  }
  tcaselect(DISP_INFO);
  ssd1306_printFixed(0,statusY+18, message.c_str(), STYLE_NORMAL);
}

/***********************
 * draw EncoderDisplays
 ***********************
 */
void drawEncLabel(EncDisplay display, int labelIndex)
{
  int posX = 0 + 67*(labelIndex%2);;
  int posY = (labelIndex < (encNum/2)) ? 0 : 31;
  tcaselect(display.tcaNum);
  ssd1306_printFixed(posX, posY, (display.label[labelIndex+(shiftEnabled*encNum)]).c_str(), STYLE_NORMAL);
}
void drawEncLabels(EncDisplay display)
{
  for (int i = 0; i<encNum;i++)
  {    
    drawEncLabel(display, i);
  }
}
void drawEncLines(EncDisplay display)
{
  tcaselect(display.tcaNum);
  ssd1306_drawLine(0,16,128,16);
  ssd1306_drawLine(64,0,64,32);
}
void drawEncStart(EncDisplay display)
{
  tcaselect(display.tcaNum);
  ssd1306_clearScreen();
  drawEncLines(display);
  drawEncLines(display);
  drawEncLabels(display);
  drawEncLabels(display);
}

/*
 * *********************
 *  Draw Button Display
 * *********************
 */
void drawButton(int butNo) 
{
  int line = butNo/buttonsPerLine;
  int col = (butNo-(line*buttonsPerLine))*7;
  lcd.setCursor(col,line);
  lcd.print(buttons[butNo+(shiftEnabled*buttonNum)]);
  //debug("bt:"+String(butNo)+" line:"+String(line)+" col:"+String(col));
}
void drawButtons()
{
  lcd.clear();
  for (int i=0;i<12;i++)
  {
    drawButton(i);
  }
  for (int y=0;y<4;y++)
  {
      lcd.setCursor(6,y);
      lcd.print("|");
      lcd.setCursor(13,y);
      lcd.print("|");
  }
}

/*
 * connect to Antimicro
 */
void establishContact() {
  while (Serial.available() <= 0) { // wait for antimicro to respond
    Serial.print('A');   // send a capital A
    delay(300);
  }
  /*connStatus = "on ";
  drawInfoStatus(); */
}

/*
 * command structure:  command,parameter,value
 * 
 * commands:
 * 0 - infodisplay
 *  parameter:  0 - current application
 *              1 - current bank number
 * 
 * 1 - enc display 1
 *  parameter:  0-3 - encoder number 
 *  value: button label
 *  
 * 2 - enc display 2
 *  parameter:  0-3 - encoder number
 *  value: button label
 * 
 * 3 - buttondisplay
 *  parameter:  0-11 - button number
 *  value: button label
 *  
 */
void parseIncomingCommand()
{
  String value = "";
  String command = "";
  String parameter = "";

  int ind1 = inputString.indexOf(',');  //finds location of first ,
  command  = inputString.substring(0, ind1);   //captures first data String
  int ind2 = inputString.indexOf(',', ind1 + 1);   //finds location of second ,
  parameter = inputString.substring(ind1+1, ind2);   //captures second data String
  value = inputString.substring(ind2+1, inputString.length()-1); 

  // incoming check from software if we are here
  if ( (ind1 < 0) && (command.substring(0,command.length()-1).equals("uhere")) )
  {
    Serial.println("yes");
  }

  // "ok" return from antimicro
  if ( (ind1 < 0) && (command.substring(0,command.length()-1).equals("ok")) )
  {
    connStatus = "ON ";
    testMode = false;
    drawButtons();
    drawInfoStatus();
    drawEncStart(encDisp[0]);
    drawEncStart(encDisp[1]);      
    return;
  }

  // check inputs for infodisplay
  //------------------------------
  if (command.equals("0")) 
  {
    if (parameter.equals("0"))  // change in connected app
    {
      int strLength = value.length();
      if (strLength > configLength) 
      {
        value = value.substring(0,configLength);
      }
      else
      {
        for (int i=0;i<(configLength-strLength);i++)
        {
          value += " ";
        }
      }
      configName = value;
      drawInfoConfig();
      return;
    }
    if (parameter.equals("1"))  // change in bank number
    {
      bankNo = value + "  "; //make sure to overwrite bigger numbers
      drawInfoBank();
      return;      
    }
  }
  
  // check inputs for encoder displays
  //-----------------------------------
  if ( (command.equals("1")) || (command.equals("2")) )
  {
    int encNumber = command.toInt() - 1; // there are only 2 encDisplays 1 and 2
    int labelNum = parameter.toInt();
    if (labelNum > encNum) return;
    if (value.length() <= encLength) // check if string is not too long
    {
      //debug ("short value:" + value + ":::" + String(value.length()));
      // center in 10 (encLength) digits
      int firstHalve = (encLength-value.length())/2;
      //debug ("firstHalve:" + String(firstHalve));
      if (!((encLength-value.length())%2))  // modulo 0 = true, same distance left and right
      {
        //debug ("equal sides, margin:" + String(firstHalve));
        for (int i=0; i<firstHalve;i++)
        {
          value = " " + value + " ";
        }
      }
      else
      {
        if ((labelNum%2)) firstHalve++; // buttons on the right side get lower margin
        int secondHalve = encLength-value.length()-firstHalve;
        for (int i=0; i<firstHalve;i++)
        {
          value = " " + value;
        }
        //debug ("firstHalve:" + String(firstHalve) +"encLength:"+String(encLength)+" secondHalve:"+String(secondHalve)+"vallength:" + String(value.length()));
        for (int i=0; i<secondHalve;i++)
        {
          value += " ";
        }
      }
    }
    else
    {
      value = value.substring(0,encLength);  
    }
    encDisp[encNumber].label[labelNum] = value;
    drawEncLabel(encDisp[encNumber],labelNum);
    return;
  }

  //  check input for button display
  // --------------------------------
  if (command.equals("3")) // button character display
  {
    int buttonIndex = parameter.toInt();
    if (buttonIndex > buttonNum) return; // just accept values up to maximum buttons
    if (value.length() > buttonLength) // check if string is too long
    {
      value = value.substring(0,encLength);      
    }
    else
    {
      int firstHalve = (buttonLength-value.length())/2;
      int secondHalve = buttonLength-value.length()-firstHalve;
      //debug("firstHalve:" + String(firstHalve) + " second:" + String(secondHalve) + " val:" + value + "::");
      for (int i=0; i<firstHalve;i++)
      {
        value = " " + value;
      }
      //debug("valueFirst:" + value + "::");
      for (int i=0; i<secondHalve;i++)
      {
        value += " ";
      }
      //debug("valueSec:" + value + "::");
    }
    buttons[buttonIndex] = value;
    drawButton(buttonIndex);
  }
}

void serialEvent() {
  drawInfoDebug("incoming message");
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      parseIncomingCommand();
      inputString = ""; // reset Inputstring
    }
  }
  drawInfoDebug("message processed");
}

/////////////////////// Rotary Encoder functions /////////////////////

// read the rotary encoder on pins X and Y, output saved in encSelect[encNo, direct]
unsigned char readEnc(Adafruit_MCP23017 mcpX, const int *pin, unsigned char prev, int encNo) {

  unsigned char encA = mcpX.digitalRead(pin[0]);    // Read encoder pins
  unsigned char encB = mcpX.digitalRead(pin[1]);

  if((!encA) && (prev)) { 
    encSelect[0] = encNo;
    if(encB) {
      encSelect[1] = 1;  // clockwise
    }
    else {
      encSelect[1] = 2;  // counter-clockwise
    }
    change=true;
  }
  return encA;
}
// setup the encoders as inputs. 
unsigned char encPinsSetup(Adafruit_MCP23017 mcpX, const int *pin) {
  mcpX.pinMode(pin[0], INPUT);  // A
  mcpX.pullUp(pin[0], HIGH);    // turn on a 100K pullup internally
  mcpX.pinMode(pin[1], INPUT);  // B
  mcpX.pullUp(pin[1], HIGH); 
}
void sendEncoderRotation(int encoder, int rotation)
{
  int button = joystickEncoderStart + (encoder * 2) + rotation - 1 + (shiftEnabled * shiftEncOffset); // -1 because rotation is 1 or 2, to get +0/+1
  Joystick.pressButton(button);
  Joystick.releaseButton(button);
}

//********************** mcp button functions *******************
// mcp button input setup
void mcpButtonSetup(Adafruit_MCP23017 mcpX, int button)
{
  mcpX.pinMode(button, INPUT);
  mcpX.pullUp(button, HIGH);
}
// send button as joystickbutton
void sendMcpButtons(uint16_t gpio)
{
  //debug("sendMCP: gpio=" + String(gpio));
  Serial.print(gpio, BIN);
  for (int button=0; button<mcpButtonNum; button++) 
  {
    int buttonId = button + joystickMcp1Start + (shiftEnabled * mcpButtonNum);
    //char stat = bitRead(gpio, button);
    //debug("ButtonID: " + String(buttonId) + " status:" + String(stat));
    Joystick.setButton(buttonId, !bitRead(gpio, button));
  }
}

/*
 * +++++++++
 * Test Mode
 * +++++++++
 */
void drawTestButton(int button, int status)
{
  debug("drawTest, status=" + String(status));
  lcd.setCursor(5,3);
  if (status)
  {
    lcd.print("-" + String(button) + "-");
  }
  else
  {
    lcd.print("    ");
  }
}
void drawTestMcpButtons(uint16_t gpio)
{
  //debug("sendMCP: gpio=" + String(gpio));
  Serial.println(gpio, BIN);
  for (int button=0; button<mcpButtonNum; button++) 
  {
    int buttonId = button + joystickMcp1Start + (shiftEnabled * mcpButtonNum);
    int stat = !bitRead(gpio, button);
    if (stat) 
    {
      debug("stat is 0");
      drawTestButton(buttonId, stat);
    }
    debug("ButtonID: " + String(buttonId) + " status:" + stat);
    //Joystick.setButton(buttonId, !bitRead(gpio, button));    
  }
}

void drawTestEncoder(int encoderNumber, int rotation)  // rotaion 1 = CW, 2 = CCW
{  
  if (rotation == 1)
  {
    ++testCounter;
  }
  else
  {
    --testCounter;    
  }
  lcd.setCursor(5,2);
  lcd.print("-" + String(encoderNumber) + "- : " + String(testCounter) + "    ");
}

/////////////////////// SETUP //////////////////////

void setup()
{
  Wire.begin(); // wake up i2c bus

  // setup directly connected buttons (shift, bank, resetData)
  pinMode(shiftButton, INPUT);
  pinMode(shiftButton, INPUT_PULLUP);

  mcp0.begin(); // default address 0, no address needed
  mcp1.begin(MCP_ADDR_BUTTON1);

  Joystick.begin();
  
  // setup the pins using loops, saves coding when you have a lot of encoders and buttons
  for (int n = 0; n < encCount0; n++) {
    encPinsSetup(mcp0, encPins0[n]);
    encoders0[n] = 1;  // default state
  }
  for (int n = 0; n < 16; n++)
  {
    mcpButtonSetup(mcp1, n);
  }

  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
  lcd.print("Inputconsole ready!");
  lcd.setCursor(0,2);
  lcd.print("Enc: ");
  lcd.setCursor(0,3);
  lcd.print("Key: ");

  tcaselect(DISP_ENC1);
  ssd1306_128x32_i2c_init();
  tcaselect(DISP_ENC1);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  tcaselect(DISP_ENC1);
  ssd1306_clearScreen();
  ssd1306_printFixed(5,5, "Inputconsole ready!", STYLE_NORMAL);

  tcaselect(DISP_ENC2);
  ssd1306_128x32_i2c_init();
  tcaselect(DISP_ENC2);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  tcaselect(DISP_ENC2);
  ssd1306_clearScreen();
  ssd1306_printFixed(5,5, "Inputconsole ready!", STYLE_NORMAL);

  tcaselect(DISP_INFO);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_128x64_i2c_init();
  ssd1306_clearScreen();
  
  drawInfoDisplay();
  //drawButtons();
  drawInfoDebug("Inputconsole ready!");
  
  Serial.begin(115200);
  /*while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }*/

  // establishContact();  // send a byte to establish contact until receiver responds
}

void loop()
{
  // read encoder statii
  for (int n = 0; n < encCount0; n++) {
     encoders0[n] = readEnc(mcp0, encPins0[n], encoders0[n],n);
  }

  // check, if any command is incoming from antimicro
  if(Serial.available() > 0)
  {
    serialEvent();
  }
  // shift button pressed?
  shiftEnabled = (!(digitalRead(shiftButton)));

  buttonsMcp1Current = mcp1.readGPIOAB(); // FIXXXME - check for interrupts

  if (!testMode)  // if there is already a connection with antimicro
  {
    if (buttonsMcp1Current != buttonsMcp1Previous)
    {
      sendMcpButtons(buttonsMcp1Current);
      buttonsMcp1Previous = buttonsMcp1Current;
    }

    // change displays to shifted labels and back
    if (shiftEnabled != shiftChanged)
    {
      drawButtons();
      drawEncLabels(encDisp[0]);
      drawEncLabels(encDisp[1]);
      shiftChanged = !shiftChanged;
    }

    // process encoder inputs
    if (change == true) {
  
      if (encSelect[0] < 100) {

        sendEncoderRotation(encSelect[0], encSelect[1]);
        
        // set the selection to 101 now we have finished doing things. Not 0 as there is an encoder 0.
        encSelect[0] = 101;
      }
      // ready for the next change
      change = false; 
    }
  
  }
  else   //////////////// test mode enabled //////////////////
  {
    if (shiftEnabled != shiftChanged)
    {
      drawTestButton(shiftButton, (!(digitalRead(shiftButton))));   
      shiftChanged = !shiftChanged;
    }
    

    if (buttonsMcp1Current != buttonsMcp1Previous)
    {
      drawTestMcpButtons(buttonsMcp1Current);
      buttonsMcp1Previous = buttonsMcp1Current;
    }
    
    if (change == true) {
  
      if (encSelect[0] < 100) {

        drawTestEncoder(encSelect[0], encSelect[1]);
        
        // set the selection to 101 now we have finished doing things. Not 0 as there is an encoder 0.
        encSelect[0] = 101;
      }
      // ready for the next change
      change = false; 
    }

  }

  
}

