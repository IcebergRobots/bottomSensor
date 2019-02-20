#include<EEPROM.h>

#define D0 11                           //die 4 Pins fuer die Multiplexer
#define D1 10
#define D2 9
#define D3 8

#define S1 A3                           //Input der drei Multiplexer
#define S2 A2
#define S3 A1



#define INTERPIN 7                      //InterruptPin
#define PLEXEN 12

byte binPins[] = {D0, D1, D2, D3};      //die 4 Pins fuer die Multiplexer
int value[48];                          //ausgelesene Werte
bool nus[48];                          //not used sensors
bool hit[48];                           //ausgeschlagene Sensoren
int threshold[48];                      //Schwellwerte fuer jeden Sensor
long stamp = 0;
int maxValue[48];
int minValue[48];
int state;
float v[41][2];
float direction;
float x[2];
const float pi = 3.14159265;
bool hitx = false;

void setup(){
    Serial.begin(9600);
    for(int i = 2; i <= 13; i++){       //digitale Pins auf Input
        pinMode(i, INPUT);
    }
    pinMode(A0, OUTPUT);                 //analoge Pins auf Input
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    pinMode(A6, INPUT);
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

    prom(false);
}


void loop(){
    if(Serial.available() || true){
        calibrate();
        prom(true);
    }
    measure();
    Serial.println(direction);
    /*if(hitx){
        Serial.write(map(direction, 0, 360, 0, 255));
        hitx = false;
    }*/
    /*for(int i = 0; i <= 40; i++){
        Serial.print(value[i]);
        Serial.print(", ");
    }
    Serial.println();
    Serial.println();*/
    /*for(int i = 0; i <= 40; i++){
        Serial.print(value[i]);
        Serial.print(", ");
        Serial.print(threshold[i]);
        Serial.print(",    ");
    }
    Serial.println();
    Serial.println();
    delay(1000);*/

    
}

void vAdd(){
    for(int counter = 0; counter <= 40; counter++){
        if(hit[counter]){
           x[0] = x[0] + v[counter][0];
           x[1] = x[1] + v[counter][1]; 
        }
    }
}

void measure(){

    for(int counter = 0; counter < 16; counter++){

        hit[counter] = false;
        hit[counter + 16] = false;
        hit[counter + 32] = false;


        String bin = String(counter, BIN);
        int binlength = bin.length();

        for(int i = 0; i <= 4-binlength; i++){
            bin = "0" + bin;
        }

        for(int i = 0; i <= 4; i++){
            if(bin[i] == '0') state = LOW;
            if(bin[i] == '1') state = HIGH;
            digitalWrite(binPins[i], state);
        }
        delay(1);

        value[counter] = analogRead(S1);
        value[counter + 16] = analogRead(S2);
        value[counter + 32] = analogRead(S3);

        if(value[counter] <= threshold[counter]){
            hit[counter] = true;
            hitx = true;
        } 
        if(value[counter + 16] <= threshold[counter + 16]){
            hit[counter + 16] = true;
            hitx = true;
        } 
        if(value[counter + 32] <= threshold[counter + 32] && counter <= 8){
            hit[counter + 16] = true;
            hitx = true;
        }
        if(hitx){
            interrupt();
        }

    
    }
    
    
    vAdd();
    direction = circulate(atan2(x[1], x[0]), 0, 359);
}

void calibrate(){
    for(int counter = 0; counter < 16; counter++){
            
        String bin = String(counter, BIN);
        int binlength = bin.length();

        for(int i = 0; i <= 4-binlength; i++){
        bin = "0" + bin;
        }

        for(int i = 0; i <= 4; i++){
            if(bin[i] == 0) state = LOW;
            if(bin[i] == 1) state = HIGH;
            digitalWrite(binPins[i], state);
        }

        value[counter] = analogRead(S1);
        value[counter + 16] = analogRead(S2);
        value[counter + 32] = analogRead(S3);

        maxValue[counter] = value[counter];
        minValue[counter] = value[counter];

        maxValue[counter + 16] = value[counter + 16];
        minValue[counter + 16] = value[counter + 16];

        maxValue[counter + 32] = value[counter + 32];
        maxValue[counter + 32] = value[counter + 32];
    }

    stamp = millis() + 10000;
    while(millis() < stamp){
        for(int counter = 0; counter < 16; counter++){
            
            String bin = String(counter, BIN);
            int binlength = bin.length();

            for(int i = 0; i <= 4-binlength; i++){
            bin = "0" + bin;
            }

            for(int i = 0; i <= 4; i++){
                if(bin[i] == 0) state = LOW;
                if(bin[i] == 1) state = HIGH;
                digitalWrite(binPins[i], state);
            }

            value[counter] = analogRead(S1);
            value[counter + 16] = analogRead(S2);
            value[counter + 32] = analogRead(S3);

            if(value[counter] > maxValue[counter])maxValue[counter] = value[counter];
            if(value[counter] < minValue[counter])minValue[counter] = value[counter];

            if(value[counter + 16] > maxValue[counter + 16])maxValue[counter + 16] = value[counter + 16];
            if(value[counter + 16] < minValue[counter + 16])minValue[counter + 16] = value[counter + 16];

            if(value[counter + 32] > maxValue[counter + 32])maxValue[counter + 32] = value[counter + 32];
            if(value[counter + 32] > maxValue[counter + 32])maxValue[counter + 32] = value[counter + 32];
        }
    }


    for(int counter = 0; counter < 16; counter ++){
        threshold[counter] = (maxValue[counter] + minValue[counter])/2;
        threshold[counter + 16] = (maxValue[counter + 16] + minValue[counter + 16])/2;
        threshold[counter + 32] = (maxValue[counter + 32] + minValue[counter + 32])/2;
    }
}

void interrupt(){
    digitalWrite(INTERPIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(INTERPIN, LOW);
}

void vectorToSensor(){
    v[0][0] = 0;
    v[0][1] = 1;
    v[1][0] = 0;
    v[1][1] = 2;
    v[2][0] = 0;
    v[2][1] = 3;
    v[3][0] = 0;
    v[3][1] = 4;
    v[4][0] = 0,7071;
    v[4][1] = 0,7071;
    v[5][0] = 1,4142;
    v[5][1] = 1,4142;
    v[6][0] = 2,1213;
    v[6][1] = 2,1213;
    v[7][0] = 2,8284;
    v[7][1] = 2,8284;
    v[8][0] = 1;
    v[8][1] = 0;
    v[9][0] = 2;
    v[9][1] = 0;
    v[10][0] = 3;
    v[10][1] = 0;
    v[11][0] = 4;
    v[11][1] = 0;
    v[12][0] = 0,7071;
    v[12][1] = -0,7071;
    v[13][0] = 1,4142;
    v[13][1] = -1,4142;
    v[14][0] = 2,1213;
    v[14][1] = -2,1213;
    v[15][0] = 2,8284;
    v[15][1] = -2,8284;
    v[16][0] = 0;
    v[16][1] = -1;
    v[17][0] = 0;
    v[17][1] = -2;
    v[18][0] = 0;
    v[18][1] = -3;
    v[19][0] = 0;
    v[19][1] = -4;
    v[20][0] = -0,7071;
    v[20][1] = -0,7071;
    v[21][0] = -1,4142;
    v[21][1] = -1,4142;
    v[22][0] = -2,1213;
    v[22][1] = -2,1213;
    v[23][0] = -2,8284;
    v[23][1] = -2,8284;
    v[24][0] = -1;
    v[24][1] = 0;
    v[25][0] = -2;
    v[25][1] = 0;
    v[26][0] = -3;
    v[26][1] = 0;
    v[27][0] = -4;
    v[27][1] = 0;
    v[28][0] = -0,7071;
    v[28][1] = 0,7071;
    v[29][0] = -1,4142;
    v[29][1] = 1,4142;
    v[30][0] = -2,1213;
    v[30][1] = 2,1213;
    v[31][0] = -2,8284;
    v[31][1] = 2,8284;
    v[32][0] = 1,5307;
    v[32][1] = 3,6955;
    v[33][0] = 3,6955;
    v[33][1] = 1,5307;
    v[34][0] = 3,6955;
    v[34][1] = -1,5307;
    v[35][0] = 1,5307;
    v[35][1] = -3,6955;
    v[36][0] = -1,5307;
    v[36][1] = -3,6955;
    v[37][0] = -3,6955;
    v[37][1] = -1,5307;
    v[38][0] = -3,6955;
    v[38][1] = 1,5307;
    v[39][0] = -1,5307;
    v[39][1] = 3,6955;
    v[40][0] = 0;
    v[40][1] = 0;
}

int circulate(int value, int min, int max) {
  max = max(min, max - min + 1);
  value -= min;
  value %= max;
  if (value < 0) value += max;
  value += min;
  return value;
}

void prom(boolean write){                               //wenn EEPROM ueberschrieben werden soll true
    if(!write){
        for(int i = 0; i <= 16; i++){
            threshold[i] = (EEPROM.read(i) * 4);
            threshold[i + 16] = (EEPROM.read(i + 16) * 4);
            threshold[i + 32] = (EEPROM.read(i + 32) * 4);
        }   
    }else if(write){
        for(int i = 0; i <= 16; i++){
            EEPROM.write(i, threshold[i]);
            EEPROM.write(i + 16, threshold[i] / 4);
            EEPROM.write(i + 32, threshold[i] / 4);
        }
    }
}

/*void specialPrint(){
    Serial.println("                " + value[3]);
    Serial.println();
    Serial.println("        " + value[39] + "     " + value[2] + "     " + value[32]);
    Serial.println();
    Serial.println(value[31] + "              " + value[1] + "             " + value[7]);
    Serial.println("        " + value[29] + "     " + value[0] + "     " + value[5]);
    Serial.println("    " + value[28] + "       " + value[4]);
    Serial.println(value[27] + "  " + value[26] + " " + value[25] + " " + value[24] + " " + value[40] + " " + value[8] + " " + value[9] + " " + value[10] + " " + value[11]);
    Serial.println("    " + value[20] + "       " + value[12]);
    Serial.println("        " + value[21] + "     " + value[16] + "     " + value[13]);
    Serial.println(value[37] + "              " + value[17] + "             " + value[34]);
    Serial.println();
    Serial.println("        " + value[22] + "     " + value[18] + "     " + value[14]);
    Serial.println();
    Serial.println("                " + value[19]);
}*/
