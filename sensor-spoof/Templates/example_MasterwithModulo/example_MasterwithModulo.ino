// Master Code Arduino Uno

#include <Wire.h>
#define SLAVE_ADDR 4
#define ANSWERSIZE 2

//String sentence = "abcdefghijklmnopqrstuvwxyz123456";
String sentence = "abcdefghijklmnopqrstuvwxyz123456    abcdefghijklmnopqrstuvwxyz123456\0";
void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(9600);
}

byte x = 0;

void loop()
{
  delay(100);
  char test[sentence.length()];
  sentence.toCharArray(test, sentence.length());

  int size = sentence.length();

  for (int i = 0; i < size; i++) {
    if (i % 32 == 0) {
      Wire.beginTransmission();
    }
    Wire.write(test[i]);
    if (test[i] % 32 == 31 || i == size - 1)
      Wire.endTransmission();
  }
  // Begin transmission every 32 characters
  // Write character
  // End transmission to send characters at max capacity OR when the last character is provided




  Wire.requestFrom(SLAVE_ADDR, ANSWERSIZE);
  String response = "";
  while (Wire.available()) {
    char b = Wire.read();
    response += b;
  }
  //  Serial.println(response);
}
