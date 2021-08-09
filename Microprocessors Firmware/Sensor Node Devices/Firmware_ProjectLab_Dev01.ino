/* Code for ProjectLab Dev01 board
 * T Barrett, Uni of Sussex

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
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAF };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_ProjectLab_Dev01(169, 254, 128, 112);
char recvdBuffer[10];
int recvdPacketSize;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "ProjectLab,Dev01";
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

// Thermocouple Variables /////////////////////////////////////////////////////////////////////////////////
#define DO  43 // Common MOSI pin
#define CLK 41 // Common serial clock pin

// Chip select pins
#define CS01  23
#define CS02  25
#define CS03  27
#define CS04  29
#define CS05  31
#define CS06  33
#define CS07  35
#define CS08  37

//Thermocouples
Adafruit_MAX31855 thermocouple01(CLK, CS01, DO);
Adafruit_MAX31855 thermocouple02(CLK, CS02, DO);
Adafruit_MAX31855 thermocouple03(CLK, CS03, DO);
Adafruit_MAX31855 thermocouple04(CLK, CS04, DO);
Adafruit_MAX31855 thermocouple05(CLK, CS05, DO);
Adafruit_MAX31855 thermocouple06(CLK, CS06, DO);
Adafruit_MAX31855 thermocouple07(CLK, CS07, DO);
Adafruit_MAX31855 thermocouple08(CLK, CS08, DO);


double temps [8]; // Buffer to hold thermocouple data
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Setup Function /////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(1000); // Allow all chips time to initialise

  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)

  mySerial.begin(9600); // Serial port for communication with pressure gauge
  //Serial.begin(9600);   // Serial port for USB debugging
  Ethernet.begin(mac, IP_ProjectLab_Dev01);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {
    
    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);
    
    // Read thermocouples
    temps[0] = thermocouple01.readCelsius();
    temps[1] = thermocouple02.readCelsius();
    temps[2] = thermocouple03.readCelsius();
    temps[3] = thermocouple04.readCelsius();
    temps[4] = thermocouple05.readCelsius();
    temps[5] = thermocouple06.readCelsius();
    temps[6] = thermocouple07.readCelsius();
    temps[7] = thermocouple08.readCelsius();
    
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

    for (int i = 0; i < 8; i++) {
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

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

