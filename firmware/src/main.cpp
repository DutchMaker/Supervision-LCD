#include <Arduino.h>

#define INTERRUPT_PIN_CLOCK 2   // This pin is an OR of the pixel clock and line latch signals.
                                // It will interrupt when pixel data is pushed and when a line is done.

#define INTERRUPT_PIN_FL    3   // This pin is the frame latch signal.
                                // It will interrupt when a frame is done.

#define PIN_DATA0           4
#define PIN_DATA1           5
#define PIN_DATA2           6
#define PIN_DATA3           7
#define PIN_PIXEL_CLOCK     8
#define PIN_LINE_LATCH      9
#define PIN_FRAME_POLARITY  10

void ISR_clock();
void ISR_frame_latch();
void render_frame();

uint8_t framebuffer[2][(160/8)*(160/8)]; // Supervision renders the screen as 2 field of one-bit pixels, 160x160 pixels per field.
uint8_t line = 0;
uint8_t pixel = 0;
uint8_t field = 0;
bool booting = true;

void setup() 
{
  pinMode(INTERRUPT_PIN_CLOCK, INPUT);
  pinMode(INTERRUPT_PIN_FL, INPUT);
  pinMode(PIN_DATA0, INPUT);
  pinMode(PIN_DATA1, INPUT);
  pinMode(PIN_DATA2, INPUT);
  pinMode(PIN_DATA3, INPUT);
  pinMode(PIN_PIXEL_CLOCK, INPUT);
  pinMode(PIN_LINE_LATCH, INPUT);
  pinMode(PIN_FRAME_POLARITY, INPUT);

  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_CLOCK), ISR_clock, RISING);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_FL), ISR_frame_latch, RISING);
}

void loop() 
{
}

void ISR_clock()
{
  if (booting && !digitalRead(PIN_FRAME_POLARITY)) {
    // Wait for first frame to start rendering before processing interrupts.
    return;
  }
  else if (booting && digitalRead(PIN_FRAME_POLARITY)) {
    booting = false;
  }

  if (digitalRead(PIN_PIXEL_CLOCK) == HIGH)
  {
    // Store the pixel data.
    framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA0) << (pixel % 8));
    framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA1) << (pixel % 8 + 1));
    framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA2) << (pixel % 8 + 2));
    framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA3) << (pixel % 8 + 3));
    pixel += 4;
  }

  if (digitalRead(PIN_LINE_LATCH) == HIGH)
  {
    // Store the line data.
    line++;
    pixel = 0;
  }
}

void ISR_frame_latch()
{
  field = !digitalRead(PIN_FRAME_POLARITY);
  line = 0;
  pixel = 0;

  if (!field) {
    render_frame();
  }
}

void render_frame()
{
  // Temporarily detach interrupts to stop processing after one frame.
  detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_CLOCK));
  detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_FL));

  // For now, just print the frame to the serial port.
  for (uint8_t i = 0; i < (160/8)*(160/8); i++)
  {
    for (uint8_t j = 0; j < 8; j++)
    {
      uint8_t bit1 = (framebuffer[0][i] >> j) & 1;
      uint8_t bit2 = (framebuffer[1][i] >> j) & 1;
      uint8_t pixel_type = 0;

      if (bit1 == 0 && bit2 == 1) {
        pixel_type = 1; // 1/3rd darkness
      }
      else if (bit1 == 1 && bit2 == 0) {
        pixel_type = 1; // 2/3rd darkness
      }
      else if (bit1 == 1 && bit2 == 1) {
        pixel_type = 3; // fully on
      }

      switch (pixel_type)
      {
        case 0:
          Serial.write(' ');
          break;
        case 1:
          Serial.write('O');
          break;
        case 2:
          Serial.write('0');
          break;
        case 3:
          Serial.write('@');
          break;
      }
    }

    if (i % (160/8) == 0) {
      Serial.println();
    }
  }
}