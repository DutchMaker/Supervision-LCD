#include <Arduino.h>

#define PIN_TEST 11

uint8_t test = 0;

void setup() {
  pinMode(PIN_TEST, OUTPUT);
}

void loop() {
    GPIO.out_w1ts = (1 << PIN_TEST); // Set the pin HIGH
    GPIO.out_w1tc = (1 << PIN_TEST); // Set the pin LOW


      // Read from pin
  // int value;
  // if (PIN < 32) {
  //   value = (GPIO.in >> PIN) & 0x1;
  // } else {
  //   value = (GPIO.in1.val >> (PIN - 32)) & 0x1;
  // }
}
