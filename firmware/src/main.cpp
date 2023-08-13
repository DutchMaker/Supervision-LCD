#include <Arduino.h>

//#define INTERRUPT_PIN_CLOCK 2   // This pin is an OR of the pixel clock and line latch signals.
                                // It will interrupt when pixel data is pushed and when a line is done.

// #define INTERRUPT_PIN_FP    3   // This pin is the frame polarity signal.
//                                 // It will change when a frame field is starting.

#define PIN_DATA0           4
#define PIN_DATA1           5
#define PIN_DATA2           6
#define PIN_DATA3           7
#define PIN_PIXEL_CLOCK     8
#define PIN_LINE_LATCH      9
#define PIN_FRAME_POLARITY  10
#define PIN_TEST            11

void ISR_clock();
void ISR_frame_polarity();
void render_frame();

uint8_t framebuffer[2][(160/8)*(160/8)]; // Supervision renders the screen as 2 field of one-bit pixels, 160x160 pixels per field.
uint8_t current_line = 0;
uint8_t current_pixel = 0;
uint8_t current_field = 0;

bool booting = true;
bool halt = false;
uint8_t first_frames_count = 0;

int pin_state_pixel_clock = 0;
int pin_state_line_latch = 0;
int pin_state_frame_polarity = 0;

int count_cc = 0;
int count_ll = 0;
int count_fp = 0;
int out = 0;

void setup() 
{
  //pinMode(INTERRUPT_PIN_CLOCK, INPUT);
  //pinMode(INTERRUPT_PIN_FP, INPUT_PULLUP);
  pinMode(PIN_DATA0, INPUT);
  pinMode(PIN_DATA1, INPUT);
  pinMode(PIN_DATA2, INPUT);
  pinMode(PIN_DATA3, INPUT);
  pinMode(PIN_PIXEL_CLOCK, INPUT);
  pinMode(PIN_LINE_LATCH, INPUT);
  pinMode(PIN_FRAME_POLARITY, INPUT);
  
  pinMode(PIN_TEST, OUTPUT);

  Serial.begin(9600);

  Serial.println("Supervision custom LCD firmware started");
  delay(1000);

  //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_CLOCK), ISR_clock, RISING);
  //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_FP), ISR_frame_polarity, CHANGE);
}

long nothing = 0;

void loop() 
{
  uint8_t pin_pixel_clock = (PIND & (1 << 2)) >> 2;
  uint8_t pin_line_latch = (PIND & (1 << 3)) >> 3;
  uint8_t pin_frame_polarity = (PIND & (1 << 4)) >> 4;

  if ((pin_pixel_clock == LOW || pin_pixel_clock == pin_state_pixel_clock) && (pin_line_latch == LOW || pin_line_latch == pin_state_line_latch)) {
    // No clock or line latch pulse, so nothing to do.
    //return;
  }

  // if (booting) {
  //   // Wait 50 frames before we start the rendering process.
  //   if (pin_frame_polarity != pin_state_frame_polarity) {
  //     first_frames_count++;
  //   }

  //   if (pin_frame_polarity == LOW && first_frames_count > 5) {
  //     booting = false;
  //   }
  //   else {
  //     pin_state_frame_polarity = pin_frame_polarity;
  //     return;
  //   }
  // }

  //PORTB |= (1 << 0);
  //PORTB &= ~(1 << 0);
  PORTB ^= (1 << 3);

  // if (pin_pixel_clock == HIGH && pin_pixel_clock != pin_state_pixel_clock) {
  //   count_cc++;
  // }
  // else if (pin_line_latch == HIGH && pin_line_latch != pin_state_line_latch) {
  //   count_ll++;
  // }

  // //if (pin_frame_polarity == HIGH && pin_frame_polarity != pin_state_frame_polarity) {
  // if (count_ll == 160) {
  //   count_fp++;

  //   Serial.print("nothing: ");
  //   Serial.println(nothing);
  //   Serial.print("cc: ");
  //   Serial.println(count_cc);
  //   Serial.print("ll: ");
  //   Serial.println(count_ll);
  //   Serial.print("fp: ");
  //   Serial.println(count_fp);

  //   halt = true;
  // }

  pin_state_pixel_clock = pin_pixel_clock;
  pin_state_line_latch = pin_line_latch;
  pin_state_frame_polarity = pin_frame_polarity;
}



// void ISR_clock()
// {
//   uint8_t fp = digitalRead(PIN_FRAME_POLARITY);

//   // Wait for a couple of frames before we start rendering.
//   if (booting) {

//     if (fp != frame_polarity) {
//       frame_polarity = fp;
//       first_frames++;
//     }

//     if (fp && first_frames > 5) {
//       booting = false;
//     }
//     else {
//       return;
//     }
//   }

//   cc++;

//   if (fp != frame_polarity) {
//     frame_polarity = fp;
//     field = !frame_polarity;
//     line = 0;
//     pixel = 0;

//     fc++;

//     if (!field) {
//       render_frame();
//     }
//   }

//   if (digitalRead(PIN_PIXEL_CLOCK) == HIGH)
//   {
//     fpc++;

//     // Store the pixel data.
//     framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA0) << (pixel % 8));
//     framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA1) << (pixel % 8 + 1));
//     framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA2) << (pixel % 8 + 2));
//     framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA3) << (pixel % 8 + 3));
//     pixel += 4;
//   }

//   if (digitalRead(PIN_LINE_LATCH) == HIGH)
//   {
//     llc++;

//     // Store the line data.
//     line++;
//     pixel = 0;
//   }
// }

// void render_frame()
// {
//   // Temporarily detach interrupts to stop processing after one frame.
//   detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_CLOCK));
  
//   Serial.println("Rendering");
//   Serial.println(cc);
//   Serial.println(fc);
//   Serial.println(fpc);
//   Serial.println(llc);

//   // For now, just print the frame to the serial port.
//   for (int i = 0; i < (160/8)*(160/8); i++)
//   {
//     if (i % (160/8) == 0) {
//       Serial.write('\n');
//     }

//     Serial.print(framebuffer[0][i], HEX);
//     Serial.print(' ');
//     // for (uint8_t j = 0; j < 8; j++)
//     // {
//     //   uint8_t bit1 = (framebuffer[0][i] >> j) & 1;
//     //   uint8_t bit2 = (framebuffer[1][i] >> j) & 1;
//     //   uint8_t pixel_type = 0;

//     //   if (bit1 == 0 && bit2 == 1) {
//     //     pixel_type = 1; // 1/3rd darkness
//     //   }
//     //   else if (bit1 == 1 && bit2 == 0) {
//     //     pixel_type = 1; // 2/3rd darkness
//     //   }
//     //   else if (bit1 == 1 && bit2 == 1) {
//     //     pixel_type = 3; // fully on
//     //   }

//     //   switch (pixel_type)
//     //   {
//     //     case 0:
//     //       Serial.write(' ');
//     //       break;
//     //     case 1:
//     //       Serial.write('O');
//     //       break;
//     //     case 2:
//     //       Serial.write('0');
//     //       break;
//     //     case 3:
//     //       Serial.write('@');
//     //       break;
//     //   }
//     // }    
//   }
// }