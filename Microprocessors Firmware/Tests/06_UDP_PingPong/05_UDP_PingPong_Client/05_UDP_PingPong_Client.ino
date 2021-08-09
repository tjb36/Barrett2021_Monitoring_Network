/* UDP Ping Pong (Client Board Code)
 *  
 * Code sends the string "Ping" to a remote server board, 
 * and then prints the reply it receives.
*/

#include <SPI.h>
#include <Ethernet.h>
#include<EthernetUdp.h>

// Initialise Ethernet Parameters ////////////////////////////
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress ipClient(169, 254, 128, 105);
IPAddress ipServer(169, 254, 128, 106);
const int localPort = 1396;
char recvdBuffer[10]; // A buffer to hold incoming strings
int recvdPacketSize;
EthernetUDP Udp;
#define UDP_TX_PACKET_MAX_SIZE 50 // Increase UDP size
String StringToServer = "Ping";
String StringFromServer;

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
  Ethernet.begin(mac, ipClient);
  Udp.begin(localPort);
  delay(1000);
}

void loop() {
  
  // Send the string to the remote server board
  Serial.print("Sending the following string to server:    ");
  Serial.println(StringToServer);
  Udp.beginPacket(ipServer, localPort);
  Udp.print(StringToServer);
  Udp.endPacket();
  delay(10); // Allow time for reply to come through

  recvdPacketSize = 0;
  recvdPacketSize = Udp.parsePacket();
  // Check if packet received, and if so read it
  if (recvdPacketSize > 0) {
    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);
    StringFromServer = String(recvdBuffer);
    Serial.print("Received the following string from server: ");
  } else {
    StringFromServer = "No string received from server";
  }
  Serial.println(StringFromServer);
  Serial.println("");

  // Clear received buffer, ready for next one
  memset(recvdBuffer, 0, sizeof(recvdBuffer));
  delay(3000);
}
