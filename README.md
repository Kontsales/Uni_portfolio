This repository acts as a portfolio of code written by me during the studies for my M.Sc (Tech.) in Electrical Engineering at LUT-University. 
As I specialized in embedded systems, most of the code is from different course assignments utilizing either an Arduino or the Zybo z7-10 development board.
Many of the assignments were completed in a group so the code provided might not constitute a complete project, but showcases only the code I was responsible for.

Here is a brief introduction to each of the projects provided:

FPGA audio codec:
- Utilized HW: Zybo-Z7-10 (FPGA)
- Language: VHDL
- I2S communication utilizing the onboard SSM2603 audio codec with configuration and operation implemented in VHDL.

FPGA reaction game:
- Utilized HW: Zybo z7-10 (FPGA)
- Language: VHDL
- A simple reaction game utilizing onboard buttons and LEDs implemented in VHDL.

PI-controller:
- Utilized HW: Zybo z7-10 (ARM Cortex A9)
- Language: C
- PI-controller for an inverter simulation utilizing onboard buttons and interrupts or uart communication for configuration and onboard LEDs for output.

Radio switch for marine vessel
- Utilized HW: Atmega8L
- Language: Arduino
- Control logic and user-interface for a device allowing 4 handheld radios to be connected to a single speaker system. Utilizes register level timer control and interrupts.
