
#include <SPI.h>
#include <Ethernet.h>
#include<EthernetUdp.h>
#include "Adafruit_MAX31855.h"
//Needed for OLED
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// OLED display TWI address /////////////////////////////////////////////////////////////////////////
#define OLED_ADDR   0x3C
// reset pin not used on 4-pin OLED module
Adafruit_SSD1306 display(-1);  // -1 = no reset pin
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Ethernet Parameters /////////////////////////////////////////////////////////////////////////
#define UDP_TX_PACKET_MAX_SIZE 150 //increase UDP size
EthernetUDP Udp;
byte mac[] = { 0xDF, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_GRLab_Dev03(169, 254, 128, 108);
char recvdBuffer[10];
int recvdPacketSize;
unsigned long ethLastRecvd = 0;
unsigned long ethTimeout = 40000;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Thermocouple Variables ////////////////////////////////////////////////////////////////////////////////
#define DO  2 // Common MOSI pin
#define CLK 3 // Common serial clock pin
#define CS01  4 // Chip select pins
#define CS02  5
#define CS03  6
#define CS04  7

//THERMOCOUPLES
Adafruit_MAX31855 thermocouple01(CLK, CS01, DO);
Adafruit_MAX31855 thermocouple02(CLK, CS02, DO);
Adafruit_MAX31855 thermocouple03(CLK, CS03, DO);
Adafruit_MAX31855 thermocouple04(CLK, CS04, DO);



// DEFINE PINS /////////////////////////////////////////////////////////////////////////////////////////
int SeedOutput_Read = A3; // Analog input channel sensor ADC values
int PD4Output_Read = A2;
int RepumpOutput_Read = A1;
int BoosTAOutput_Read = A0;

int ShutterButtonPin = 46;
int ShutterPin = 41;
int ShutterPin_Read = 40;

int InterlockButtonPin = 42;
int InterlockPin = 39;
int InterlockPin_Read = 38;

int WatchdogButtonPin = 43;
int WatchdogPin = 44;
int WatchdogTripPin_Read = 45;

int LasermetShutterPin_Read = 34;
int RepumpShutterPin_Read = 35;

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "GRLab,Dev03";
String payloadStr;

double temps [4]; // Buffer to hold thermocouple data

int BoosTAOutputVal = 0;
int RepumpOutputVal = 0;
int PD4OutputVal = 0;
int seedOutputVal = 0;

const unsigned int delayTime = 0;
const float seedPowerThresholdLower = 15;
const float seedPowerThresholdUpper = 35;
float seedPower;
float BoosTAPower;
float RepumpPower;
float PD4Power;

bool watchdogTripped = false;

// Debounce Stuff ///////////////////////////////////////////////////////////////////////
const int buttons = 3; // number of buttons you use
const int ShuttersInterlocks = 4; // number of shutters and interlocks

int currState[ShuttersInterlocks] = {0};                  // both current state and state are needed for
int state[buttons] = {1};                                 // interlock and shutter because there are switches
int buttonState[buttons] = {1};                           // external to ard control that effect these states
int lastButtonState[buttons] = {1};
int buttonPin[buttons];
unsigned long lastDebounceTime[buttons] = {0};
/////////////////////////////////////////////////////////////////////////////////////////

// Display Screen Variables /////////////////////////////////////////////////////////////
const unsigned long screenUpdateInterval = 1000; // How often to take a data sample in ms
unsigned long previousScreenUpdateMillis = 0; // Time at which previous sample was acquired
unsigned long currentMillis = 0; // Current time elapsed since program started in ms
/////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  //display.dim(true);

  // Allow ethernet chip time to initialise after being powered up
  delay(1000);
  //Serial.begin(57600);

  pinMode(53, OUTPUT); // Needed for slave select SPI for Mega (not connected)
  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)

  pinMode(ShutterButtonPin, INPUT_PULLUP);
  pinMode(InterlockButtonPin, INPUT_PULLUP);
  pinMode(WatchdogButtonPin, INPUT_PULLUP);

  pinMode(ShutterPin, OUTPUT);
  pinMode(InterlockPin, OUTPUT);
  pinMode(WatchdogTripPin_Read, OUTPUT);

  Ethernet.begin(mac, IP_GRLab_Dev03);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise
  textDisplay("Ethernet initialisation started");


}

void loop() {

  currentMillis = millis();

  // READ PHOTODIODE POWERS /////////////////////////////////////////////////////////////
  seedPower = 0.0;
  BoosTAPower = 0.0;
  RepumpPower = 0.0;
  
  seedOutputVal = analogRead(SeedOutput_Read);
  seedPower = (seedOutputVal) / 684.0 * 23.0;
  BoosTAOutputVal = analogRead(BoosTAOutput_Read);
  BoosTAPower = (BoosTAOutputVal) / 679.0 * 2750.0 * 1.1736;
  RepumpOutputVal = analogRead(RepumpOutput_Read);
  RepumpPower = (RepumpOutputVal) / 347.0 * 57.5;
  
  PD4OutputVal = analogRead(PD4Output_Read);
  PD4Power = (PD4OutputVal) / 1023.0 * 1.0;
  ///////////////////////////////////////////////////////////////////////////////////////

  // DETECT STATE CHANGE OF DCHP INTERLOCK BUTTON ///////////////
  // DETECT STATE CHANGE OF WATCHDOG LISTENING BUTTON ///////////
  // READ LASERMET SHUTTER STATE ////////////////////////////////
  // READ FIBRE DOCK SHUTTER STATE //////////////////////////////
  // READ DCHP INTERLOCK STATE //////////////////////////////////
  // READ BOOSTA OUTPUT POWER ///////////////////////////////////
  // READ REPUMP POWER //////////////////////////////////////////


  // DEFINE REFERENCES ///////////////////////////////////////////////////////////
  int Shutter = 0;
  int Interlock = 1;
  int Watchdog = 2;
  int LasermetShutter = 2;
  int RepumpShutter = 3;

  buttonPin[Shutter] = ShutterButtonPin;
  buttonPin[Interlock] = InterlockButtonPin;
  buttonPin[Watchdog] = WatchdogButtonPin;
  ////////////////////////////////////////////////////////////////////////////////////////

  // CHECK BUTTON STATES /////////////////////////////////////////////////////////////////
  if (!watchdogTripped) {
    if (buttonStateChange(Shutter)) {
      //Serial.println("Shutter State Changed");
      digitalWrite(ShutterPin, state[Shutter]);
    }

    if (buttonStateChange(Interlock)) {
      //Serial.println("Interlock State Changed");
      digitalWrite(InterlockPin, state[Interlock]);
    }
  }

  if (buttonStateChange(Watchdog)) {
    //Serial.println("Watchdog State Changed");
    digitalWrite(WatchdogPin, state[Watchdog]);
    if (watchdogTripped) { // resets the watchdog
      watchdogTripped = false;
      digitalWrite(WatchdogTripPin_Read, LOW);
    }
  }

  currState[Shutter] = digitalRead(ShutterPin_Read);
  currState[Interlock] = digitalRead(InterlockPin_Read);

  // Account for the fact that the optocoupler circuit has inversion
  if (digitalRead(LasermetShutterPin_Read) == HIGH) {
    currState[LasermetShutter] = 0;
  } else {
    currState[LasermetShutter] = 1;
  }

  if (digitalRead(RepumpShutterPin_Read) == HIGH) {
    currState[RepumpShutter] = 0;
  } else {
    currState[RepumpShutter] = 1;
  }

  ///////////////////////////////////////////////////////////////////////////////////////

  // WATCHDOG CONDITIONS ////////////////////////////////////////////////////////////////
  if (state[Watchdog]) {

    // Trip conditions: fibre dock shutter open AND seed power too low
    //               OR fibre dock shutter open AND seed power too high
    //               OR fibre dock shutter open AND DCHP controller interlock disabled

    //               OR BoosTA current applied AND fibre dock shutter closed
    //               OR BoosTA current applied and seed power too low


    if (((currState[Shutter] == 1) && (seedPower < seedPowerThresholdLower)) ||
        ((currState[Shutter] == 1) && (seedPower > seedPowerThresholdUpper))) {
      //Serial.println("WARNING: Trip condition met");
      digitalWrite(WatchdogTripPin_Read, HIGH);
      watchdogTripped = true;
      digitalWrite(ShutterPin, LOW);
      state[Shutter] = 0;
      digitalWrite(InterlockPin, LOW);
      state[Interlock] = 0;
      digitalWrite(WatchdogPin, LOW);
      state[Watchdog] = 0;
    }

  }
  ///////////////////////////////////////////////////////////////////////////////////////

  // ETHERNET /////////////////////////////////////////////////////////////////////////////
  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {

    static bool packet1 = true;
    if (packet1) {
      textDisplay("Ethernet connection established");
      packet1 = false;
    }

    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);

    ethLastRecvd = millis();

    // Read thermocouples
    temps[0] = thermocouple01.readCelsius();
    temps[1] = thermocouple02.readCelsius();
    temps[2] = thermocouple03.readCelsius();
    temps[3] = thermocouple04.readCelsius();

    payloadStr = headerStr
                 + "," + seedOutputVal
                 + "," + BoosTAOutputVal
                 + "," + RepumpOutputVal
                 + "," + seedPower
                 + "," + BoosTAPower
                 + "," + RepumpPower;

    for (int i = 0; i < 4; i++) {
      if (isnan(temps[i])) {
        payloadStr = payloadStr + "," + "0.0";
      } else {
        payloadStr = payloadStr + "," + temps[i];
      }
    }

    payloadStr = payloadStr
                 + "," + String(currState[Shutter])
                 + "," + String(currState[Interlock])
                 + "," + String(state[Watchdog])
                 + "," + String(watchdogTripped)
                 + "," + String(currState[LasermetShutter])
                 + "," + String(currState[RepumpShutter]);

    //Serial.println(payloadStr);
    Udp.beginPacket(IP_Collector, localPort);
    Udp.print(payloadStr);
    Udp.endPacket();

    memset(recvdBuffer, 0, sizeof(recvdBuffer));
  }
  ///////////////////////////////////////////////////////////////////////////////////////

  // UPDATE DISPLAY SCREEN //////////////////////////////////////////////////////////////
  if ((currentMillis - previousScreenUpdateMillis) >= screenUpdateInterval) { // TODO: OR if anything has been tripped OR button been pressed
    //Serial.print("Repump val = ");
    //Serial.print(RepumpOutputVal);
    //Serial.print(" ,Sensor Value = ");
    //Serial.print(seedOutputVal);
    //Serial.print(" , Seed Power = ");
    //Serial.println(seedPower);
    
    display.setCursor(0, 0);

    if (seedPower < seedPowerThresholdLower) {
      //Serial.println("Seed Power Too Low!");
      display.println("Seed Power Too Low!");
    }
    else if (seedPower > seedPowerThresholdUpper) {
      //Serial.println("Seed Power Too High!");
      display.println("Seed Power Too High!");
    } else {
      //Serial.println("Seed Power is Fine");
      display.println("Seed Power is Fine");
    }

    display.setCursor(0, 8);
    display.print("Seed Power = ");
    display.println(seedPower);
    static int OLEDupdate = 0;
    String currmsg;
    if (OLEDupdate % 5 == 0) {
      currmsg = "BoosTA Power = " + String(BoosTAPower);
      textDisplay(currmsg);
      currmsg = "Repump Power = " + String(RepumpPower);
      textDisplay(currmsg);
      currmsg = "Lasermet = " + String(currState[LasermetShutter]);
      textDisplay(currmsg);
      currmsg = "Repump = " + String(currState[RepumpShutter]);
      textDisplay(currmsg);
    }
    OLEDupdate++;

    currmsg = "Shutter Current = " + String(currState[Shutter]);
    //Serial.println(currmsg);
    currmsg = "Interlock State = " + String(currState[Interlock]);
    //Serial.println(currmsg);
    currmsg = "Watchdog Listening=" + String(state[Watchdog]);
    //Serial.println(currmsg);
    currmsg = "Watchdog Tripped=" + String(watchdogTripped);
    //Serial.println(currmsg);
    currmsg = "Lasermet Shutter State = " + String(currState[LasermetShutter]);
    //Serial.println(currmsg);
    currmsg = "Repump Shutter State = " + String(currState[RepumpShutter]);
    //Serial.println(currmsg);
    //Serial.println("");
    previousScreenUpdateMillis += screenUpdateInterval;
  }

  if ((millis() - ethLastRecvd) > ethTimeout) {
    static int displayDelay = 0;
    if (displayDelay % 10000 == 0) {
      textDisplay("Ethernet error Check connection and/or Restart device");
    }
    displayDelay++;
  }
  /////////////////////////////////////////////////////////////////////////////////////////
 delay(10);
}

//*************** DETECT STATE CHANGE OF BUTTON ***************//
bool buttonStateChange(int button) {
  int reading = digitalRead(buttonPin[button]);
  int debounceDelay = 50;
  if (reading != lastButtonState[button]) {
    // reset the debouncing timer
    lastDebounceTime[button] = millis();
  }
  if ((millis() - lastDebounceTime[button]) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonState[button]) {
      buttonState[button] = reading;
      if (buttonState[button] == LOW) {
        state[button] = !state[button];
        lastButtonState[button] = reading;
        return true;
      }
    }
  }
  lastButtonState[button] = reading;
  return false;
}

//*************** FIFO OLED STRING DISPLAY ***************//
// No. of lines = 8
// Max char per line = 21
void textDisplay(String msg) {
  const int displayLines = 6; // number of fifo display lines wanted
  static String OLEDstrings[displayLines] = "";
  int len[displayLines] = {0};
  int linecalc = 0;
  int startStr = 0;

  //  flashes display when updated
  //  display.display();
  //  delay(1);

  if (msg == "") msg = " ";
  // fills fifo array with input strings
  if (OLEDstrings[(displayLines - 1)] == "") {
    for (int line = 0; line < displayLines; line++) {
      if (OLEDstrings[line] == "") {
        OLEDstrings[line] = msg;
        len[line] = OLEDstrings[line].length(); // fills array with length of corresponding string
        break;
      }
    }
  } else {
    for (int line = 0; line < (displayLines - 1); line++) {
      OLEDstrings[line] = OLEDstrings[line + 1];
      len[line] = OLEDstrings[line].length(); // fills array with length of corresponding string
    }
    OLEDstrings[(displayLines - 1)] = msg;
    len[(displayLines - 1)] = msg.length();
  }

  // works out which string in OLEDstrings to start displaying from
  for (int i = (displayLines - 1); i >= 0; i--) {  // starting with the last input working up
    if (OLEDstrings[i] != "") {
      linecalc += (len[i] / 21);                   // sums number of lines until either...
      if (len[i] % 21 != 0) linecalc++;
    }
    if (linecalc == displayLines) {                // ...the right number of lines is hit (start display loop with current string no)
      startStr = i;
      break;
    } else if (linecalc > displayLines) {           // OR it goes over the number of lines (start with previous string no)
      if (i = 1) startStr = i;
      startStr = i + 1;
      break;
    }
  }

  //Displays array
  int Xcursor = (8 - displayLines) * 8;
  display.setCursor(0, Xcursor);
  for (int line = startStr; line < displayLines; line++) {
    display.println(OLEDstrings[line]);
  }
  display.display();
  delay(1);
  //clears display
  display.fillRect(0, Xcursor, display.width(), (display.height() - Xcursor), BLACK);
}
