#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(2,3); //2 -> slave's RX, 3 -> slave's TX
int i = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(74880); //highest baud rate without errors
  i = 0;
}
char val;
void loop() {
  if(mySerial.available() > 0) {
    i = 1;
    val = mySerial.read();
    Serial.print(val);
  } else if (i == 1){ //don't want to send in the beginning
      i = 0;
      Serial.println();
      mySerial.write("some");
  }
}
