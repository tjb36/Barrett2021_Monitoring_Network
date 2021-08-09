/* Code for MainCorridor_Dev01 board

*/
#include <SPI.h>
#include <Ethernet2.h>

// Initialise Ethernet Parameters /////////////////////////////////////////////////////////////////////////
#define UDP_TX_PACKET_MAX_SIZE 150 //increase UDP size
EthernetUDP Udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAA };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
IPAddress IP_MainCorr_Dev01(169, 254, 128, 111);
char recvdBuffer[10];
int recvdPacketSize;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
String headerStr = "MainCorr,Dev01";
String payloadStr;
int sensorValueA0 = 0;

void setup() {

  //Serial.begin(57600);

  delay(1000); // Allow ethernet chip time to initialise after being powered up
  pinMode(10, OUTPUT); // SPI SS pin for most Arduino models (need to connect)
  Ethernet.begin(mac, IP_MainCorr_Dev01);
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise
}

void loop() {

  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {

    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);

    sensorValueA0 = analogRead(A0);

    payloadStr = headerStr + "," + sensorValueA0;

    Serial.println(payloadStr);
    Udp.beginPacket(IP_Collector, localPort);
    Udp.print(payloadStr);
    Udp.endPacket();

    memset(recvdBuffer, 0, sizeof(recvdBuffer));
  }
}

