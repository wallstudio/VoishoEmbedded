
#include <avr/wdt.h>
#include <avr/sleep.h>

int i=0;
void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  for(int i=0;i<12;i++){
    digitalWrite(2,HIGH);
    delay(50);
    digitalWrite(2,LOW);
    delay(50);
  }
  digitalWrite(2,HIGH);
  delay(1000);
  wdt_enable(WDTO_4S);
  digitalWrite(2,LOW);
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_mode();
  delay(100);
}
void loop() {
  // put your main code here, to run repeatedly:
}
