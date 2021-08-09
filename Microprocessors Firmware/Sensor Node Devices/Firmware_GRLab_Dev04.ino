/* Code for GRLab Dev04 board

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
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAC };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_GRLab_Dev04(169, 254, 128, 113);
char recvdBuffer[10];
int recvdPacketSize;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "GRLab,Dev04";
String payloadStr;

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

// Setup Function /////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(1000); // Allow all chips time to initialise

  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)

  mySerial.begin(9600); // Serial port for communication with pressure gauge
  //Serial.begin(9600);   // Serial port for USB debugging
  Ethernet.begin(mac, IP_GRLab_Dev04);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {
    
    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);
    
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
    payloadStr = headerStr;

    if (pressure_string.length() > 0) {
      pressure_val = pressure_string.toFloat();
      exponent_val = exponent_string.toFloat();
      total_val = pressure_val * pow(10, -exponent_val);
      payloadStr = payloadStr + "," + String(total_val, 13);
    }
    else
      payloadStr = payloadStr + "," + "0.0";

    //payloadStr = headerStr + "," + 0.00000002;
    //Serial.print(millis());
    //Serial.print("  ");
    //Serial.println(payloadStr);
    
    Udp.beginPacket(IP_Collector, localPort);
    Udp.print(payloadStr);
    Udp.endPacket();
    memset(recvdBuffer, 0, sizeof(recvdBuffer));
  }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

