/* Code for LaserLab_Dev01 board

*/
#include <SPI.h>
#include <Ethernet.h>
#include<EthernetUdp.h>
#include "Adafruit_MAX31855.h"

// Initialise Ethernet Parameters /////////////////////////////////////////////////////////////////////////
#define UDP_TX_PACKET_MAX_SIZE 150 //increase UDP size
EthernetUDP Udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_LaserLab_Dev01(169, 254, 128, 106);
char recvdBuffer[10];
int recvdPacketSize;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "LaserLab,Dev01";
String payloadStr;
int sensorValueA0 = 0; // Analog input channel sensor ADC values
int sensorValueA1 = 0;
const int Pin_Shutter_Cooler = 12;
const int Pin_Shutter_Repump = 11;
int ShutterState_Cooler = 0;
int ShutterState_Repump = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Thermocouple Variables ////////////////////////////////////////////////////////////////////////////////
#define DO  8 // Common MOSI pin
#define CLK 9 // Common serial clock pin

// Chip select pins
#define CS01  4
#define CS02  5
#define CS03  6
#define CS04  7

//THERMOCOUPLES
Adafruit_MAX31855 thermocouple01(CLK, CS01, DO);
Adafruit_MAX31855 thermocouple02(CLK, CS02, DO);
Adafruit_MAX31855 thermocouple03(CLK, CS03, DO);
Adafruit_MAX31855 thermocouple04(CLK, CS04, DO);

double temps [4]; // Buffer to hold thermocouple data
///////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {

  delay(1000); // Allow ethernet chip time to initialise after being powered up
  //Serial.begin(57600);

  //pinMode(53, OUTPUT); // Needed for slave select SPI for Mega (not connected)
  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)
  pinMode(Pin_Shutter_Cooler, INPUT);
  pinMode(Pin_Shutter_Repump, INPUT);

  Ethernet.begin(mac, IP_LaserLab_Dev01);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise

}

void loop() {

  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {

    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);

    sensorValueA0 = analogRead(A0);
    sensorValueA1 = analogRead(A1);

    // Read thermocouples
    temps[0] = thermocouple01.readCelsius();
    temps[1] = thermocouple02.readCelsius();
    temps[2] = thermocouple03.readCelsius();
    temps[3] = thermocouple04.readCelsius();

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

    payloadStr = headerStr
                 + "," + sensorValueA0
                 + "," + sensorValueA1
                 + "," + ShutterState_Cooler
                 + "," + ShutterState_Repump;

    for (int i = 0; i < 4; i++) {
      if (isnan(temps[i])) {
        payloadStr = payloadStr + "," + "0.0";
      } else {
        payloadStr = payloadStr + "," + temps[i];
      }
    }
    //Serial.println(payloadStr);
    Udp.beginPacket(IP_Collector, localPort);
    Udp.print(payloadStr);
    Udp.endPacket();

    memset(recvdBuffer, 0, sizeof(recvdBuffer));
  }


}
