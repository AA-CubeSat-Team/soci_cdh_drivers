#define UNO 0

#if UNO
  #include <SoftwareSerial.h>
  SoftwareSerial mySerial = SoftwareSerial(2,3); //2 -> slave's RX, 3 -> slave's TX
#endif

#define messageSize 16

bool writeFlag = false;
void setup() {
  #if UNO
    Serial.begin(115200);
    mySerial.begin(74880); //highest baud rate without errors
  #else //DUE: hardwareSerial
    Serial.begin(74880);
  #endif
  writeFlag = 0;
}
int len = 0;
char val[100];
void loop() {
  #if UNO
    if(mySerial.available() > 0) {
      writeFlag = true;
      val[len++] = mySerial.read();
  #else
    if (Serial.available() > 0) {
      writeFlag = true;
      val[len++] = Serial.read();
  #endif
  
  } else if (writeFlag){ //don't send when we haven't read
    if (len == messageSize) {
    #if UNO
      for(int i = 0; i < len; i++) {
        Serial.print(val[i]);
      }
      Serial.println();
    #endif
      len = 0;
      writeFlag = false; //finish writing
      char str[] = "1234567890123456";
      for (int i = 0; i < strlen(str); i++) {
        #if UNO
          mySerial.write(str[i]);
        #else
          Serial.write(str[i]);
        #endif
        delay(15);
      }
    }
  }
}
