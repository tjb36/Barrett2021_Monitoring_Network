/* 04_WaterSensorReader
 * Code for measuring water level.
*/


unsigned int sensorValueA0 = 0; // Analog input channel sensor ADC value

void setup() {
  Serial.begin(9600);
}

void loop() {

    sensorValueA0 = analogRead(A0);

    Serial.println(sensorValueA0);
    delay(100);

}
