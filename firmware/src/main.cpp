#include "Arduino.h"

#define DEBUG false

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

// Test pins
#define PIN_TEST_SV_PIXEL_CLOCK  15
#define PIN_TEST_SV_LINE_LATCH   14
#define PIN_TEST_SV_FRAME_POLARITY  13

IntervalTimer ips_hsyncTimer;
IntervalTimer ips_vsyncTimer;

uint8_t frameBuffer[160][144];
volatile unsigned int ips_currentLine = 0;
volatile unsigned int ips_currentPixel = 0;

bool sv_wait_for_first_field = true;
int sv_pin_state_clock = 0;
int sv_pin_state_line_latch = 0;
int sv_pin_state_frame_polarity = -1;
volatile int sv_currentField = 0;
int sv_currentLine = 0;
bool sv_skip_line = false;
int sv_currentPixel = 0;
bool ips_rendering_frame = false;

void draw_test_screen();
void render_ips_frame();
void capture_sv_frame();
void ips_hsync();
void ips_vsync();
void sv_lineLatch();
void sv_framePolarity();
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

  pinMode(PIN_TEST_SV_PIXEL_CLOCK, OUTPUT);
  pinMode(PIN_TEST_SV_LINE_LATCH, OUTPUT);
  pinMode(PIN_TEST_SV_FRAME_POLARITY, OUTPUT);

  digitalWriteFast(PIN_IPS_HSYNC, LOW);
  digitalWriteFast(PIN_IPS_VSYNC, LOW);
  digitalWriteFast(PIN_IPS_CLOCK, LOW);
  digitalWriteFast(PIN_IPS_DATA0, LOW);
  digitalWriteFast(PIN_IPS_DATA1, LOW);

  digitalWriteFast(PIN_TEST_SV_PIXEL_CLOCK, LOW);
  digitalWriteFast(PIN_TEST_SV_LINE_LATCH, LOW);
  digitalWriteFast(PIN_TEST_SV_FRAME_POLARITY, LOW);

  attachInterrupt(digitalPinToInterrupt(PIN_SV_FRAME_POLARITY), sv_framePolarity, CHANGE);

  draw_test_screen();

  ips_hsyncTimer.begin(ips_hsync, 108);
  ips_vsyncTimer.begin(ips_vsync, 16666);
}

//////////////////////////////////////////////////////////////////////////
void loop() {
  if (ips_rendering_frame) {
    render_ips_frame();
    return; // We cannot process SV screen data and render the IPS at the same time as the CPU is not fast enough.
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
  delayNanoseconds(5);
  digitalWriteFast(PIN_IPS_DATA0, frameBuffer[ips_currentPixel][ips_currentLine] & 1);
  digitalWriteFast(PIN_IPS_DATA1, (frameBuffer[ips_currentPixel][ips_currentLine] >> 1) & 1);
  delayNanoseconds(5);
  digitalWriteFast(PIN_IPS_CLOCK, LOW);
  ips_currentPixel++;
}

//////////////////////////////////////////////////////////////////////////
void capture_sv_frame() {
  // Wait for frame polarity to go transition from low to high for the first time.
  if (sv_wait_for_first_field) {
      return;
  }

  uint8_t sv_pixel_clock = digitalReadFast(PIN_SV_PIXEL_CLOCK);
  uint8_t sv_line_latch = digitalReadFast(PIN_SV_LINE_LATCH);

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
      
      #if DEBUG
      digitalWriteFast(PIN_TEST_SV_LINE_LATCH, sv_line_latch);
      #endif

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

  sv_pin_state_clock = sv_pixel_clock;
  sv_pin_state_line_latch = sv_line_latch;

  #if DEBUG
  digitalWriteFast(PIN_TEST_SV_PIXEL_CLOCK, sv_pixel_clock);
  digitalWriteFast(PIN_TEST_SV_LINE_LATCH, sv_line_latch);
  #endif
}

//////////////////////////////////////////////////////////////////////////
void sv_framePolarity() {
  if (ips_rendering_frame) {
    return;
  }

  sv_currentField = !digitalReadFast(PIN_SV_FRAME_POLARITY);
  sv_currentLine = 0;
  sv_currentPixel = 0;

  #if DEBUG
  digitalWriteFast(PIN_TEST_SV_FRAME_POLARITY, !sv_currentField);
  #endif

  if (sv_wait_for_first_field && sv_currentField == 0) {
    sv_wait_for_first_field = false;
    return;
  }

  // First and second field captured.
  // Start rendering the IPS screen.
  if (sv_currentField == 0) {
    ips_rendering_frame = true;
  }
}

//////////////////////////////////////////////////////////////////////////
void ips_hsync() {
  if (!ips_rendering_frame) {
    return; 
  }

  if (ips_currentLine == 144) {
    return;
  }

  digitalWriteFast(PIN_IPS_HSYNC, HIGH);
  delayNanoseconds(200);
  digitalWriteFast(PIN_IPS_HSYNC, LOW);

  ips_currentLine++;
  ips_currentPixel = 0;
}

//////////////////////////////////////////////////////////////////////////
void ips_vsync() {
  if (!ips_rendering_frame) {
    return; 
  }

  digitalWriteFast(PIN_IPS_VSYNC, HIGH);
  delayNanoseconds(200);
  digitalWriteFast(PIN_IPS_VSYNC, LOW);

  reset_state();
}

//////////////////////////////////////////////////////////////////////////
void reset_state() {
  sv_wait_for_first_field = true;
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