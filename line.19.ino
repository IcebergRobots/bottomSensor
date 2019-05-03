#include<EEPROM.h>

//MUX-Bits; D0 ist MSB; D3 ist LSB
#define D0 9
#define D1 8
#define D2 10
#define D3 11

//Analoge Ausgänge Multiplexer
#define S1 A3
#define S2 A2
#define S3 A1

#define SWITCH A6
#define BUZZER A0

#define INTERRUPT_PIN   7                      //InterruptPin
#define MUX_EN          12

byte  binPins[] = {D0, D1, D2, D3};      //die 4 Pins fuer die Multiplexer
byte  ledPins[] = {2, 3, 4, 5, 6};

int   value[48];                          //ausgelesene Werte
bool  defect[48];                          //not used sensors
bool  hit[48];                           //ausgeschlagene Sensoren
int   threshold[48];                      //Schwellwerte fuer jeden Sensor

int sendValue = -1;

int branches[8];
int gaps[8];
int angles[8] = {0, 315, 270, 225, 180, 135, 90, 45};

bool side_branches[8];
int side_angles[8] = {337, 292, 247, 202, 157, 112, 62, 22};

int angle;
int power;

bool  line = false;

void setup() {
  Serial.begin(115200);

  for (int i = 2; i <= 13; i++) {     //digitale Pins auf Input
    pinMode(i, INPUT);
  }

  for (int i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(BUZZER, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  pinMode(INTERRUPT_PIN, OUTPUT);


  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);

  pinMode(MUX_EN, OUTPUT);
  digitalWrite(MUX_EN, LOW);

  loadFromEEPROM();

  if((analogRead(SWITCH) > 500)){

  }else{
    calibrate();
  }
}

void loadFromEEPROM() {
  for (int i = 0; i < 48; i++) {
    threshold[i] = EEPROM.read(i);
  }
}

void loop() {
  if((analogRead(SWITCH) > 500)){

    measure(true);
    calculate();

    if (line && power != 0) {
      interrupt();
      send();
      digitalWriteArray(ledPins, 5, true, constrain(power-1,0,4));
      delay(10);
    }else{
      digitalWriteArray(ledPins, 5, false);
      sendValue = -1;
    }

    if (Serial.available()) {
      if (Serial.read() == 42) {
        calibrate();
      }
    }

  }else{
    updateBlink(600);
    digitalWrite(BUZZER, 0);
  }
}

void digitalWriteArray(byte a[], int l, bool state){
  for(int i = 0; i<l; i++){
    digitalWrite(a[i], state);
  }
}

void digitalWriteArray(byte a[], int l, bool state, byte pwr){
  for(int i = 0; i<l; i++){
    if(i<=pwr){
      digitalWrite(a[i], state);
    }
    else{
      digitalWrite(a[i], !state);
    }
  }
}

void send() {
  power = constrain(power, 0, 255);
  angle = constrain(angle, 0, 360);
  if(sendValue == -1){
    sendValue = angle;
  }
  byte output = ((byte)sendValue/6) << 2;
  output |= constrain((power-1)/2, 0, 3) & B00000011;
  Serial.write(output);
}

void calculate() {
  //angles zurücksetzen
  for(int i = 0; i<8; i++){
    angles[i] %= 360;
  }

  power = 0;
  angle = -1;
  if (!line)
    return;
  int sum = 0;
  int count = 0;

  //Branches zurücksetzen
  for (int i = 0; i < 8; i++)
    branches[i] = 0;

  //Branchwerte berechnen
  for (int b = 0; b < 8; b++) {
    for (int i = 3; i >= 0; i--) {
      if (hit[b * 4 + i])
        branches[b] =  4 - i;
    }
    //side_branches[b] = hit[32 + b];    
  }

  //Gaps zuruecksetzen
  for(int i=0; i<8; i++){
    gaps[i] = 0;
  }

  //groesste Gap ermitteln
  byte maxVal = 0;
  for(int i=0; i<16; i++){
    if(branches[i%8] == 0){
       gaps[i%8]++;
       if(i != 0){
         gaps[i%8] += gaps[(i-1)%8];
       }
       if(gaps[i%8] > gaps[maxVal]){
          maxVal = i%8;
        }
    }
  }

  //Wertesprung zu Gap verschieben
  for(int i = 0; i < maxVal; i++){
    angles[i] += 360;
  }
  

  //Durchschnitt bilden
  for(int b = 0; b < 8; b++){
    count += branches[b];
    //count += side_branches[b];
    sum += branches[b] * angles[b];
    //sum += side_branches[b] * side_angles[b];
  }
  angle = sum / count;

  angle %= 360;
  power = count;

  if (hit[40])
    power += 5;  
}

void calibrate() {
  int   maxValue[48];
  int   minValue[48];

  measure(false);
  for (int i = 0; i < 48; i++) {
    defect[i] = false;                         //not used sensors
    threshold[i] = value[i];                     //Schwellwerte fuer jeden Sensor
    maxValue[i] = value[i];
    minValue[i] = value[i];
  }

  unsigned long calibration_Timer = millis() + 10000;
  while (millis() < calibration_Timer) {
    updateBlink(300);
    measure(false);
    for (int i = 0; i < 48; i++) {
      if (value[i] < minValue[i])
        minValue[i] = value[i];
      if (value[i] > maxValue[i])
        maxValue[i] = value[i];
    }
  }

  for (int i = 0; i < 48; i++) {
    threshold[i] = (2*minValue[i] + maxValue[i]) / 3;
    if (maxValue[i] - minValue[i] < 50)
      threshold[i] = 0;                         //not used sensors
    EEPROM.write(i, threshold[i]);
  }

  while(Serial.available()){
    Serial.read();
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
      if (value[counter] <= threshold[counter] && !defect[counter]) {
        hit[counter] = true;
        line = true;
      }

      if (value[counter + 16] <= threshold[counter + 16] && !defect[counter + 16]) {
        hit[counter + 16] = true;
        line = true;
      }
      if (value[counter + 32] <= threshold[counter + 32] && counter < 8 && !defect[counter + 32]) {
        hit[counter + 32] = true;
        line = true;
      }
    }

  }
}

void interrupt() {
  digitalWrite(INTERRUPT_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(INTERRUPT_PIN, LOW);
  delayMicroseconds(10);
}
