
#define I0 14
#define I1 15
#define I2 16

void setup() {
  Serial.begin(9600);

  pinMode(I0,INPUT_PULLUP);
  pinMode(I1,INPUT_PULLUP);
  pinMode(I2,INPUT_PULLUP);
}

void loop() { 
  if ( digitalRead(I0) == LOW )  {      // If anything comes in Serial (USB),
    Serial.write("A");   // read it and send it out Serial1 (pins 0 & 1)
  }
  if (digitalRead(I1)== LOW) {      // If anything comes in Serial (USB),
    Serial.write("B");   // read it and send it out Serial1 (pins 0 & 1)
  }
  if (digitalRead(I2)== LOW) {      // If anything comes in Serial (USB),
    Serial.write("C");   // read it and send it out Serial1 (pins 0 & 1)
  }
  delay(500);
}
