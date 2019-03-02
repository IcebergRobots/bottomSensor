#include<EEPROM.h>

//MUX-Bits; B0 ist MSB; B3 ist LSB
#define B0 9
#define B1 8
#define B2 11
#define B3 10

//Analoge Ausgänge Multiplexer
#define S1 A3
#define S2 A2
#define S3 A1

#define SWITCH A6
#define BUZZER A0

#define INTERRUPT_PIN   7                      //InterruptPin
#define MUX_EN          12



byte  binPins[] = {B0, B1, B2, B3};      //die 4 Pins fuer die Multiplexer
byte  ledPins[] = {2, 3, 4, 5, 6};

int   value[48];                          //ausgelesene Werte
bool  defect[48];                          //not used sensors
bool  hit[48];                           //ausgeschlagene Sensoren
int   threshold[48];                      //Schwellwerte fuer jeden Sensor


bool  line = false;

bool state;

void setup() {
  Serial.begin(9600);

  for (int i = 2; i <= 13; i++) {     //digitale Pins auf Input
    pinMode(i, INPUT);
  }

  for (int i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(BUZZER, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  pinMode(B0, OUTPUT);
  pinMode(B1, OUTPUT);
  pinMode(B2, OUTPUT);
  pinMode(B3, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);

  pinMode(MUX_EN, OUTPUT);
  digitalWrite(MUX_EN, LOW);

  calibrate();
}


void loop() {
  measure(true);
  updateBlink(600);
  if (line) {

  }
}

void calculate() {
  int branches[8];
  for (int i = 0; i < 8; i++)
    branches[b] = 0;
  for (int b = 0; b < 8; b++) {
    for (int i = 3; i >= 0; i--) {
      if (hit[b * 4 + i])
        branches[b] =  i;
    }
  }
}

void calibrate() {
  int   maxValue[48];
  int   minValue[48];

  measure();
  for (int i = 0; i < 48; i++) {
    defect[i] = false;                         //not used sensors
    threshold[i] = value[i];                     //Schwellwerte fuer jeden Sensor
    maxValue[i] = value[i];
    minValue[i] = value[i];
  }

  unsigned long calibration_Timer = millis() + 2000;
  while (millis() < calibration_Timer) {
    updateBlink(200);
    measure(false);
    for (int i = 0; i < 48; i++) {
      if (value[i] < minValue[i])
        maxValue[i] = value[i];
      if (value[i] > maxValue[i])
        minValue[i] = value[i];
    }
  }

  for (int i = 0; i < 48; i++) {
    if (maxValue[i] - minValue[i] < 10)
      defect[i] = false;                         //not used sensors
    threshold[i] = (minValue[i] + maxValue[i] * 2) / 3
  }

}

void updateBlink(int LEDCYCLE) {
  byte on = (millis() % LEDCYCLE) * 5 / LEDCYCLE;
  for (int i = 0; i < 5; i++) {
    if (millis() % (2 * LEDCYCLE) < LEDCYCLE) {
      digitalWrite(ledPins[i], i == on);
    } else {
      digitalWrite(ledPins[i], 4 - i == on);
    }
  }
}

void measure(bool detectLine) {
  line = false;
  for (int counter = 0; counter < 16; counter++) {
    hit[counter] = false;
    hit[counter + 16] = false;
    hit[counter + 32] = false;


    String bin = String(counter, BIN);
    int binlength = bin.length();

    for (int i = 0; i < 4 - binlength; i++) {
      bin = "0" + bin;
    }

    for (int i = 0; i < 4; i++) {
      if (bin[i] == '0') state = LOW;
      if (bin[i] == '1') state = HIGH;
      digitalWrite(binPins[i], state);
    }
    delayMicroseconds(1);

    value[counter] = analogRead(S1);
    value[counter + 16] = analogRead(S2);
    value[counter + 32] = analogRead(S3);

    if (detectLine) {
      if (value[counter] <= threshold[counter]) {
        hit[counter] = true;
        line = true;
      }

      if (value[counter + 16] <= threshold[counter + 16]) {
        hit[counter + 16] = true;
        line = true;
      }
      if (value[counter + 32] <= threshold[counter + 32] && counter <= 8) {
        hit[counter + 32] = true;
        line = true;
      }
      if (line) {
        interrupt();
      }
    }

  }
}

void interrupt() {
  digitalWrite(INTERRUPT_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(INTERRUPT_PIN, LOW);
}
