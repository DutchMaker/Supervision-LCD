#include "Arduino.h"

// IPS pins
#define PIN_IPS_HSYNC 4
#define PIN_IPS_VSYNC 7
#define PIN_IPS_CLOCK 8
#define PIN_IPS_DATA0 5
#define PIN_IPS_DATA1 6

// Supervision pins
#define PIN_SV_DATA0           22
#define PIN_SV_DATA1           21
#define PIN_SV_DATA2           20
#define PIN_SV_DATA3           19
#define PIN_SV_PIXEL_CLOCK     18
#define PIN_SV_LINE_LATCH      17
#define PIN_SV_FRAME_POLARITY  16

IntervalTimer ips_hsyncTimer;
IntervalTimer ips_vsyncTimer;

uint8_t frameBuffer[160][144];
volatile unsigned int ips_currentLine = 0;
volatile unsigned int ips_currentPixel = 0;

bool sv_wait_for_new_field = true;
int sv_pin_state_clock = 0;
int sv_pin_state_line_latch = 0;
int sv_pin_state_frame_polarity = -1;
int sv_currentField = 0;
int sv_currentLine = 0;
bool sv_skip_line = false;
int sv_currentPixel = 0;
bool ips_rendering_frame = false;

void draw_test_screen();
void render_ips_frame();
void capture_sv_frame();
void ips_hsync();
void ips_vsync();
void reset_state();

//////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(PIN_IPS_HSYNC, OUTPUT);
  pinMode(PIN_IPS_VSYNC, OUTPUT);
  pinMode(PIN_IPS_CLOCK, OUTPUT);
  pinMode(PIN_IPS_DATA0, OUTPUT);
  pinMode(PIN_IPS_DATA1, OUTPUT);

  pinMode(PIN_SV_DATA0, INPUT);
  pinMode(PIN_SV_DATA1, INPUT);
  pinMode(PIN_SV_DATA2, INPUT);
  pinMode(PIN_SV_DATA3, INPUT);
  pinMode(PIN_SV_PIXEL_CLOCK, INPUT);
  pinMode(PIN_SV_LINE_LATCH, INPUT);
  pinMode(PIN_SV_FRAME_POLARITY, INPUT);

  digitalWriteFast(PIN_IPS_HSYNC, LOW);
  digitalWriteFast(PIN_IPS_VSYNC, LOW);
  digitalWriteFast(PIN_IPS_CLOCK, LOW);
  digitalWriteFast(PIN_IPS_DATA0, LOW);
  digitalWriteFast(PIN_IPS_DATA1, LOW);

  draw_test_screen();
}

//////////////////////////////////////////////////////////////////////////
void loop() {
  if (ips_rendering_frame) {
    render_ips_frame();
  }

  capture_sv_frame();
}

//////////////////////////////////////////////////////////////////////////
void draw_test_screen() {
  for (int i = 0; i < 160; i++) {
    for (int j = 0; j < 144; j++) {
      frameBuffer[i][j] = LOW;
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
              frameBuffer[i][j] = HIGH;
          }
      }
  }
}

//////////////////////////////////////////////////////////////////////////
void render_ips_frame() {
  if (ips_currentPixel >= 160) {
    return;
  }

  digitalWriteFast(PIN_IPS_CLOCK, HIGH);
  digitalWriteFast(PIN_IPS_DATA0, frameBuffer[ips_currentPixel][ips_currentLine] & 1);
  digitalWriteFast(PIN_IPS_DATA1, (frameBuffer[ips_currentPixel][ips_currentLine] >> 1) & 1);
  digitalWriteFast(PIN_IPS_CLOCK, LOW);
  ips_currentPixel++;

  // We cannot process SV screen data and render the IPS at the same time as the CPU is not fast enough.
  return;
}

//////////////////////////////////////////////////////////////////////////
void capture_sv_frame() {
  uint8_t sv_pixel_clock = digitalReadFast(PIN_SV_PIXEL_CLOCK);
  uint8_t sv_line_latch = digitalReadFast(PIN_SV_LINE_LATCH);
  uint8_t sv_frame_polarity = digitalReadFast(PIN_SV_FRAME_POLARITY);

  // Wait for frame polarity to go transition from low to high for the first time.
  if (sv_wait_for_new_field) {
    if (sv_pin_state_frame_polarity == -1 && sv_frame_polarity == LOW) {
      sv_pin_state_frame_polarity = 0;
      return;
    }
    else if (sv_pin_state_frame_polarity == 0 && sv_frame_polarity == HIGH) {
      sv_pin_state_frame_polarity = 1;
      sv_wait_for_new_field = false;
    }
    else {
      return;
    }
  }

  if (sv_pixel_clock == HIGH && sv_pin_state_clock != sv_pixel_clock) {
    // Pixel clock transitioned from low to high.
    if (sv_currentPixel < 160 - 3 && !sv_skip_line) {
      if (sv_currentField == 0) {
        frameBuffer[sv_currentPixel++][sv_currentLine] = digitalReadFast(PIN_SV_DATA0);
        frameBuffer[sv_currentPixel++][sv_currentLine] = digitalReadFast(PIN_SV_DATA1);
        frameBuffer[sv_currentPixel++][sv_currentLine] = digitalReadFast(PIN_SV_DATA2);
        frameBuffer[sv_currentPixel++][sv_currentLine] = digitalReadFast(PIN_SV_DATA3);
      }
      else {
        frameBuffer[sv_currentPixel++][sv_currentLine] |= digitalReadFast(PIN_SV_DATA0) << 1;
        frameBuffer[sv_currentPixel++][sv_currentLine] |= digitalReadFast(PIN_SV_DATA1) << 1;
        frameBuffer[sv_currentPixel++][sv_currentLine] |= digitalReadFast(PIN_SV_DATA2) << 1;
        frameBuffer[sv_currentPixel++][sv_currentLine] |= digitalReadFast(PIN_SV_DATA3) << 1;
      }
    }
  }
  else if (sv_line_latch == HIGH && sv_pin_state_line_latch != sv_line_latch) {
    // Line latch transitioned from low to high.
    if (sv_currentLine % 10 == 0 && !sv_skip_line) {  
      // Skip every 10th line. SuperVision has 160 lines, the IPS has 144.
      sv_skip_line = true;
      return;
    }

    sv_skip_line = false;
    
    if (sv_currentLine < 159) {
      sv_currentLine++;
      sv_currentPixel = 0;
    }
    else {
      sv_currentPixel = 0;
    }
  }

  if (sv_frame_polarity == HIGH && sv_pin_state_frame_polarity != sv_frame_polarity) {
    // Frame polarity transitioned from low to high.
    sv_currentField = 0;
    sv_currentLine = 0;
    sv_currentPixel = 0;

    // Start the timers to render the IPS screen.
    ips_rendering_frame = true;
    ips_hsyncTimer.begin(ips_hsync, 108);
    ips_vsyncTimer.begin(ips_vsync, 16666);

    return;
  }
  else if (sv_frame_polarity == LOW && sv_pin_state_frame_polarity != sv_frame_polarity) {
    // Frame polarity transitioned from high to low.
    sv_currentField = 1;
    sv_currentLine = 0;
    sv_currentPixel = 0;
  }

  sv_pin_state_clock = sv_pixel_clock;
  sv_pin_state_line_latch = sv_line_latch;
  sv_pin_state_frame_polarity = sv_frame_polarity;
}

//////////////////////////////////////////////////////////////////////////
void ips_hsync() {
  if (ips_currentLine == 144) {
    return;
  }

  digitalWriteFast(PIN_IPS_HSYNC, HIGH);
  delayNanoseconds(300);
  digitalWriteFast(PIN_IPS_HSYNC, LOW);

  ips_currentLine++;
  ips_currentPixel = 0;
}

//////////////////////////////////////////////////////////////////////////
void ips_vsync() {
  digitalWriteFast(PIN_IPS_VSYNC, HIGH);
  delayNanoseconds(300);
  digitalWriteFast(PIN_IPS_VSYNC, LOW);

  ips_hsyncTimer.end();
  ips_vsyncTimer.end();

  reset_state();
}

//////////////////////////////////////////////////////////////////////////
void reset_state() {
  sv_wait_for_new_field = true;
  sv_pin_state_clock = 0;
  sv_pin_state_line_latch = 0;
  sv_pin_state_frame_polarity = -1;
  sv_currentField = 0;
  sv_currentLine = 0;
  sv_skip_line = false;
  sv_currentPixel = 0;

  ips_rendering_frame = false;
  ips_currentLine = 0;
  ips_currentPixel = 0;
}