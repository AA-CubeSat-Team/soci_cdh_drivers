#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(2,3); //2 -> slave's RX, 3 -> slave's TX
#define messageSize 8

bool writeFlag = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(74880); //highest baud rate without errors
  writeFlag = 0;
}
int len = 0;
char val[100];
void loop() {
  if(mySerial.available() > 0) {
    writeFlag = true;
    val[len++] = mySerial.read();
  } else if (writeFlag){ //don't send when we haven't read
    if (len == messageSize) {
      for(int i = 0; i < len; i++) {
        Serial.print(val[i]);
      }
      Serial.println();
      len = 0;
      writeFlag = false; //finish writing
      char str[] = "somethin";
      for (int i = 0; i < strlen(str); i++) {
        mySerial.write(str[i]);
        delay(15);
      }
    }
  }
}
