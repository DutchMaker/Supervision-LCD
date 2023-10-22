#include "Arduino.h"

// IPS pins
#define PIN_HSYNC 4
#define PIN_VSYNC 7
#define PIN_CLOCK 8
#define PIN_DATA0 5
#define PIN_DATA1 6

// Supervision pins
#define PIN_SV_DATA0           22
#define PIN_SV_DATA1           21
#define PIN_SV_DATA2           20
#define PIN_SV_DATA3           19
#define PIN_SV_PIXEL_CLOCK     18
#define PIN_SV_LINE_LATCH      17
#define PIN_SV_FRAME_POLARITY  16

IntervalTimer hsyncTimer;
IntervalTimer vsyncTimer;

uint8_t frameBuffer[160][144];
volatile unsigned int currentLine = 0;
volatile unsigned int currentPixel = 0;
bool initialized = false;

void hsync();
void vsync();

void setup() {
  pinMode(PIN_HSYNC, OUTPUT);
  pinMode(PIN_VSYNC, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA0, OUTPUT);
  pinMode(PIN_DATA1, OUTPUT);

  digitalWriteFast(PIN_HSYNC, LOW);
  digitalWriteFast(PIN_VSYNC, LOW);
  digitalWriteFast(PIN_CLOCK, LOW);
  digitalWriteFast(PIN_DATA0, LOW);
  digitalWriteFast(PIN_DATA1, LOW);

  for (int i = 0; i < 160; i++) {
    for (int j = 0; j < 144; j++) {
      frameBuffer[i][j] = LOW;
    }
  }

  // Draw a circle
  int centerX = 80;
  int centerY = 72;
  int radius = 50;
  for (int i = 0; i < 160; i++) {
      for (int j = 0; j < 144; j++) {
          if (sqrt(pow(i - centerX, 2) + pow(j - centerY, 2)) <= radius) {
              frameBuffer[i][j] = HIGH;
          }
      }
  }

  hsyncTimer.begin(hsync, 108);
  vsyncTimer.begin(vsync, 16666);
}

void loop() {
  if (currentPixel >= 160) {
    return;
  }

  digitalWriteFast(PIN_CLOCK, HIGH);
  digitalWriteFast(PIN_DATA0, frameBuffer[currentPixel][currentLine]);
  digitalWriteFast(PIN_DATA1, frameBuffer[currentPixel][currentLine]);
  digitalWriteFast(PIN_CLOCK, LOW);
  currentPixel++;
}

void hsync() {
  if (currentLine == 144) {
    return;
  }

  digitalWriteFast(PIN_HSYNC, HIGH);
  delayMicroseconds(1);
  digitalWriteFast(PIN_HSYNC, LOW);

  currentLine++;
  currentPixel = 0;
}

void vsync() {
  digitalWriteFast(PIN_VSYNC, HIGH);
  delayMicroseconds(1);
  digitalWriteFast(PIN_VSYNC, LOW);
  
  hsyncTimer.end();
  vsyncTimer.end();
  hsyncTimer.begin(hsync, 108);
  vsyncTimer.begin(vsync, 16666);

  currentLine = 0;
  currentPixel = 0;
}
