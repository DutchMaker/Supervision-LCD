#include "Arduino.h"
#include "intro_image.h"

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
#define PIN_SV_FRAME_LATCH     16
#define PIN_SV_FRAME_POLARITY  15
#define PIN_SV_POWER           12 // This pin controls the relay to turn on/off the Supervision console.

#define SV_BOOT_TIME 39000
#define VSYNC_TIMER_DELAY 16666
#define VSYNC_END_TIMER_DELAY 107
#define HSYNC_TIMER_DELAY 108

#define FRAMEBUFFER_WIDTH 160
#define FRAMEBUFFER_HEIGHT 144
#define FRAMEBUFFER_INDEX(x, y) (x + y * FRAMEBUFFER_WIDTH)

// Timers for the IPS screen hsync and vsync signals.
IntervalTimer ips_hsyncTimer;
IntervalTimer ips_vsyncTimer;
IntervalTimer ips_vsyncEndTimer;

// Framebuffer.
volatile uint8_t frameBuffer[4][FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
volatile unsigned int ips_currentLine = 0;
volatile unsigned int ips_currentPixel = 0;

// Various state variables.
bool wait_for_intro = false;
uint32_t wait_for_intro_time = 0;
bool sv_wait_for_boot = true;
uint8_t boot_line_latch_count = 0;
uint8_t boot_frame_latch_count = 0;
bool sv_wait_for_new_field = true;
int sv_pin_state_clock = 0;
int sv_pin_state_line_latch = 0;
int sv_pin_state_frame_polarity = -1;
int sv_pin_state_frame_latch = 0;
int sv_currentField = 0;
int sv_currentLine = 0;
bool sv_skip_line = false;
int sv_currentPixel = 0;
bool ips_rendering_frame = false;
bool rendering_intro = false;

// Function definitions.
void draw_intro_screen();
void wait_for_sv_boot();
void start_rendering_ips();
void render_ips_frame(bool pulse_clock);
void capture_sv_frame();
void ips_hsync();
void ips_hsync(bool reset_line);
void ips_vsync1_start();
void ips_vsync1_end();
void ips_frame_rendered();
void reset_state();

//////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(PIN_SV_POWER, OUTPUT);
  digitalWriteFast(PIN_SV_POWER, LOW);

  pinMode(PIN_IPS_VSYNC, OUTPUT);
  pinMode(PIN_IPS_CLOCK, OUTPUT);
  pinMode(PIN_IPS_HSYNC, OUTPUT);
  pinMode(PIN_IPS_DATA0, OUTPUT);
  pinMode(PIN_IPS_DATA1, OUTPUT);

  pinMode(PIN_SV_DATA0, INPUT_PULLDOWN);
  pinMode(PIN_SV_DATA1, INPUT_PULLDOWN);
  pinMode(PIN_SV_DATA2, INPUT_PULLDOWN);
  pinMode(PIN_SV_DATA3, INPUT_PULLDOWN);
  pinMode(PIN_SV_PIXEL_CLOCK, INPUT_PULLDOWN);
  pinMode(PIN_SV_LINE_LATCH, INPUT_PULLDOWN);
  pinMode(PIN_SV_FRAME_POLARITY, INPUT_PULLDOWN);
  pinMode(PIN_SV_FRAME_LATCH, INPUT_PULLDOWN);

  digitalWriteFast(PIN_IPS_HSYNC, LOW);
  digitalWriteFast(PIN_IPS_VSYNC, LOW);
  digitalWriteFast(PIN_IPS_CLOCK, LOW);
  digitalWriteFast(PIN_IPS_DATA0, LOW);
  digitalWriteFast(PIN_IPS_DATA1, LOW);

  // Draw the intro screen into the framebuffer array.
  draw_intro_screen();

  wait_for_intro = true;
  rendering_intro = true;

  // Render the framebuffer to the IPS screen.
  start_rendering_ips();
}

//////////////////////////////////////////////////////////////////////////
// The main loop will either capture the Supervision LCD data and store it 
// into the framebufer or render the framebuffer to the IPS screen.
void loop() {
  if (ips_rendering_frame) {
    render_ips_frame(true);
    return;
  }

  if (wait_for_intro) {
    if (millis() - wait_for_intro_time > 2000) {
      wait_for_intro = false;
      digitalWriteFast(PIN_SV_POWER, HIGH);
    }
    else if (!ips_rendering_frame) {
      start_rendering_ips();
      return;
    }
    return;
  }

  if (sv_wait_for_boot) {
    wait_for_sv_boot();
    return;
  }

  capture_sv_frame();
}

//////////////////////////////////////////////////////////////////////////
// Draws the custom intro screen into the framebuffer.
void draw_intro_screen() {
  for (int i = 0; i < 160; i++) {
    for (int j = 0; j < 144; j++) {
      frameBuffer[0][FRAMEBUFFER_INDEX(i, j)] = intro_image[FRAMEBUFFER_INDEX(i, j)];
      frameBuffer[1][FRAMEBUFFER_INDEX(i, j)] = intro_image[FRAMEBUFFER_INDEX(i, j)];
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Wait for the Supervision to boot up in the correct state.
// If the boot state is not correct, reboot the console and try again.
void wait_for_sv_boot() {
  uint8_t sv_line_latch = digitalReadFast(PIN_SV_LINE_LATCH);
  uint8_t sv_frame_latch = digitalReadFast(PIN_SV_FRAME_LATCH);

  if (boot_line_latch_count < 50 && sv_line_latch != sv_pin_state_line_latch  && sv_line_latch == HIGH) {
    boot_line_latch_count++;
  }

  if (sv_frame_latch != sv_pin_state_frame_latch  && sv_frame_latch == HIGH) {
    boot_frame_latch_count++;
  }

  sv_pin_state_line_latch = sv_line_latch;
  sv_pin_state_frame_latch = sv_frame_latch;

  if (boot_line_latch_count == 50) {
    boot_line_latch_count++;
    boot_frame_latch_count = 0;
    return;
  }
  
  if (boot_frame_latch_count >= 4) {
    if (digitalReadFast(PIN_SV_FRAME_POLARITY) == LOW) {
      // Invalid boot state:
      // we want frame polarity to start with HIGH signal for so we know we are starting with the correct type of frame.
      // Reboot the console and try again.
      delay(1000);
      digitalWriteFast(PIN_SV_POWER, LOW);
      boot_line_latch_count = 0;
      boot_frame_latch_count = 0;
      sv_pin_state_line_latch = 0;
      sv_pin_state_frame_latch = 0;
      delay(1000);
      digitalWriteFast(PIN_SV_POWER, HIGH);

      return;
    }

    // Valid boot state.
    // Start the main loop to capture the Supervision LCD data and write it to the IPS screen.
    sv_wait_for_boot = false;
  }
}

//////////////////////////////////////////////////////////////////////////
// Start the timers to render one frame to the IPS screen.
void start_rendering_ips() {
  ips_rendering_frame = true;
  ips_vsync1_start();
}

//////////////////////////////////////////////////////////////////////////
// Sends the framebuffer to the IPS screen one pixel at a time.
void render_ips_frame(bool pulse_clock) {
  if (ips_currentPixel >= 160) {
    return;
  }

  if (pulse_clock) {
    digitalWriteFast(PIN_IPS_CLOCK, HIGH);
    delayNanoseconds(15);
  }

  digitalWriteFast(PIN_IPS_DATA0, frameBuffer[0][FRAMEBUFFER_INDEX(ips_currentPixel, ips_currentLine)]);
  digitalWriteFast(PIN_IPS_DATA1, frameBuffer[1][FRAMEBUFFER_INDEX(ips_currentPixel, ips_currentLine)]);

  if (pulse_clock) {
    delayNanoseconds(15);
    digitalWriteFast(PIN_IPS_CLOCK, LOW);
  }
  
  ips_currentPixel++;
}

//////////////////////////////////////////////////////////////////////////
// Capture the Supervision LCD data and store it into the framebuffer.
void capture_sv_frame() {
  uint8_t sv_pixel_clock = digitalReadFast(PIN_SV_PIXEL_CLOCK);
  uint8_t sv_line_latch = digitalReadFast(PIN_SV_LINE_LATCH);
  uint8_t sv_frame_polarity = digitalReadFast(PIN_SV_FRAME_POLARITY);

  // Wait for frame polarity to go transition from low to high for the first time.
  // We do this so we can start capturing the frame fields at the correct time so we don't start with an inverted frame field.
  if (sv_wait_for_new_field) {
    if (sv_pin_state_frame_polarity == -1 && sv_frame_polarity == LOW) {
      // Frame polarity transitioned for the first time since we started capturing.
      sv_pin_state_frame_polarity = 0;
      sv_pin_state_clock = sv_pixel_clock;
      sv_pin_state_line_latch = sv_line_latch;
      return;
    }
    else if (sv_pin_state_frame_polarity == 0 && sv_frame_polarity == HIGH) {
      // Frame polarity transitioned from low to high.
      sv_pin_state_frame_polarity = 1;
      sv_wait_for_new_field = false;
    }
    else {
      // Continue to wait.
      goto update_pinstate_and_return;
    }
  }

  if (sv_line_latch == HIGH && sv_pin_state_line_latch != sv_line_latch) {
    // Line latch transitioned from low to high.
    if (sv_currentLine % 10 == 0 && !sv_skip_line) {  
      // Supervision has 160 lines but the IPS we use only has 144.
      // Skip every 10th line.
      sv_skip_line = true;
    }
    else {
      sv_skip_line = false;    
      sv_currentLine++;
      sv_currentPixel = 0;
    }
  }
  else if (sv_pixel_clock == HIGH && sv_pin_state_clock != sv_pixel_clock) {
    // Pixel clock transitioned from low to high.
    if (sv_currentPixel < 157 && !sv_skip_line && sv_currentLine < 144 && sv_currentField < 2) {
      frameBuffer[sv_currentField][FRAMEBUFFER_INDEX(sv_currentPixel++, sv_currentLine)] = digitalReadFast(PIN_SV_DATA0);
      frameBuffer[sv_currentField][FRAMEBUFFER_INDEX(sv_currentPixel++, sv_currentLine)] = digitalReadFast(PIN_SV_DATA1);
      frameBuffer[sv_currentField][FRAMEBUFFER_INDEX(sv_currentPixel++, sv_currentLine)] = digitalReadFast(PIN_SV_DATA2);
      frameBuffer[sv_currentField][FRAMEBUFFER_INDEX(sv_currentPixel++, sv_currentLine)] = digitalReadFast(PIN_SV_DATA3);
    }
  }

  if (sv_frame_polarity == HIGH && sv_pin_state_frame_polarity != sv_frame_polarity) {
    // Frame polarity transitioned from low to high.
    // Start the timers to render the IPS screen.
    sv_currentField++;
    sv_currentLine = 0;
    sv_currentPixel = 0;

    if (sv_currentField > 2) {
      start_rendering_ips();
    }
    goto update_pinstate_and_return;
  }
  else if (sv_frame_polarity == LOW && sv_pin_state_frame_polarity != sv_frame_polarity) {
    // Frame polarity transitioned from high to low.
    sv_currentField++;
    sv_currentLine = 0;
    sv_currentPixel = 0;
  }

update_pinstate_and_return:
  sv_pin_state_clock = sv_pixel_clock;
  sv_pin_state_line_latch = sv_line_latch;
  sv_pin_state_frame_polarity = sv_frame_polarity;
}

//////////////////////////////////////////////////////////////////////////
void ips_hsync() {
  ips_hsync(true);
}

//////////////////////////////////////////////////////////////////////////
void ips_hsync(bool reset_line) {
  if (ips_currentLine == 144) {
    return;
  }

  if (reset_line) {
    ips_currentLine++;
    ips_currentPixel = 0;
  }

  // Push the first pixel so it is ready during the hsync.
  render_ips_frame(false);

  digitalWriteFast(PIN_IPS_HSYNC, HIGH);
  delayNanoseconds(1500);
  digitalWriteFast(PIN_IPS_CLOCK, HIGH);
  delayNanoseconds(80);
  digitalWriteFast(PIN_IPS_CLOCK, LOW);
  delayNanoseconds(1500);
  digitalWriteFast(PIN_IPS_HSYNC, LOW);
}

//////////////////////////////////////////////////////////////////////////
void ips_vsync1_start() {
  ips_vsyncTimer.begin(ips_frame_rendered, VSYNC_TIMER_DELAY);

  // vsync start
  digitalWriteFast(PIN_IPS_VSYNC, HIGH);
  ips_vsyncEndTimer.begin(ips_vsync1_end, VSYNC_END_TIMER_DELAY);
  
  delayMicroseconds(18);
  ips_hsyncTimer.begin(ips_hsync, HSYNC_TIMER_DELAY);
  
  ips_hsync(false);
}

//////////////////////////////////////////////////////////////////////////
void ips_vsync1_end() {
  // Vsync cycle of 108 uS is done.
  ips_vsyncEndTimer.end();
  digitalWriteFast(PIN_IPS_VSYNC, LOW);
}

//////////////////////////////////////////////////////////////////////////
void ips_frame_rendered() {
  // Frame is rendered, end the timers and reset state so we are ready for the next frame.
  ips_hsyncTimer.end();
  ips_vsyncTimer.end();

  if (rendering_intro) {
    rendering_intro = false;
    wait_for_intro_time = millis();
  }

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