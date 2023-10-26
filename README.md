# Watara Supervision custom LCD

The goal of this project is to replace the stock LCD screen of the Watara Supervision with a custom IPS screen.  
The current approach is as follows:

- Use the GameBoy RIPS v5 screen mod
  - This contains an IPS screen plus a driver board with firmware that accepts GameBoy LCD signals as input.
  - Huge advantage using this mod is that is easily sourced and it already supports colour schemes.
- Use a Teensy 4 to translate the Supervision LCD signals to GameBoy LCD signals to drive the RIPS mod.
