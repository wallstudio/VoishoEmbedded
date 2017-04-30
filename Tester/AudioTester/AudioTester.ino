
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
void setup() {
  // put your setup code here, to run once:
        Serial.begin (9600);
        pinMode(INPUT,2);
        pinMode(INPUT,15);
        digitalWrite(2,HIGH);
        digitalWrite(15,LOW);
 mp3_set_serial(Serial);
  delay(1);
  mp3_set_volume(30);
}

bool a = true;
void loop() {
  // put your main code here, to run repeatedly:
  a=!a;
        //digitalWrite(2,a);
        //digitalWrite(15,a);
mp3_play(3);
delay(5000);
}
