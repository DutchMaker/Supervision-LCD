#include <Arduino.h>

#define PIN_DATA0           2
#define PIN_DATA1           3
#define PIN_DATA2           4
#define PIN_DATA3           5
#define PIN_PIXEL_CLOCK     6
#define PIN_LINE_LATCH      7
#define PIN_FRAME_POLARITY  8
#define PIN_TEST            9

#define FRAMEBUFFER_SIZE    3200 //160*(160/8)

uint8_t framebuffer[2][FRAMEBUFFER_SIZE]; // Supervision renders the screen as 2 field of one-bit pixels, 160x160 pixels per field.
uint8_t current_pixel = 0;
uint8_t current_line = 0;
uint8_t current_field = 0;

uint8_t pixel_clock_state = 0;
uint8_t line_latch_state = 0;
uint8_t frame_polarity_state = 0;

bool booting = true;
uint16_t booting_framecount = 0;
bool halt = false;

uint32_t count_cc = 0;
uint32_t count_ll = 0;
uint32_t count_fp = 0;

uint8_t out = 0;

void ISR_pixel_clock();
void ISR_line_latch();
void ISR_frame_polarity();

void setup() 
{
  pinMode(PIN_DATA0, INPUT);
  pinMode(PIN_DATA1, INPUT);
  pinMode(PIN_DATA2, INPUT);
  pinMode(PIN_DATA3, INPUT);
  pinMode(PIN_PIXEL_CLOCK, INPUT);
  pinMode(PIN_LINE_LATCH, INPUT);
  pinMode(PIN_FRAME_POLARITY, INPUT);
  
  pinMode(PIN_TEST, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_PIXEL_CLOCK), ISR_pixel_clock, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_LINE_LATCH), ISR_line_latch, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_FRAME_POLARITY), ISR_frame_polarity, CHANGE);

  Serial.begin(9600);

  Serial.println("Supervision custom LCD firmware started");
  delay(1000);
}

void loop() 
{
  if (halt) {
    return;
  }

  // uint8_t pixel_clock = digitalReadFast(PIN_PIXEL_CLOCK);
  // uint8_t line_latch = digitalReadFast(PIN_LINE_LATCH);
  // uint8_t frame_polarity = digitalReadFast(PIN_FRAME_POLARITY);

  // if (pixel_clock == pixel_clock_state && line_latch == line_latch_state) {
  //   // No clock or line latch pulse, so nothing to do.
  //    return;
  // }

  // if (booting) {
  //   // Wait some frames before we start the rendering process.
  //   if (frame_polarity != frame_polarity_state) {
  //     booting_framecount++;
  //     frame_polarity_state = frame_polarity;
  //   }

  //   if (frame_polarity == LOW && booting_framecount > 100) {
  //     booting = false;
  //   }
  //   else {
  //     return;
  //   }
  // }

  // if (pixel_clock == HIGH && pixel_clock != pixel_clock_state) {
  //   count_cc++;
  // }
  // else if (line_latch == HIGH && line_latch != line_latch_state) {
  //   count_ll++;
  // }

  // if (frame_polarity != frame_polarity_state) {
  //   count_fp++;

    if (count_fp == 1000) {
      Serial.print("cc: ");
      Serial.println(count_cc);
      Serial.print("ll: ");
      Serial.println(count_ll);
      Serial.print("fp: ");
      Serial.println(count_fp);
      
      halt = true;
    }
  // }

  // pixel_clock_state = pixel_clock;
  // line_latch_state = line_latch;
  // frame_polarity_state = frame_polarity;
}

void ISR_pixel_clock()
{
  if (booting) {
    // Wait some frames before we start the rendering process.
    uint8_t frame_polarity = digitalReadFast(PIN_FRAME_POLARITY);

    if (frame_polarity != frame_polarity_state) {
      booting_framecount++;
      frame_polarity_state = frame_polarity;
    }

    if (frame_polarity == LOW && booting_framecount > 100) {
      booting = false;
    }
    else {
      return;
    }
  }

  // if (current_line*20 + current_pixel/8 >= FRAMEBUFFER_SIZE) {
  //     Serial.println("OVERFLOW!");
  //     Serial.println(current_line*20 + current_pixel/8);
  //     Serial.print("current_field: ");
  //     Serial.println(current_field);
  //     Serial.print("current_line: ");
  //     Serial.println(current_line);
  //     Serial.print("current_pixel: ");
  //     Serial.println(current_pixel);
  // }

  // Store the pixel data.
  framebuffer[current_field][current_line*20 + current_pixel/8] |= (digitalReadFast(PIN_DATA0) << (current_pixel % 8));
  framebuffer[current_field][current_line*20 + current_pixel/8] |= (digitalReadFast(PIN_DATA1) << (current_pixel % 8 + 1));
  framebuffer[current_field][current_line*20 + current_pixel/8] |= (digitalReadFast(PIN_DATA2) << (current_pixel % 8 + 2));
  framebuffer[current_field][current_line*20 + current_pixel/8] |= (digitalReadFast(PIN_DATA3) << (current_pixel % 8 + 3));
  current_pixel += 4;

  if (current_pixel >= 160) {
    current_pixel = 0;
  }

  count_cc++;
}

void ISR_line_latch()
{
  if (booting) {
    return;
  }

  current_line++;
  current_pixel = 0;

  if (current_line >= 160) {
    current_line = 0;
  }

  count_ll++;
}

void ISR_frame_polarity()
{
  if (booting) {
    return;
  }

  current_field = !current_field;
  current_line = 0;
  current_pixel = 0;

  count_fp++;
}