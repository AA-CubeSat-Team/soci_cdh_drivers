#define DUE 0

#if !DUE
  #include <SoftwareSerial.h>
  SoftwareSerial mySerial = SoftwareSerial(2,3); //2 -> slave's RX, 3 -> slave's TX
#endif

#define messageSize 4

bool writeFlag = false;
void setup() {
  #if DUE
    Serial.begin(74880); //hardware serial
  #else
    Serial.begin(115200);
    mySerial.begin(74880); //highest baud rate without errors
  #endif
  writeFlag = 0;
}
int len = 0;
char val[100];
void loop() {
  #if DUE
    if (Serial.available() > 0) {
      writeFlag = true;
      val[len++] = Serial.read();
  #else
    if(mySerial.available() > 0) {
      writeFlag = true;
      val[len++] = mySerial.read();
  #endif
  
  } else if (writeFlag){ //don't send when we haven't read
    if (len == messageSize) {
    #if !DUE //print if using UNO
      for(int i = 0; i < len; i++) {
        Serial.print(val[i]);
      }
      Serial.println();
    #endif
      len = 0;
      writeFlag = false; //finish writing
      char str[] = "some";
      for (int i = 0; i < strlen(str); i++) {
        #if DUE
          Serial.write(str[i]);
        #else
          mySerial.write(str[i]);
        #endif
        delay(15);
      }
    }
  } 
}
