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

Before I go into more detail about how this mod is achieved, let me start by telling you that it is far from perfect and has a few flaws. 

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

### The screen 

I first tried to find suitable IPS screens but soon realized that going with just an IPS would require a lot of additional work and I could just go with the same IPS mod board that I used for the Mega Duck project:  
The DMG RIPS (v5) mod for Game Boy, which is an IPS screen combined with an FPGA-based controller that is driven by Game Boy LCD signals.  

A big advantage of using the RIPS board is that the controller does much of the heavy lifting and provides a color scheme feature (render the monochrome shades in color).  
Another advantage is that, if I want to mod other handhelds in the future, I only have to translate their LCD signals to Game Boy signals and I can use this same RIPS board for the IPS screen.

Although there are a lot of advantages, there is one huge downside which is that the RIPS board only supports a 160x144 resolution. This cannot be circumvented without changing the FPGA code which is pretty much impossible.
I decided to accept this as a flaw and continue with this approach.

### The microcontroller

Next challenge was the microcontroller that sits inbetween the Supervision motherboard and the IPS screen.  
I started with a simple 8-bit Arduino (Atmel 328) but quickly found out that it is not fast enough to capture the data coming from the Supervision motherboard, let alone do both capturing and rendering the new data.

I dabbled around with an ESP32 board before settling on a Teensy 4.0.  
The Teensy has a 600 MHz CPU, plenty of I/O and can be programmed using the Arduino library.


## Supervision LCD connector pinout

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

Supervision LCD pins:

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