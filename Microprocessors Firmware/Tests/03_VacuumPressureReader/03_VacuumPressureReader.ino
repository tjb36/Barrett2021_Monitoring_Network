/* 03_VacuumPressureReader
 * Code for reading pressure from XGS-600 Gilent gauge controller
 * T Barrett, Uni of Sussex
*/

#include <SoftwareSerial.h>

String payloadStr;

String pressure_string = ""; //Reading from the gauge
String exponent_string = ""; //Readings from the exponent
float pressure_val;
float exponent_val;
float total_val;

#define RX  11 //Define the ports for 
#define TX  12 //Software Serials

SoftwareSerial mySerial(RX, TX); // Define the Serial Port for Gauge communication

void setup() {
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  Serial.begin(9600);
}

void loop() {

    payloadStr = "";

    
    mySerial.write("#000F\r"); // Send command to controller for pressure readings
    delay(100);                // Give controller chance to respond

    pressure_string = "";
    exponent_string = "";

    // Read data transmitted from controller
    while (mySerial.available()) {
      char a  = mySerial.read();
      if (isDigit(a) || a == '.')
        pressure_string += a;
      else if (a == 'E')
        break;
      delay(10);
    }
    delay(10);

    // Read exponent of pressure
    while (mySerial.available()) {
      char a  = mySerial.read();
      if (isDigit(a))
        exponent_string += a;
      delay(10);
    }

    // Convert engineering notation into floating point for sending back
    if (pressure_string.length() > 0) {
      pressure_val = pressure_string.toFloat();
      exponent_val = exponent_string.toFloat();
      total_val = pressure_val * pow(10, -exponent_val);
      payloadStr = String(total_val, 13);
    }
    else {
      payloadStr = "0.0";
    }

    Serial.println(payloadStr);
    delay(1000);

      

}
