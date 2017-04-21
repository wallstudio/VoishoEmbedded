
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
void setup() {
  // put your setup code here, to run once:
        Serial.begin (9600);
        pinMode(INPUT,14);
        digitalWrite(14,HIGH);
 mp3_set_serial(Serial);
  delay(1);
  mp3_set_volume(30);
}

void loop() {
  // put your main code here, to run repeatedly:
        digitalWrite(14,!digitalRead(14));
mp3_play(3);
delay(1000);
}
