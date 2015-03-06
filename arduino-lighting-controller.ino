#include <SPI.h>

// pin definitions
#define CLEARPIN 4    // master clear for 74HC595 shift registers
#define LATCHPIN 5    // latch for 74HC595 shift registers
#define OEPIN    6    // output enable for 74HC595 shift registers
#define ARMEDPIN 7    // optoisolator connected to load power
#define DATAPIN  11   // data for 74HC595 shift registers
#define CLOCKPIN 13   // clock for 74HC595 shift registers 

#define bitFlip(x,n)  bitRead(x,n) ? bitClear(x,n) : bitSet(x,n)

// how long to keep fire active per message, in ms
#define FIRETIME 100

char c;
byte r1 = 0, r2 = 0;
unsigned long t[16];

void setup() {
  // set all output pins
  SPI.begin(); // handles DATAPIN and CLOCKPIN
  pinMode(LATCHPIN, OUTPUT);
  pinMode(OEPIN, OUTPUT);
  pinMode(CLEARPIN, OUTPUT);

  // make sure no lines go active until data is shifted out
  digitalWrite(CLEARPIN, HIGH);
  digitalWrite(OEPIN, LOW);

  // clear any lines that were left active
  digitalWrite(LATCHPIN, LOW);
  digitalWrite(OEPIN, HIGH);
  c = SPI.transfer(0);
  c = SPI.transfer(0);
  digitalWrite(LATCHPIN, HIGH);
  digitalWrite(OEPIN, LOW);

  // activate built-in pull-up resistor
  digitalWrite(ARMEDPIN, HIGH);

  // ignore the WiFly radio start text
  delay(5000);

  // start serial communication with the WiFly
  Serial.begin(115200);

  // initialize the expiration time array
  for(c=0;c<16;c++) {
    t[c] = 0;
  }
}

void loop() {
  for(c=0;c<8;c++) {
    if(t[c] > millis()) {
      bitSet(r1,c);
    }
    else {
      bitClear(r1,c);
    }
  }
  
  for(c=0;c<8;c++) {
    if(t[8+c] > millis()) {
      bitSet(r2,c);
    }
    else {
      bitClear(r2,c);
    }
  }

  digitalWrite(LATCHPIN, LOW);
  digitalWrite(OEPIN, HIGH);
  c = SPI.transfer(r2);
  c = SPI.transfer(r1);
  digitalWrite(LATCHPIN, HIGH);
  digitalWrite(OEPIN, LOW);

  if(Serial.available() > 1) {
    c = Serial.read();
    if( c == 0x01 ) {
      c = Serial.read();
      switch(c) {
        case '0' : t[0] = millis() + FIRETIME; break;
        case '1' : t[1] = millis() + FIRETIME; break;
        case '2' : t[2] = millis() + FIRETIME; break;
        case '3' : t[3] = millis() + FIRETIME; break;
        case '4' : t[4] = millis() + FIRETIME; break;
        case '5' : t[5] = millis() + FIRETIME; break;
        case '6' : t[6] = millis() + FIRETIME; break;
        case '7' : t[7] = millis() + FIRETIME; break;
        case '8' : t[8] = millis() + FIRETIME; break;
        case '9' : t[9] = millis() + FIRETIME; break;
        case 'a' : case 'A' : t[10] = millis() + FIRETIME; break;
        case 'b' : case 'B' : t[11] = millis() + FIRETIME; break;
        case 'c' : case 'C' : t[12] = millis() + FIRETIME; break;
        case 'd' : case 'D' : t[13] = millis() + FIRETIME; break;
        case 'e' : case 'E' : t[14] = millis() + FIRETIME; break;
        case 'f' : case 'F' : t[15] = millis() + FIRETIME; break;
        case '?' :
          // load power on = low, off = high
          digitalRead(ARMEDPIN) ? Serial.print("-") : Serial.print("+");
          break;
      }
    }
  }
}
