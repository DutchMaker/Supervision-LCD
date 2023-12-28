# Watara Supervision IPS mod build details

The Watara Supervision is a hidden gem from the past.  
Although it is a pretty good handheld console with a decent game library but like many other consoles from the era, it has a horrible dot matrix LCD.

Having previously replaced a similar old LCD of the Mega Duck, a similar handheld console from the same era, I went on a mission to do the same with the Supervision: replace the stock LCD with a modern IPS screen.

This article describes how I did it and how you could replicate it.

<!--
 - Approach
  - Parts list
- Flaws
- Overview of LCD protocol
- Timing / synchronisation challenge
- Teensy code
- Schematic for electronics 
-->

## This mod has flaws

Before I go into the details about how this mod is achieved, let me start by telling you that it is far from perfect and has a few flaws. 

### 1. Lower resolution
The IPS display has a 160x144 resolution whereas the Supervision uses 160x160. The mod will simply ignore every 10th row of pixels thereby omitting 16 rows of pixels in total, leaving 144 rows.  

Missing 10% of the vertical pixels is clearly visible in the resulting picture and is the most obvious and annoying flaw.

### 2. Loss of framerate
The mod cannot process the Supervision LCD data, translate it and write back to the IPS screen in the same time that the Supervision generates a frame. We need processing time and therefore skip a frame, resulting in half the framerate (25 FPS).

### 3. Boot sequence
The code that translates the signal from the Supervision to the IPS board needs the Supervision to boot in a certain state in order to generate the correct shading in the picture.  
The Teensy board starts the Supervision and tries to detect an incorrect boot state and then restarts the Supervision if needed. However, this fails sometimes and that will boot the mod in a state in which you get only black and white color and you miss part of the frame data.

This happens in maybe 5% of the cases or less and it is solved by turning the console off and on again.

## Approach

The initial plan was simple: find an IPS display that fits the Supervision shell, hook up a microcontroller to the Supervision to read the LCD signals, translate it to signals compatible with the IPS and pass it through.
In reality, it turned out to be quite challenging.

### Parts list

- [DMG RIPS kit (larger size)](https://www.aliexpress.com/item/1005001509799866.html)\
- [Teensy 4.0 development board](https://www.pjrc.com/store/teensy40.html)
- [Bi-directional level shifter TXS0108E](https://www.tinytronics.nl/shop/en/communication-and-signals/level-converters/txs0108e-spi-i2c-uart-bi-directionele-logic-level-converter-8-channel)
- [Solid State Relay LBA110 DPST 120mA 350V](https://eu.mouser.com/ProductDetail/IXYS-Integrated-Circuits/LBA110?qs=nnG3q7dUVNxIzz7QSkkObA%3D%3D)

### The screen 

I first tried to find suitable IPS screens but soon realized that going with just an IPS would require a lot of additional work and I could just go with the same IPS mod board that I used for the Mega Duck project:  
The [DMG RIPS (v5)](https://www.aliexpress.com/item/1005001509799866.html) mod for Game Boy, which is an IPS screen combined with an FPGA-based controller that is driven by Game Boy LCD signals.  

A big advantage of using the RIPS board is that the controller does much of the heavy lifting and provides a color scheme feature (render the monochrome shades in color).  
Another advantage is that, if I want to mod other handhelds in the future, I only have to translate their LCD signals to Game Boy signals and I can use this same RIPS board for the IPS screen.

Although there are a lot of advantages, there is one huge downside which is that the RIPS board only supports a 160x144 resolution. This cannot be circumvented without changing the FPGA code which is pretty much impossible.
I decided to accept this as a flaw and continue with this approach.

### The microcontroller

The next challenge was the microcontroller that sits inbetween the Supervision motherboard and the IPS screen.  
I started with a simple 8-bit Arduino (Atmel 328) but quickly found out that it is not fast enough to capture the data coming from the Supervision motherboard, let alone do both capturing and rendering the new data.

I dabbled around with an ESP32 board before settling on a Teensy 4.0.  
The Teensy has a 600 MHz CPU and although that does not mean you can do I/O at that speed, it is still fast enough for what we are doing.
It is also very easy to program for as it supports the Arduino library.

## Pinouts

### Supervision LCD connector pinout

```
    1  2  3  4  5  6  7  8  9  10 11 12
    |  |  |  |  |  |  |  |  |  |  |  |
.---|--|--|--|--|--|--|--|--|--|--|--|---.
|   |  |  |  |  |  |  |  |  |  |  |  |   |
|   o  o  o  o  o  o  o  o  o  x  o  x   |
|            <LCD connector>             |
|                                        |
|              | | | | | |               |
|            -             -             |
|            -    C P U    -             |
|            -  (top side) -             |
|            -             -             |
|              | | | | | |               |
|                                        |
|      <controller board connection>     |
`----------------------------------------`
```

**Supervision LCD pins:**

1. Ground
2. Data 0
3. Data 1
4. Data 2
5. Data 3
6. Pixel clock
7. Line latch
8. Frame latch
9. Frame polarity
10. Unused (power control)
11. +6V
12. Unused (no idea what it does)

### DMG RIPS (IPS) connector pinout

1. Unused
2. +5V
3. GND
4. +5V
5. +5V
6. GND
7. Unused
8. HSYNC
9. D0
10. D1
11. CLOCK
12. VSYNC
13. GND
14. Unused
15. Rotary encoder button
16. Rotary encoder button
17. Rotary encoder button

## LCD protocols

Work in progress...

### GameBoy LCD protocol

Work in progress...

### Supervision LCD protocol

Work in progress...

## Synchronization challenge

Work in progress...

## Teensy code

The C++ code for the Teensy [is located here](../firmware/src/main.cpp) as a Platform.io project for Visual Studio Code.  

## Electronics schematic

Haven't had the time to do an actual schematic yet but it is fairly straight forward:  
The Supervision LCD pins are connected to the Teensy through the level shifter (bringing down the 5V signal from the Supervision to 3.3V which is used by the Teensy).  
The IPS screen pins are connected directly to the Teensy (the Teensy drives the LCD using 3.3V signals).  
The Supervision 5V power line (from the batteries to the motherboard) is cut so it can be controlled by a Relays that is turned on by the Teensy.
Pinouts are all defined in code and described above.

Here is a photo of everything laid out on a breadboard:

<img src="https://github.com/DutchMaker/Supervision-LCD/blob/main/docs/images/breadboard.jpg" />