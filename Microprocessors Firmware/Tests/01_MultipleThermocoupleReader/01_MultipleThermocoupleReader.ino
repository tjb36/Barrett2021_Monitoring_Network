/* 01_MultipleThermocoupleRead
 * Code for measuring multiple temperatures using MAX31855 chips.
*/

#include<SPI.h>
#include "Adafruit_MAX31855.h"

#define DO  2 // Common MOSI pin
#define CLK 3 // Common serial clock pin

// Chip select pins
#define CS01  4
#define CS02  5
#define CS03  6
#define CS04  7
#define CS05  8
#define CS06  9

//THERMOCOUPLES
Adafruit_MAX31855 thermocouple01(CLK, CS01, DO);
Adafruit_MAX31855 thermocouple02(CLK, CS02, DO);
Adafruit_MAX31855 thermocouple03(CLK, CS03, DO);
Adafruit_MAX31855 thermocouple04(CLK, CS04, DO);
Adafruit_MAX31855 thermocouple05(CLK, CS05, DO);
Adafruit_MAX31855 thermocouple06(CLK, CS06, DO);
//

String payloadStr;
double temps [6]; // Buffer to hold thermocouple data

void setup() {
  Serial.begin(9600);
}

void loop() {

  // Read thermocouples
  temps[0] = thermocouple01.readCelsius();
  delay(100);
  temps[1] = thermocouple02.readCelsius();
  delay(100);
  temps[2] = thermocouple03.readCelsius();
  delay(100);
  temps[3] = thermocouple04.readCelsius();
  delay(100);
  temps[4] = thermocouple05.readCelsius();
  delay(100);
  temps[5] = thermocouple06.readCelsius();
  delay(100);

  // Construct string to print, using "0.0" if NaN is returned
  payloadStr = "";
  if (isnan(temps[0])) {
    payloadStr = payloadStr + "0.0";
  } else {
    payloadStr = payloadStr + temps[0];
  }
  for (int i = 1; i < 6; i++) {
    if (isnan(temps[i])) {
      payloadStr = payloadStr + "," + "0.0";
    } else {
      payloadStr = payloadStr + "," + temps[i];
    }
  }
  
  Serial.print(millis());
  Serial.print(",");
  Serial.println(payloadStr);

}
