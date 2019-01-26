
#define D0 11                       //die 4 Pins fuer die Multiplexer
#define D1 10
#define D2 9
#define D3 8

#define S1 A3                       //Input der drei Multiplexer
#define S2 A2
#define S3 A1

#define PLEXEN D12

byte binPins[] = {D0, D1, D2, D3};      //die 4 Pins fuer die Multiplexer
int value[48];                          //ausgelesene Werte
bool nus[48];                           //not used sensors
bool hit[48];                           //ausgeschlagene Sensoren
int threshold[48];                      //Schwellwerte fuer jeden Sensor
long stamp = 0;
int maxValue[48];
int minValue[48];

void setup(){
    for(int i = 2; i <= 13; i++){       //digitale Pins auf Input
        pinMode(i, INPUT);
    }
    pinMode(A0, INPUT);                 //analoge Pins auf Input
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    pinMode(A6, INPUT);
    pinMode(A7, INPUT);

    pinMode(D0, OUTPUT);                              
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);

    pinMode(S1, INPUT_PULLUP);
    pinMode(S2, INPUT_PULLUP);
    pinMode(S3, INPUT_PULLUP);
}


void loop(){
    measure();
    delay(100);
    specialPrint();
    delay(10000);
}

void measure(){
    int state;

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
            if(bin[i] == 0) state = LOW;
            if(bin[i] == 1) state = HIGH;
            digitalWrite(binPins[i], state);
        }

        value[counter] = analogRead(S1);
        value[counter + 16] = analogRead(S2);
        value[counter + 32] = analogRead(S3);

        if(value[counter] <= threshold[counter]){
            hit[counter] = true;
            interrupt();
        } 
        if(value[counter + 16] <= threshold[counter + 16]){
            hit[counter + 16] = true;
            interrupt();
        } 
        if(value[counter + 32] <= threshold[counter + 32] && counter <= 8){
            hit[counter + 16] = true;
            interrupt();
        } 

    
    }
}

calibrate(){
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

void specialPrint(){
    Serial.println("                " + 100);
    Serial.println();
    Serial.println("        " + 100 + "     " + 100 + "     " + 100);
    Serial.println();
    Serial.println(100 + "              " + 100 + "             " + 100);
    Serial.println("    " + 100 + "                     " + 100);
    Serial.println("        " + 100 + "     " + 100 + "     " + 100);
    Serial.println(100 + "  " + 100 + " " + 100 + " " + 100 + " " + 100 + " " + 100 + " " + 100 + " " + 100 + " " + 100);
    Serial.println("        " + 100 + "     " + 100 + "     " + 100);
    Serial.println("    " + 100 + "                     " + 100);
    Serial.println(100 + "              " + 100 + "             " + 100);
    Serial.println();
    Serial.println("        " + 100 + "     " + 100 + "     " + 100);
    Serial.println();
    Serial.println("                " + 100);
}
