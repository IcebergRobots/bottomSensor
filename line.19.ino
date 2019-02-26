#include <EEPROM.h>

#define D1 9                           //die 4 Pins fuer die Multiplexer
#define D2 8
#define D3 10
#define D0 11

#define S1 A3                           //Input der drei Multiplexer
#define S2 A2
#define S3 A1



#define INTERPIN 7

#define INTERPIN 7                      //InterruptPin
#define PLEXEN 12

byte binPins[] = {D0, D1, D2, D3};      //die 4 Pins fuer die Multiplexer
int values[48];
int counter;

void setup() {
  Serial.begin(9600);
  for (int i = 2; i <= 13; i++) {     //digitale Pins auf Input
    pinMode(i, INPUT);
  }
  pinMode(A0, OUTPUT);                 //analoge Pins auf Input
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT_PULLUP);
  pinMode(A7, INPUT);

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);

  pinMode(PLEXEN, OUTPUT);
  digitalWrite(PLEXEN, LOW);
  counter = EEPROM.read(0) + 1;
  counter %= 16;
  EEPROM.write(0, counter);
  Serial.println("Sensor Nr.: "+String(counter));
  delay(1000);

}
void loop() {
  String bin = String(counter, BIN);
  int binlength = bin.length();
  bool state;

  for (int i = 0; i <= 4 - binlength; i++) {
    bin = "0" + bin;
  }

  for (int i = 0; i <= 4; i++) {
    if (bin[i] == '0') state = LOW;
    if (bin[i] == '1') state = HIGH;
    digitalWrite(binPins[i], state);
  }

  unsigned long startT = millis() + 15000;


  while (1) {
    Serial.println(analogRead(S1));
  }
}
