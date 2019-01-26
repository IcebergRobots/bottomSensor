
#define D0 11                       //die 4 Pins fuer die Multiplexer
#define D1 10
#define D2 9
#define D3 8

#define S1 A3                       //Input der drei Multiplexer
#define S2 A2
#define S3 A1

#define PLEXEN D12

byte binPins[] = {D0, D1, D2, D3};  //die 4 Pins fuer die Multiplexer
int value[]  = value{};             //ausgelesene Werte
bool nus[] = nus[];                 //not used sensors

void setup(){
    for(int i = 2; i <= 13; i++){   //digitale Pins auf Input
        pinMode(i, INPUT);
    }
    pinMode(A0, INPUT);             //analoge Pins auf Input
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

    for(int counter = 0; counter <= 16; counter++){
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

    }
}

void specialPrint(){
  
}
