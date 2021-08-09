/* 02_MultipleAnalogVoltageReadr
 * Code for measuring multiple analog voltages.
*/


unsigned int sensorValueA0 = 0; // Analog input channel sensor ADC values
unsigned int sensorValueA1 = 0;
unsigned int sensorValueA2 = 0;
unsigned int sensorValueA3 = 0;
unsigned int sensorValueA4 = 0;
unsigned int sensorValueA5 = 0;
String payloadStr;

void setup() {
  Serial.begin(9600);
}

void loop() {

    sensorValueA0 = analogRead(A0);
    sensorValueA1 = analogRead(A1);
    sensorValueA2 = analogRead(A2);
    sensorValueA3 = analogRead(A3);
    sensorValueA4 = analogRead(A4);
    sensorValueA5 = analogRead(A5);
    payloadStr = "";
    payloadStr = payloadStr + sensorValueA0 + "," + sensorValueA1 
                 + "," + sensorValueA2 + "," + sensorValueA3 + "," 
                 + sensorValueA4 + "," + sensorValueA5;
    Serial.print("Analog Voltages: ");
    Serial.println(payloadStr);
    delay(100);

}
