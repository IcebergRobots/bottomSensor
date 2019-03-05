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

int   value[8];                          //ausgelesene Werte
bool  defect[8];                          //not used sensors
bool  hit[8];                           //ausgeschlagene Sensoren
int   threshold[8];                      //Schwellwerte fuer jeden Sensor

bool side_branches[8];
int side_angles[8] = {337, 292, 247, 202, 157, 112, 62, 22};

int angle;
int power;

bool  line = false;

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

  loadFromEEPROM();
}

void loadFromEEPROM() {
  for (int i = 0; i < 8; i++) {
    threshold[i] = EEPROM.read(i);
  }
}

void loop() {
  measure(true);
  calculate();
  if (line && power != 0) {
    send();
    digitalWriteArray(ledPins, 5, true, constrain(power - 1, 0, 4));
    delay(10);
  } else {
    digitalWriteArray(ledPins, 5, false);
  }

  if (Serial.available()) {
    if (Serial.read() == 42) {
      calibrate();
    }
  }
}

void digitalWriteArray(byte a[], int l, bool state) {
  for (int i = 0; i < l; i++) {
    digitalWrite(a[i], state);
  }
}

void digitalWriteArray(byte a[], int l, bool state, byte pwr) {
  for (int i = 0; i < l; i++) {
    if (i <= pwr) {
      digitalWrite(a[i], state);
    }
    else {
      digitalWrite(a[i], !state);
    }
  }
}

void send() {
  power = constrain(power, 0, 255);
  angle = constrain(angle, 0, 360);
  Serial.write(power);
  Serial.write(angle);
  Serial.write(angle >> 8);
}

void calculate() {
  power = 0;
  angle = -1;
  if (!line)
    return;

  int sum = 0;
  int count = 0;
  int bestBranch = 0;
  
  for (int b = 0; b < 8; b++) {
    side_branches[b] = hit[b];

    if (side_branches[b] && threshold[b] - value[b] > threshold[bestBranch] - value[bestBranch])
      bestBranch = b;

    count += side_branches[b];
    sum += side_branches[b] * side_angles[b];
  }

  angle = sum / count;
  power = count;

  if (abs(angle - side_angles[bestBranch]) > 120) {
    angle += 180;
    angle %= 360;
  }
}

void calibrate() {
  int   maxValue[8];
  int   minValue[8];

  measure(false);
  for (int i = 0; i < 8; i++) {
    defect[i] = false;                         //not used sensors
    threshold[i] = value[i];                     //Schwellwerte fuer jeden Sensor
    maxValue[i] = value[i];
    minValue[i] = value[i];
  }

  unsigned long calibration_Timer = millis() + 10000;
  while (millis() < calibration_Timer) {
    updateBlink(300);
    measure(false);
    for (int i = 0; i < 8; i++) {
      if (value[i] < minValue[i])
        minValue[i] = value[i];
      if (value[i] > maxValue[i])
        maxValue[i] = value[i];
    }
  }

  for (int i = 0; i < 8; i++) {
    if (maxValue[i] - minValue[i] < 30)
      threshold[i] = 0;                         //not used sensors
    threshold[i] = (minValue[i] + maxValue[i]) / 2;
    EEPROM.write(i, threshold[i]);
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
  bool state;
  line = false;
  for (int counter = 0; counter < 8; counter++) {
    hit[counter] = false;

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
    delayMicroseconds(5);

    value[counter] = analogRead(S3);

    if (detectLine) {
      if (value[counter] <= threshold[counter] && !defect[counter]) {
        hit[counter] = true;
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
