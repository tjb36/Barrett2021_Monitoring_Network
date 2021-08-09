/*   04_LaserMetShutterReader
     Code for reading in the states of two LaserMet shutters

*/

const int Pin_Shutter_1 = 2;
const int Pin_Shutter_2 = 3;
int ShutterState_1 = 0;
int ShutterState_2 = 0;

void setup() {

  Serial.begin(9600);
  pinMode(Pin_Shutter_1, INPUT);
  pinMode(Pin_Shutter_2, INPUT);

}

void loop() {

  if (digitalRead(Pin_Shutter_1) == HIGH) {
    ShutterState_1 = 0;
  } else {
    ShutterState_1 = 1;
  }

  if (digitalRead(Pin_Shutter_2) == HIGH) {
    ShutterState_2 = 0;
  } else {
    ShutterState_2 = 1;
  }

  Serial.print("Shutter 1 State = ")
  Serial.print(ShutterState_1)
  Serial.print(", Shutter 2 State = ")
  Serial.println(ShutterState_2)

  delay(1000)

}

