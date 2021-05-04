#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(2,3);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(19200);
}
char val;
char values[32];
int i = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if(mySerial.available() > 0) {
    val = mySerial.read();
    Serial.print(val);
    Serial.print(" ");
//    mySerial.write(val);
  } else if (i == 0){
      i = 1;
      mySerial.write("something constant");
  }
//  mySerial.write(values);
//  i=0;
}
