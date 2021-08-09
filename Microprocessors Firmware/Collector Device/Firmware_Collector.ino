/* Code for client board

*/
#include <SPI.h>
#include <Ethernet.h>
#include<EthernetUdp.h>
#include <utility/w5100.h>

// Initialise Ethernet Parameters /////////////////////////////////////////////////////////////////////////
#define UDP_TX_PACKET_MAX_SIZE 150 //increase UDP size
EthernetUDP Udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
const unsigned int localPort = 1396;
IPAddress IP_Collector(169, 254, 128, 105);
const int NumberOfServers = 10;
IPAddress Server_IPs_List[NumberOfServers] = { IPAddress(169, 254, 128, 106),  //LaserLab,Dev01 IP address
                                               IPAddress(169, 254, 128, 107),  //GRLab,Dev02 IP address
                                               IPAddress(169, 254, 128, 108),  //GRLab,Dev03 IP address
                                               IPAddress(169, 254, 128, 109),  //GRLab,Dev01 IP address
                                               IPAddress(169, 254, 128, 110),  //BLLab,Dev01 IP address
                                               IPAddress(169, 254, 128, 111),  //MainCorr,Dev01 IP address
                                               IPAddress(169, 254, 128, 112),  //ProjectLab,Dev01 IP address
                                               IPAddress(169, 254, 128, 113),  //GRLab,Dev04 IP address
                                               IPAddress(169, 254, 128, 114),  //BakeoutTent,Dev01 IP address
                                               IPAddress(169, 254, 128, 115)   //OYLab,Dev01 IP address
                                             };


char recvdBuffer[100];
int recvdPacketSize;
String answer;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialise Variables ///////////////////////////////////////////////////////////////////////////////////
const unsigned long samplingInterval = 20000; // How often to take a data sample in ms
unsigned long previousSampledMillis = 0;      // Time at which previous sample was acquired
unsigned long currentMillis = 0;              // Current time elapsed since program started in ms
unsigned long zeroRelativeMillis = 0;         // Zero of relative time
unsigned long currentRelativeMillis = 0;      // Relative time after resetting the zeroRelativeMillis variable
const int timeoutDuration = 1500;             //Timeout duration when waiting for UDP replay from a remote server
unsigned long timeoutMillis;                  //Timer variable for timeout
unsigned long initialTimeoutMillis;           //Timer variable for timeout
int healthVector[NumberOfServers];
String headerStr = "SYSTEM,Collector";
String payloadStr;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  delay(1000); // Allow ethernet chip time to initialise after being powered up
  Serial.begin(57600);

  pinMode(10, OUTPUT); // Needed for slave select SPI for Mega (not connected)
  pinMode(11, OUTPUT); // SPI SS pin for most Arduino models (need to connect)
  pinMode(12, OUTPUT); // SPI SS pin for most Arduino models (need to connect)
  pinMode(13, OUTPUT); // SPI SS pin for most Arduino models (need to connect)

  Ethernet.begin(mac, IP_Collector);
  W5100.setRetransmissionCount(4);  // Number of retries when sending UDP packet
  W5100.setRetransmissionTime(100); // Time for transmission of UDP packet (default 2000 = 200ms)
  Udp.begin(localPort);
  delay(1000); // Allow ethernet chip time to initialise

  zeroRelativeMillis = millis();

}

void loop() {

  currentMillis = millis();
  currentRelativeMillis = currentMillis - zeroRelativeMillis;

  // When sampling interval has elapsed, go into this loop and take data points ////////////////////////////
  if ((currentRelativeMillis - previousSampledMillis) >= samplingInterval) {

    payloadStr = headerStr;
    
    // Loop over all server IP addresses ///////////////////////////////////////////////////////////////////
    for (int k = 0; k < NumberOfServers; k++) {

      // Read any packets available in buffer to flush first ///////////////////////////////////////////////
      recvdPacketSize = Udp.parsePacket();
      if (recvdPacketSize > 0) {
        Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);
        memset(recvdBuffer, 0, sizeof(recvdBuffer));
      }

      // Send a message to the remote server ///////////////////////////////////////////////////////////////
      Udp.beginPacket(Server_IPs_List[k], localPort);
      Udp.print("READ");
      Udp.endPacket();

      // Keep checking if a reply is received until timeout duration has elapsed //////////////////////////
      initialTimeoutMillis = millis();
      timeoutMillis = initialTimeoutMillis;
      recvdPacketSize = 0;
      while ((timeoutMillis - initialTimeoutMillis <= timeoutDuration) && (recvdPacketSize == 0)) {
        recvdPacketSize = Udp.parsePacket();
        timeoutMillis = millis();
      }

      // Check if timout occured - if not, a packet was received and so go ahead and read it /////////////
      if (recvdPacketSize > 0) {
        Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);
        answer = String(recvdBuffer);
        healthVector[k] = 1;
      } else {
        answer = "Timeout";
        healthVector[k] = 0;
      }

      payloadStr = payloadStr + "," + healthVector[k];

      Serial.println(answer);
      memset(recvdBuffer, 0, sizeof(recvdBuffer)); // Reset received buffer ready for next data point
    }

    delay(100);
    Serial.println(payloadStr);
    

    previousSampledMillis += samplingInterval;
  }

}
