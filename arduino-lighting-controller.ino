#include <SPI.h>

// pin definitions
#define CLEARPIN 4    // master clear for 74HC595 shift registers
#define LATCHPIN 5    // latch for 74HC595 shift registers
#define OEPIN    6    // output enable for 74HC595 shift registers
#define ARMEDPIN 7    // optoisolator connected to load power
#define DATAPIN  11   // data for 74HC595 shift registers
#define CLOCKPIN 13   // clock for 74HC595 shift registers

#define STARTUP_TIMEOUT 5000
#define MESSAGE_WAIT_TIMEOUT 10000

#define NUM_BOARDS 6  // the number of boards in the chain
#define OUTPUT_BYTES_PER_BOARD 2 // it is called the wifire16 :)

#define PACKET_START 0xee
#define MESSAGE_BUFFER 0xde
#define MESSAGE_MODE 0xbe

int numOutputBytes = NUM_BOARDS * OUTPUT_BYTES_PER_BOARD;
int numChannels = numOutputBytes * 8;
byte outputBytes[NUM_BOARDS * OUTPUT_BYTES_PER_BOARD] = {0};

int atMessageByte = 0;
byte readChar;
int MESSAGE_LENGTH = numOutputBytes + 2;
byte message[NUM_BOARDS * OUTPUT_BYTES_PER_BOARD + 2] = {0};

unsigned long lastMessageAt = 0;
unsigned long lastUpdate = 0;
unsigned long currentStep = 0;

int TIME_BETWEEN_FRAMES = 100;

boolean showPattern = true;

void (*currentPattern)();

void setup() {
  // set all output pins
  SPI.begin(); // handles DATAPIN and CLOCKPIN
  pinMode(LATCHPIN, OUTPUT);
  pinMode(OEPIN, OUTPUT);
  pinMode(CLEARPIN, OUTPUT);

  // make sure no lines go active until data is shifted out
  digitalWrite(CLEARPIN, HIGH);
  digitalWrite(OEPIN, LOW);
  
  // activate built-in pull-up resistor
  digitalWrite(ARMEDPIN, HIGH);

  setLights();

  // ignore the WiFly radio start text
  delay(STARTUP_TIMEOUT);

  // start serial communication with the WiFly
  Serial.begin(115200);
  
  currentPattern = bouncePattern;
}

void loop() {
  if (Serial.available() > 0) {
    readChar = Serial.read();
    
    message[atMessageByte] = readChar;
    atMessageByte++;
    
    switch (atMessageByte) {
      // first byte in message
      case 1:
        if (readChar == PACKET_START) {
          Serial.println("Got message start sigil.");
        } else {
          resetStream();
        }
        break;
      // second byte in message
      case 2:
        switch (readChar) {
          case MESSAGE_MODE:
            Serial.println("message is to change pattern");
            break;
          case MESSAGE_BUFFER:
            Serial.println("message is a buffer");
            break;
          default:
            resetStream();
            break;
        }
      default:
        if (atMessageByte == MESSAGE_LENGTH) {
          Serial.println("Got full message.");
          handleMessage();
        }
        break;
    }
    
    if (atMessageByte > MESSAGE_LENGTH) {
      resetStream();
    }
  }
  
  unsigned long currentMillis = millis();
  
  if (!showPattern && (currentMillis - lastMessageAt) > MESSAGE_WAIT_TIMEOUT) {
    Serial.println("too long since last message, doing pattern");
    showPattern = true;
  }
  
  if (showPattern) {
    if ((currentMillis - lastUpdate) > TIME_BETWEEN_FRAMES) {
      setLights();
      currentPattern();
      
      lastUpdate = currentMillis;
      currentStep++;
    }
  }
}

void resetStream() {
  Serial.println("resetting stream");
  atMessageByte = 0;
}

void handleMessage() {
  switch (message[1]) {
    case MESSAGE_MODE:
      Serial.println("message is mode change");
      
      switch (message[2]) {
        case 0x01:
          currentPattern = chasePattern;
          break;
        case 0x02:
          currentPattern = bouncePattern;
          break;
      }
      
      showPattern = true;
      break;
    case MESSAGE_BUFFER:
      Serial.println("showing buffer");
      
      lastMessageAt = millis();
      showPattern = false;
      
      for (int i = 0; i < numOutputBytes; i++) {
        outputBytes[i] = message[i + 2];
      }
      setLights();
      break;
  }
  
  resetStream();
}

void chasePattern() {
  if (currentStep >= numChannels * 2) {
    currentStep = 0;
  }
  
  // Clear all lights
  for (int channel = 0; channel < numChannels; channel++) {
    bitSet(outputBytes[channel / 8], channel % 8);
  }
  
  int centerLight;
  
  if (currentStep < numChannels) {
    centerLight = currentStep;
  } else if (currentStep < (numChannels * 2)) {
    centerLight = (numChannels * 2) - currentStep;
  }
  
  channelClear(centerLight - 1);
  channelClear(centerLight);
  channelClear(centerLight + 1);
}

void bouncePattern() {
  if (currentStep >= (numChannels - 2) - 1) {
    currentStep = 0;
  }
  
  // Clear all lights
  for (int channel = 0; channel < numChannels; channel++) {
    bitSet(outputBytes[channel / 8], channel % 8);
  }
  
  int topLight = currentStep;
  int bottomLight = numChannels - currentStep;
  
  channelClear(topLight);
  channelClear(topLight + 1);
  
  channelClear(bottomLight - 1);
  channelClear(bottomLight - 2);
}

void setLights() {
  digitalWrite(LATCHPIN, LOW);
  digitalWrite(OEPIN, HIGH);
  
  // actually send out the buffer
  for (int i = 0; i < numOutputBytes; i++) {
    SPI.transfer(outputBytes[i]);
  }
  
  digitalWrite(LATCHPIN, HIGH);
  digitalWrite(OEPIN, LOW);
}

void channelSet(int channel) {
  if (channel >= 0 && channel < numChannels) {
    bitSet(outputBytes[channel / 8], channel % 8);
  }
}

void channelClear(int channel) {
  if (channel >= 0 && channel < numChannels) {
    bitClear(outputBytes[channel / 8], channel % 8);
  }
}
