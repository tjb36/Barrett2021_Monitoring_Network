/* Code for LaserLab_Dev01 board
   IN DEVELOPMENT 03/08/2018
*/

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Adafruit_MAX31855.h"
#include <Wire.h>
#include <SoftwareSerial.h>

// Initialise Ethernet Parameters /////////////////////////////////////////////////////////////////////////
#define UDP_TX_PACKET_MAX_SIZE 150 //increase UDP size
EthernetUDP Udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_BLLab_Dev01(169, 254, 128, 110);
char recvdBuffer[10];
int recvdPacketSize;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "BlueLab,Dev01";
String payloadStr;

int sensorValueA0 = 0; // Analog input channel sensor ADC values
int sensorValueA1 = 0;
int sensorValueA2 = 0;
int sensorValueA3 = 0;
int sensorValueA4 = 0;
int sensorValueA5 = 0;

const int Pin_Shutter_Cooler = 47;
const int Pin_Shutter_Repump = 49;
int ShutterState_Cooler = 0;
int ShutterState_Repump = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Pressure Reading Variables /////////////////////////////////////////////////////////////////////////////
String pressure_string = ""; //Reading from the gauge
String exponent_string = ""; //Readings from the exponent
float pressure_val;
float exponent_val;
float total_val;

#define RX  11 //Define the ports for 
#define TX  12 //Software Serials
SoftwareSerial mySerial(RX, TX); // Define the Serial Port for Gauge communication
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Thermocouple Variables /////////////////////////////////////////////////////////////////////////////////
#define DO  43 // Common MOSI pin
#define CLK 41 // Common serial clock pin

// Chip select pins
#define CS01  33
#define CS02  31
#define CS03  29
#define CS04  27
#define CS05  25

//Thermocouples
Adafruit_MAX31855 thermocouple01(CLK, CS01, DO);
Adafruit_MAX31855 thermocouple02(CLK, CS02, DO);
Adafruit_MAX31855 thermocouple03(CLK, CS03, DO);
Adafruit_MAX31855 thermocouple04(CLK, CS04, DO);
Adafruit_MAX31855 thermocouple05(CLK, CS05, DO);

double temps [5]; // Buffer to hold thermocouple data
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Setup Function /////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(1000); // Allow all chips time to initialise

  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)
  pinMode(Pin_Shutter_Cooler, INPUT);
  pinMode(Pin_Shutter_Repump, INPUT);

  mySerial.begin(9600);
  //Serial.begin(9600);  // Baud rate set at 57600 in laser lab

  Ethernet.begin(mac, IP_BLLab_Dev01);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Loop Function //////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {

    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);

    // Read photodiodes
    sensorValueA0 = analogRead(A0);
    sensorValueA1 = analogRead(A1);
    sensorValueA2 = analogRead(A2);
    sensorValueA3 = analogRead(A3);
    sensorValueA4 = analogRead(A4);
    sensorValueA5 = analogRead(A5);

    // Read shutter states
    if (digitalRead(Pin_Shutter_Cooler) == HIGH) {
      ShutterState_Cooler = 0;
    } else {
      ShutterState_Cooler = 1;
    }

    if (digitalRead(Pin_Shutter_Repump) == HIGH) {
      ShutterState_Repump = 0;
    } else {
      ShutterState_Repump = 1;
    }

    // Read thermocouples
    temps[0] = thermocouple01.readCelsius();
    temps[1] = thermocouple02.readCelsius();
    temps[2] = thermocouple03.readCelsius();
    temps[3] = thermocouple04.readCelsius();
    temps[4] = thermocouple05.readCelsius();

    // Reading pressure gauge
    mySerial.write("#000F\r");
    delay(100);

    pressure_string = "";
    exponent_string = "";

    while (mySerial.available()) {
      char a  = mySerial.read();
      if (isDigit(a) || a == '.')
        pressure_string += a;
      else if (a == 'E')
        break;
      delay(10);
    }
    delay(10);

    while (mySerial.available()) {
      char a  = mySerial.read();
      if (isDigit(a))
        exponent_string += a;
      delay(10);
    }

    // Construct string to print, using "0.0" if NaN is returned
    payloadStr = headerStr
                 + "," + sensorValueA0
                 + "," + sensorValueA1
                 + "," + sensorValueA2
                 + "," + sensorValueA3
                 + "," + sensorValueA4
                 + "," + sensorValueA5
                 + "," + ShutterState_Cooler
                 + "," + ShutterState_Repump;

    for (int i = 0; i < 5; i++) {
      if (isnan(temps[i])) {
        payloadStr = payloadStr + "," + "0.0";
      } else {
        payloadStr = payloadStr + "," + temps[i];
      }
    }

    if (pressure_string.length() > 0) {
      pressure_val = pressure_string.toFloat();
      exponent_val = exponent_string.toFloat();
      total_val = pressure_val * pow(10, -exponent_val);
      payloadStr = payloadStr + "," + String(total_val, 13);
    }
    else
      payloadStr = payloadStr + "," + "0.0";

    //Serial.println(payloadStr);
    Udp.beginPacket(IP_Collector, localPort);
    Udp.print(payloadStr);
    Udp.endPacket();

    memset(recvdBuffer, 0, sizeof(recvdBuffer));
  }

  delay(1000);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

