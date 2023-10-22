#ifndef _ILI948x_t40_p_H_
#define _ILI948x_t40_p_H_

// uncomment below the line corresponding to your screen:

//#define ILI9481_1
//#define ILI9481_2
#define ILI9486
//#define ILI9488
//#define R61529

#include "Arduino.h"
#include "FlexIO_t4.h"

#define BUS_WIDTH 9
#define SHIFTNUM 4 // number of shifters used (must be 1, 2, 4, or 8)
#define SHIFTER_DMA_REQUEST 3 // only 0, 1, 2, 3 expected to work


#define DATABUFBYTES (480*320)/4


#define _TFTWIDTH   320      // ILI9488 TFT width in default rotation
#define _TFTHEIGHT  480      // ILI9488 TFT height in default rotation

#define ILI9488_NOP         0x00  // No-op
#define ILI9488_SWRESET     0x01  // Software reset
#define ILI9488_RDDID       0x04  // Read display ID
#define ILI9488_RDDST       0x09  // Read display status

#define ILI9488_SLPIN       0x10  // Enter Sleep Mode
#define ILI9488_SLPOUT      0x11  // Sleep Out
#define ILI9488_PTLON       0x12  // Partial Mode ON
#define ILI9488_NORON       0x13  // Normal Display Mode ON

#define ILI9488_RDMODE      0x0A  // Read Display Power Mode
#define ILI9488_RDMADCTL    0x0B  // Read Display MADCTL
#define ILI9488_RDCOLMOD    0x0C  // Read Display Pixel Format
#define ILI9488_RDIMGFMT    0x0D  // Read Display Image Mode
#define ILI9488_RDDSM       0x0E  // Read Display Signal Mode
#define ILI9488_RDSELFDIAG  0x0F  // Read Display Self-Diagnostic Result

#define ILI9488_INVOFF      0x20  // Display Inversion OFF
#define ILI9488_INVON       0x21  // Display Inversion ON
#define ILI9488_GAMMASET    0x26  // Gamma Set
#define ILI9488_DISPOFF     0x28  // Display OFF
#define ILI9488_DISPON      0x29  // Display ON

#define ILI9488_CASET       0x2A  // Column Address Set 
#define ILI9488_PASET       0x2B  // Page Address Set 
#define ILI9488_RAMWR       0x2C  // Memory Write 
#define ILI9488_RAMRD       0x2E  // Memory Read

#define ILI9488_PTLAR       0x30  // Partial Area
#define ILI9488_TEOFF       0x34  // Tearing effect line off
#define ILI9488_TEON        0x35  // Tearing effect line on
#define ILI9488_MADCTL      0x36  // Memory Access Control
#define ILI9488_VSCRSADD    0x37  // Vertical Scrolling Start Address
#define ILI9488_COLMOD      0x3A  // Interface pixel format

#define ILI9488_TESLWR      0x44  // Write tear scan line

#define ILI9488_FRMCTR1     0xB1  // Frame Rate Control (Normal Mode / Full Colors)
#define ILI9488_FRMCTR2     0xB2  // Frame Rate Control (Idle Mode / 8 Colors)
#define ILI9488_FRMCTR3     0xB3  // Frame Rate Control (Partial Mode / Full Colors)
#define ILI9488_INVCTR      0xB4  // Display Inversion Control
#define ILI9488_DFUNCTR     0xB6  // Display Function Control
#define ILI9488_ETMOD       0xB7  // Entry Mode Set

#define ILI9488_PWCTR1      0xC0  // Power Control 1
#define ILI9488_PWCTR2      0xC1  // Power Control 2
#define ILI9488_PWCTR3      0xC2  // Power Control 3 (For Normal Mode)
#define ILI9488_PWCTR4      0xC3  // Power Control 4 (For Idle Mode)
#define ILI9488_PWCTR5      0xC4  // Power Control 5 (For Partial Mode)
#define ILI9488_VMCTR1      0xC5  // VCOM Control
#define ILI9488_CABCCTRL1   0xC6  // CABC Control 1
#define ILI9488_CABCCTRL2   0xC8  // CABC Control 2

#define ILI9488_PGAMCTRL    0xE0  // Positive Gamma Control
#define ILI9488_NGAMCTRL    0xE1  // Negative Gamma Control
#define ILI9488_SETIMAGE    0xE9  // Set Image Function

#define ILI9488_RDID1       0xDA  // Read ID1 value
#define ILI9488_RDID2       0xDB  // Read ID2 value
#define ILI9488_RDID3       0xDC  // Read ID3 value

#define MADCTL_MY  0x80  // Bottom to top
#define MADCTL_MX  0x40  // Right to left
#define MADCTL_MV  0x20  // Row/Column exchange
#define MADCTL_ML  0x10  // LCD refresh Bottom to top
#define MADCTL_RGB 0x00  // Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  // Blue-Green-Red pixel order
#define MADCTL_MH  0x04  // LCD refresh right to left
#define MADCTL_GS  0x01
#define MADCTL_SS  0x02

//MADCTL 0,1,2,3 for setting rotation and 4 for screenshot
#if defined (ILI9488) || defined (ILI9486)
#define MADCTL_ARRAY { MADCTL_MX | MADCTL_BGR, MADCTL_MV | MADCTL_BGR, MADCTL_MY | MADCTL_BGR, MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR, MADCTL_MY | MADCTL_MV | MADCTL_BGR } // ILI9488/9486
#elif defined (ILI9481_1) || defined (ILI9481_2)
#define MADCTL_ARRAY { MADCTL_BGR | MADCTL_SS, MADCTL_MV | MADCTL_BGR, MADCTL_BGR | MADCTL_GS, MADCTL_MV | MADCTL_BGR | MADCTL_SS | MADCTL_GS } // ILI9481
#elif defined (R61529)
#define MADCTL_ARRAY { MADCTL_RGB, MADCTL_MV | MADCTL_MX | MADCTL_RGB, MADCTL_RGB | MADCTL_GS | MADCTL_MX, MADCTL_MV | MADCTL_RGB | MADCTL_GS } // R61529
#endif

#ifdef __cplusplus
class ILI948x_t40_p {
  public:
    ILI948x_t40_p(int8_t dc, int8_t cs = -1, int8_t rst = -1);
    void begin(uint8_t buad_div = 20);
    uint8_t getBusSpd();


    uint8_t setBitDepth(uint8_t bitDepth);
    uint8_t getBitDepth();

    void setFrameRate(uint8_t frRate);
    uint8_t getFrameRate();

    void setTearingEffect(bool tearingOn);
    bool getTearingEffect();

    void setTearingScanLine(uint16_t scanLine);
    uint16_t getTearingScanLine();

    void setRotation(uint8_t r);
    void invertDisplay(bool invert);
    void displayInfo();
    void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void pushPixels16bit(const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void pushPixels8bit(const uint8_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);//


    uint8_t readCommand(uint8_t const cmd);
    
    //void pushPixels16bitTearing(uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2 );
    //void pushPixels24bitTearing(uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2 );
    void DMAerror();

    typedef void(*CBF)();
    CBF _callback;
    void onCompleteCB(CBF callback);
    
  private:

  FlexIOHandler *pFlex;
  IMXRT_FLEXIO_t *p;
  const FlexIOHandler::FLEXIO_Hardware_t *hw;
  
   
    uint8_t _buad_div = 20; 

    uint8_t _bitDepth = 16;
    uint8_t _rotation = 0;
    const uint8_t MADCTL[5] = MADCTL_ARRAY;

    uint8_t _frameRate = 60;

    bool _bTearingOn = false;
    uint16_t _tearingScanLine = 0;

    int16_t _width, _height;
    int8_t  _dc, _cs, _rst;

    uint8_t _dummy;
    uint8_t _curMADCTL;

    uint16_t _lastx1, _lastx2, _lasty1, _lasty2;

    volatile bool WR_DMATransferDone = true;
    uint32_t MulBeatCountRemain;
    uint16_t *MulBeatDataRemain;
    uint32_t TotalSize; 

    void displayInit();
    void CSLow();
    void CSHigh();
    void DCLow();
    void DCHigh();
    void gpioWrite();
    void gpioRead();
    
    void FlexIO_Init();
    void FlexIO_Config_SnglBeat();
    void FlexIO_Clear_Config_SnglBeat();
    void FlexIO_Config_SnglBeat_Read();

    void SglBeatWR_nPrm_8(uint32_t const cmd, uint8_t const *value , uint32_t const length);
    void SglBeatWR_nPrm_16(uint32_t const cmd, const uint16_t *value, uint32_t const length);
  

    void microSecondDelay();
    
    static ILI948x_t40_p *dmaCallback;
    
};
#endif //__cplusplus
#endif //_IILI948x_t40_p_H_