#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("bem-vinda, Ellie32.");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.printf("tempo:%d\n",millis()/1000);
  delay(1000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}