
#include <avr/wdt.h>
#include <avr/sleep.h>

int i=0;
void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
  delay(5000);
  wdt_enable(WDTO_8S);
  digitalWrite(2,HIGH);
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_mode();
  delay(10000);
}
void loop() {
  // put your main code here, to run repeatedly:
}
