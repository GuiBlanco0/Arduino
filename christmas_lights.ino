// Christmas Lights Pattern
// Alternates between: all on, all off, odds on, evens on

// Define os pinos das lâmpadas (bulbs)
const int bulbs[] = {9, 10, 11, 12, 13};
const int numBulbs = 5;

void setup() {
  for (int i = 0; i < numBulbs; i++) {
    pinMode(bulbs[i], OUTPUT);
  }
}

void allOn() {
  for (int i = 0; i < numBulbs; i++) {
    digitalWrite(bulbs[i], HIGH);
  }
}

void allOff() {
  for (int i = 0; i < numBulbs; i++) {
    digitalWrite(bulbs[i], LOW);
  }
}

void oddsOn() {
  // bulbs 1,3,5 (índices 0,2,4 no array)
  for (int i = 0; i < numBulbs; i++) {
    digitalWrite(bulbs[i], (i % 2 == 0) ? HIGH : LOW);
  }
}

void evensOn() {
  // bulbs 2,4 (índices 1,3 no array)
  for (int i = 0; i < numBulbs; i++) {
    digitalWrite(bulbs[i], (i % 2 == 1) ? HIGH : LOW);
  }
}

void loop() {
  allOn();
  delay(1000);
  
  allOff();
  delay(1000);
  
  oddsOn();
  delay(1000);
  
  evensOn();
  delay(1000);
}