#include <ILI948x_t40_p.h>
#include "flexio_teensy_mm.c"

ILI948x_t40_p lcd = ILI948x_t40_p(13, 11, 12); //(dc, cs, rst)

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.print(CrashReport);
  
  lcd.begin();
  lcd.setBitDepth(16);
  lcd.setRotation(3);

  lcd.displayInfo();
}

void loop() {
  delay(1000);
  lcd.pushPixels16bit(flexio_teensy_mm,0,0,479,319); // 480x320
}

// ILI948x_t40_p lcd = ILI948x_t40_p(ILI9486_DISP, 7, 6, 5); //(dc, cs, rst)


// void setup() {
//   Serial.begin(115200);
//   delay(3000);
//   Serial.print(CrashReport);
//   pinMode(4, OUTPUT);
//   digitalWrite(4, HIGH);
  
//   lcd.begin(20);
//   lcd.displayInfo();
//   lcd.setRotation(3);
// }

// void loop() {
//   lcd.pushPixels16bit(teensy41,0,0,479,319); // 480x320
//   delay(1000);
//   lcd.pushPixels16bitAsync(teensy41,0,0,479,319); // 480x320
//   delay(1000);
// }
