/* Code for GRLab_Dev01 board

*/
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

// Initialise Ethernet Parameters /////////////////////////////////////////////////////////////////////////
#define UDP_TX_PACKET_MAX_SIZE 150 //increase UDP size
EthernetUDP Udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_GRLab_Dev01(169, 254, 128, 109);
char recvdBuffer[10];
int recvdPacketSize;
unsigned long ethLastRecvd = 0;
unsigned long ethTimeout = 40000;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "GRLab,Dev01";
String payloadStr;
int sensorValueA0 = 0; // Analog input channel sensor ADC values
int sensorValueA1 = 0;
int sensorValueA2 = 0;
int sensorValueA3 = 0;
int sensorValueA4 = 0;
int sensorValueA5 = 0;

// Thermocouple Variables ////////////////////////////////////////////////////////////////////////////////
#define DO  2 // Common MOSI pin
#define CLK 3 // Common serial clock pin

// Chip select pins
#define CS01  4
#define CS02  5
#define CS03  6
#define CS04  7

// Button pin
#define buttonPin 47

//THERMOCOUPLES
Adafruit_MAX31855 thermocouple01(CLK, CS01, DO);
Adafruit_MAX31855 thermocouple02(CLK, CS02, DO);
Adafruit_MAX31855 thermocouple03(CLK, CS03, DO);
Adafruit_MAX31855 thermocouple04(CLK, CS04, DO);

double temps [4]; // Buffer to hold thermocouple data

int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long OLEDonms = 60000; // Screen on time in milliseconds
bool OLEDon = true;
bool updateOLED = false;

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long lastButtonPress = 0; // This allows the screen to be on on start up



void setup() {

  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
  //display.dim(true);

  delay(1000); // Allow ethernet chip time to initialise after being powered up
  Serial.begin(57600);

  pinMode(53, OUTPUT); // Needed for slave select SPI for Mega (not connected)
  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)
  pinMode(buttonPin, INPUT_PULLUP);

  Ethernet.begin(mac, IP_GRLab_Dev01);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise
  textDisplay("Ethernet initialisation started");
}

void loop() {

  recvdPacketSize = Udp.parsePacket();
  
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        lastButtonPress = millis();
        OLEDon = true;
        textDisplay("You booped me!");
      }
    }
  }
  lastButtonState = reading;
  
  if (recvdPacketSize > 0) {

    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);

    ethLastRecvd = millis();
    
    sensorValueA0 = analogRead(A0);
    sensorValueA1 = analogRead(A1);
    sensorValueA2 = analogRead(A2);
    sensorValueA3 = analogRead(A3);
    sensorValueA4 = analogRead(A4);
    sensorValueA5 = analogRead(A5);
//
    // Read thermocouples
    temps[0] = thermocouple01.readCelsius();
    temps[1] = thermocouple02.readCelsius();
    temps[2] = thermocouple03.readCelsius();
    temps[3] = thermocouple04.readCelsius();

    payloadStr = headerStr
                 + "," + sensorValueA0
                 + "," + sensorValueA1
                 + "," + sensorValueA2
                 + "," + sensorValueA3
                 + "," + sensorValueA4
                 + "," + sensorValueA5;

    for (int i = 0; i < 4; i++) {
      if (isnan(temps[i])) {
        payloadStr = payloadStr + "," + "0.0";
      } else {
        payloadStr = payloadStr + "," + temps[i];
      }
    }

    Serial.println(payloadStr);
    Udp.beginPacket(IP_Collector, localPort);
    Udp.print(payloadStr);
    Udp.endPacket();
    
    memset(recvdBuffer, 0, sizeof(recvdBuffer));

    updateOLED = true;
  }
  
  if (((millis() - lastButtonPress) < OLEDonms) && OLEDon && updateOLED){
      textDisplay(payloadStr);
      updateOLED = false;
    }else if(OLEDon && updateOLED){
      OLEDon = false;
      display.clearDisplay();
      display.display();
    }
    
  if ((millis() - ethLastRecvd) > ethTimeout) {
    textDisplay("Ethernet error Check connection Restart device");
    updateOLED = true;
    delay(5000);
  }
}

//*************** Fifo String Display***************//
// No. of lines = 8
// Max char per line = 21
void textDisplay(String msg) {
  static String OLEDstrings[8]= "";
  int len[8] = {0};
  int linecalc = 0;
  int startStr = 0;
  
  display.display(); // flashes display when updated
  delay(1);
  
  if (msg == "") msg = " ";
  // fills fifo array with input strings
  if (OLEDstrings[7] == "") {      
    for (int line = 0; line<8; line++){
      if (OLEDstrings[line] == "") {
        OLEDstrings[line] = msg;
        len[line] = OLEDstrings[line].length(); // fills array with length of corresponding string
        break;
      }
    }
  }else{               
    for (int line=0; line<7; line++){ 
      OLEDstrings[line] = OLEDstrings[line+1];
      len[line] = OLEDstrings[line].length(); // fills array with length of corresponding string
    }
  OLEDstrings[7] = msg;
  len[7] = msg.length();
  }
  
  // works out which string in OLEDstrings to start displaying from
  for (int i=7; i>=0; i--){             // starting with the last input working up
    if (OLEDstrings[i] != ""){
      linecalc += (len[i]/21);          // sums number of lines until either...
      if(len[i]%21 != 0) linecalc++;
    }
    if(linecalc == 8) {                 // ...the right number of lines is hit (start with current string no)
      startStr = i;
      break;
    }else if(linecalc > 8){             // OR it goes over the number of lines (start with previous string no)
      startStr = i+1;
      break;
    }
  }

  //Displays array
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
//  display.println(startStr);
//  display.println(linecalc);
  for (int line=startStr; line<8; line++){
    display.println(OLEDstrings[line]);
  }
  display.display();
  delay(1);
  display.clearDisplay();
}






