/*
        Uses hardware registers for fast speed.
*/

#include "Adafruit_ILI9341_8bit.h"

void RD_STROBE() {
  RD_ACTIVE;
  RD_IDLE;
}

void WR_STROBE() {
  WR_ACTIVE;
  WR_IDLE;
}

void swap(int16_t a, int16_t b) {
  int16_t t = a;
  a = b;
  b = t;
}

void write8special(uint8_t c) {
  digitalWriteFast(TFT_WR, LOW);
  *((volatile uint8_t *)(&GPIOC_PDOR)) = c;
  digitalWriteFast(TFT_WR, HIGH);
  // NOPs needed here because changes on 8 bit bus are usually to quick for driver to respond
  // You can remove these NOPs if you wish but without them weird glitches are bound to occur.
  asm("NOP");  // wait ten ns
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

// Constructor when using 8080 mode of control.

Adafruit_ILI9341_8bit_STM::Adafruit_ILI9341_8bit_STM(void)

    : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  pinMode(TFT_RD, OUTPUT);

  pinMode(TFT_WR, OUTPUT);

  pinMode(TFT_RS, OUTPUT);

  pinMode(TFT_CS, OUTPUT);

  CS_IDLE;  // Set all control bits to HIGH (idle)

  CD_DATA;  // Signals are ACTIVE LOW

  WR_IDLE;

  RD_IDLE;

  if (TFT_RST) {
    pinMode(TFT_RST, OUTPUT);

    digitalWriteFast(TFT_RST, HIGH);
  }

  // set up 8 bit parallel port to write mode.

  setWriteDataBus();
}

void Adafruit_ILI9341_8bit_STM::setWriteDataBus(void) {
  // set the pins to output mode

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
}

void Adafruit_ILI9341_8bit_STM::setReadDataBus(void) {
  // set the pins to input mode

  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
}

void Adafruit_ILI9341_8bit_STM::write8(uint8_t c) {
  CS_ACTIVE;

  digitalWriteFast(TFT_WR, LOW);
  *((volatile uint8_t *)(&GPIOC_PDOR)) = c;
  digitalWriteFast(TFT_WR, HIGH);
  asm("NOP");  // wait ten ns
  asm("NOP");
  asm("NOP");
  asm("NOP");

  CS_IDLE;
}

void Adafruit_ILI9341_8bit_STM::writecommand(uint8_t c) {
  CD_COMMAND;

  write8(c);
}

void Adafruit_ILI9341_8bit_STM::writedata(uint8_t c) {
  CD_DATA;

  write8(c);
}

void Adafruit_ILI9341_8bit_STM::begin(void) {
  // toggle RST low to reset

  digitalWrite(TFT_RST, HIGH);

  delay(5);

  digitalWrite(TFT_RST, LOW);

  delay(20);

  digitalWrite(TFT_RST, HIGH);

  delay(150);

  writecommand(0xEF);

  writedata(0x03);

  writedata(0x80);

  writedata(0x02);

  writecommand(0xCF);

  writedata(0x00);

  writedata(0XC1);

  writedata(0X30);

  writecommand(0xED);

  writedata(0x64);

  writedata(0x03);

  writedata(0X12);

  writedata(0X81);

  writecommand(0xE8);

  writedata(0x85);

  writedata(0x00);

  writedata(0x78);

  writecommand(0xCB);

  writedata(0x39);

  writedata(0x2C);

  writedata(0x00);

  writedata(0x34);

  writedata(0x02);

  writecommand(0xF7);

  writedata(0x20);

  writecommand(0xEA);

  writedata(0x00);

  writedata(0x00);

  writecommand(ILI9341_PWCTR1);  // Power control

  writedata(0x23);  // VRH[5:0]

  writecommand(ILI9341_PWCTR2);  // Power control

  writedata(0x10);  // SAP[2:0];BT[3:0]

  writecommand(ILI9341_VMCTR1);  // VCM control

  writedata(0x3e);  //�Աȶȵ���

  writedata(0x28);

  writecommand(ILI9341_VMCTR2);  // VCM control2

  writedata(0x86);  //--

  writecommand(ILI9341_MADCTL);  // Memory Access Control

  writedata(0x48);

  writecommand(ILI9341_PIXFMT);

  writedata(0x55);

  writecommand(ILI9341_FRMCTR1);

  writedata(0x00);

  writedata(0x18);

  writecommand(ILI9341_DFUNCTR);  // Display Function Control

  writedata(0x08);

  writedata(0x82);

  writedata(0x27);

  writecommand(0xF2);  // 3Gamma Function Disable

  writedata(0x00);

  writecommand(ILI9341_GAMMASET);  // Gamma curve selected

  writedata(0x01);

  writecommand(ILI9341_GMCTRP1);  // Set Gamma

  writedata(0x0F);

  writedata(0x31);

  writedata(0x2B);

  writedata(0x0C);

  writedata(0x0E);

  writedata(0x08);

  writedata(0x4E);

  writedata(0xF1);

  writedata(0x37);

  writedata(0x07);

  writedata(0x10);

  writedata(0x03);

  writedata(0x0E);

  writedata(0x09);

  writedata(0x00);

  writecommand(ILI9341_GMCTRN1);  // Set Gamma

  writedata(0x00);

  writedata(0x0E);

  writedata(0x14);

  writedata(0x03);

  writedata(0x11);

  writedata(0x07);

  writedata(0x31);

  writedata(0xC1);

  writedata(0x48);

  writedata(0x08);

  writedata(0x0F);

  writedata(0x0C);

  writedata(0x31);

  writedata(0x36);

  writedata(0x0F);

  writecommand(ILI9341_INVOFF);  // Invert Off

  delay(120);

  writecommand(ILI9341_SLPOUT);  // Exit Sleep

  delay(120);

  writecommand(ILI9341_DISPON);  // Display on
}

void Adafruit_ILI9341_8bit_STM::setAddrWindow(uint16_t x0, uint16_t y0,
                                              uint16_t x1,

                                              uint16_t y1) {
  CS_ACTIVE;

  CD_COMMAND;

  write8special(ILI9341_CASET);  // Column addr set

  CD_DATA;

  write8special(x0 >> 8);

  write8special(x0 & 0xFF);  // XSTART

  write8special(x1 >> 8);

  write8special(x1 & 0xFF);  // XEND

  CD_COMMAND;

  write8special(ILI9341_PASET);  // Row addr set

  CD_DATA;

  write8special(y0 >> 8);

  write8special(y0);  // YSTART

  write8special(y1 >> 8);

  write8special(y1);  // YEND

  CD_COMMAND;

  write8special(ILI9341_RAMWR);  // write to RAM
}

void Adafruit_ILI9341_8bit_STM::pushColor(uint16_t color) {
  writedata(color >> 8);

  writedata(color);
}

void Adafruit_ILI9341_8bit_STM::drawPixel(int16_t x, int16_t y,
                                          uint16_t color) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x, y, x + 1, y + 1);

  CD_DATA;

  write8special(color >> 8);

  write8special(color);

  CS_IDLE;
}

void Adafruit_ILI9341_8bit_STM::drawFastVLine(int16_t x, int16_t y, int16_t h,

                                              uint16_t color) {
  // Rudimentary clipping

  if ((x >= _width) || (y >= _height || h < 1)) return;

  if ((y + h - 1) >= _height) h = _height - y;

  if (h < 2) {
    drawPixel(x, y, color);

    return;
  }

  //  if (hwSPI) spi_begin();

  setAddrWindow(x, y, x, y + h - 1);

  CD_DATA;

  uint8_t hi = color >> 8, lo = color;

  while (h--) {
    write8special(hi);

    write8special(lo);
  }

  CS_IDLE;
}

void Adafruit_ILI9341_8bit_STM::drawFastHLine(int16_t x, int16_t y, int16_t w,

                                              uint16_t color) {
  // Rudimentary clipping

  if ((x >= _width) || (y >= _height || w < 1)) return;

  if ((x + w - 1) >= _width) w = _width - x;

  if (w < 2) {
    drawPixel(x, y, color);

    return;
  }

  //  if (hwSPI) spi_begin();

  setAddrWindow(x, y, x + w - 1, y);

  CD_DATA;

  uint8_t hi = color >> 8, lo = color;

  while (w--) {
    write8special(hi);

    write8special(lo);
  }

  CS_IDLE;
}

void Adafruit_ILI9341_8bit_STM::fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}

// fill a rectangle

void Adafruit_ILI9341_8bit_STM::fillRect(int16_t x, int16_t y, int16_t w,
                                         int16_t h,

                                         uint16_t color) {
  // rudimentary clipping (drawChar w/big text requires this)

  if ((x >= _width) || (y >= _height || h < 1 || w < 1)) return;

  if ((x + w - 1) >= _width) w = _width - x;

  if ((y + h - 1) >= _height) h = _height - y;

  if (w == 1 && h == 1) {
    drawPixel(x, y, color);

    return;
  }

  setAddrWindow(x, y, x + w - 1, y + h - 1);

  CD_DATA;

  uint8_t hi = color >> 8, lo = color;

  for (y = h; y > 0; y--) {
    for (x = w; x > 0; x--) {
      write8special(hi);

      write8special(lo);
    }
  }

  CS_IDLE;
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color

uint16_t Adafruit_ILI9341_8bit_STM::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define MADCTL_MY 0x80

#define MADCTL_MX 0x40

#define MADCTL_MV 0x20

#define MADCTL_ML 0x10

#define MADCTL_RGB 0x00

#define MADCTL_BGR 0x08

#define MADCTL_MH 0x04

void Adafruit_ILI9341_8bit_STM::setRotation(uint8_t m) {
  writecommand(ILI9341_MADCTL);

  rotation = m % 4;  // can't be higher than 3

  switch (rotation) {
    case 0:

      writedata(MADCTL_MX | MADCTL_BGR);

      _width = ILI9341_TFTWIDTH;

      _height = ILI9341_TFTHEIGHT;

      break;

    case 1:

      writedata(MADCTL_MV | MADCTL_BGR);

      _width = ILI9341_TFTHEIGHT;

      _height = ILI9341_TFTWIDTH;

      break;

    case 2:

      writedata(MADCTL_MY | MADCTL_BGR);

      _width = ILI9341_TFTWIDTH;

      _height = ILI9341_TFTHEIGHT;

      break;

    case 3:

      writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);

      _width = ILI9341_TFTHEIGHT;

      _height = ILI9341_TFTWIDTH;

      break;
  }
}

void Adafruit_ILI9341_8bit_STM::invertDisplay(boolean i) {
  writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
}

////////// stuff not actively being used, but kept for posterity

uint8_t Adafruit_ILI9341_8bit_STM::read8(void) {
  RD_ACTIVE;

  delay(5);

  uint8_t temp = 0;

  if (digitalReadFast(D0)) {
    temp |= (1 << 0);
  }  // slow reading but works

  if (digitalReadFast(D1)) {
    temp |= (1 << 1);
  }

  if (digitalReadFast(D2)) {
    temp |= (1 << 2);
  }

  if (digitalReadFast(D3)) {
    temp |= (1 << 3);
  }

  if (digitalReadFast(D4)) {
    temp |= (1 << 4);
  }

  if (digitalReadFast(D5)) {
    temp |= (1 << 5);
  }

  if (digitalReadFast(D6)) {
    temp |= (1 << 6);
  }

  if (digitalReadFast(D7)) {
    temp |= (1 << 7);
  }

  RD_IDLE;

  delay(5);

  return temp;
}

uint8_t Adafruit_ILI9341_8bit_STM::readcommand8(uint8_t c) {
  writecommand(c);

  CS_ACTIVE;

  CD_DATA;

  setReadDataBus();

  delay(5);

  // single dummy data

  uint8_t data = read8();

  // real data

  data = read8();

  setWriteDataBus();

  CS_IDLE;

  return data;
}

uint32_t Adafruit_ILI9341_8bit_STM::readID(void) {
  writecommand(ILI9341_RDDID);

  CS_ACTIVE;

  CD_DATA;

  setReadDataBus();

  uint32_t r = read8();

  r <<= 8;

  r |= read8();

  r <<= 8;

  r |= read8();

  r <<= 8;

  r |= read8();

  setWriteDataBus();

  CS_IDLE;

  return r;
}
