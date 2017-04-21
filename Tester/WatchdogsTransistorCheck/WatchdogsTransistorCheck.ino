
#include <avr/wdt.h>
#include <avr/sleep.h>
void setup() {
  // put your setup code here, to run once:
  pinMode(14, OUTPUT);
  pinMode(19, OUTPUT);
  digitalWrite(19, HIGH);
  for(int i=0 ; i<5;i++){
    digitalWrite(14, HIGH);
    delay(100);
    digitalWrite(14, LOW);
    delay(100);
  }
  digitalWrite(14, HIGH);
}

void loop() {
  //digitalWrite(14, LOW);
  delay(3000);
  digitalWrite(14, HIGH);
  // put your main code here, to run repeatedly:
  wdt_reset();
  wdt_enable(WDTO_4S);  // After 1s Restart.
  set_sleep_mode(SLEEP_MODE_STANDBY); 
  digitalWrite(19, LOW);
  sleep_mode(); // System go down.

}
