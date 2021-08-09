/* UDP Ping Pong (Server Board Code)
 *  
 * Code receives a string from a remote client board, 
 * and then sends back the string "Pong".
*/

#include <SPI.h>
#include <Ethernet.h>
#include<EthernetUdp.h>

// Initialise Ethernet Parameters ////////////////////////////
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ipClient(169, 254, 128, 105);
IPAddress ipServer(169, 254, 128, 106);
const int localPort = 1396;
char recvdBuffer[10];
int recvdPacketSize;
EthernetUDP Udp;
#define UDP_TX_PACKET_MAX_SIZE 50 // Increase UDP size

String StringToClient = "Pong";
String StringFromClient;

void setup() {
  
  delay(1000);
  while (!Serial) {}
  Serial.begin(9600);

  // Pin 10 should be kept as output for CS, even when using using MEGA (10 is 
  // defined in Ethernet.h library already). Pin 53 should still be set as 
  // output if using a Mega, for SPI to work properly.
  pinMode(10, OUTPUT);
  pinMode(53, OUTPUT); 

  // Begin ethernet and UDP
  Ethernet.begin(mac, ipServer);
  Udp.begin(localPort);
  delay(1000);
}

void loop() {

  recvdPacketSize = Udp.parsePacket();

  if (recvdPacketSize > 0) {
    
    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);
    StringFromClient = String(recvdBuffer);
    Serial.print("Received the following string from client: ");
    Serial.println(StringFromClient);

    Serial.print("Sending the following string to client:    ");
    Serial.println(StringToClient);
    Serial.println("");
    
    Udp.beginPacket(ipClient, localPort);
    Udp.print(StringToClient);
    Udp.endPacket();
    
    memset(recvdBuffer, 0, sizeof(recvdBuffer));
  }
}
