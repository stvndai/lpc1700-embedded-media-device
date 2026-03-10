# LPC1700 Media Device
A bare metal multimedia device implemented in C on an ARM Cortex-M3 LPC1700 microcontroller. Built as part of the COE718 Embedded Systems course at Toronto Metropolitan University.
Overview
The Media Center is a joystick-navigated menu system running on the LPC1700 development board that integrates three distinct subsystems: a photo gallery, a USB audio player, and a car dodging game — all rendered on an onboard GLCD display.
No operating system or middleware. All control flow, peripheral interfacing, and display rendering is handled directly in C.
Features

Joystick-driven menu navigation — directional and center-press input handled via polling in the main loop, with a debounce flag (changeOption) to prevent multiple triggers per press
Photo Gallery — displays three bitmap images stored in flash memory using GLCD_Bitmap(), navigated with directional joystick inputs
USB Audio Player — streams audio from a host PC over USB, using Timer0 interrupts for sample-rate timing, ADC input, and DAC output on the LPC1700. USB stack initialized and managed with USB_Init(), USB_Connect(), and NVIC interrupt enabling
Car Dodging Game — real-time game loop with joystick-controlled player movement, randomized wall generation, boundary detection, and pixel-level collision detection using sprite bitmaps rendered via GLCD_Bitmap()

Hardware

Board: NXP LPC1700 (ARM Cortex-M3)
Display: Onboard GLCD via GLCD library
Input: Onboard 5-way joystick
Audio: USB connection to host PC, onboard DAC (P0.26/AOUT)
Clock: SysTick configured at 10ms intervals, Timer0 used for audio sample rate

Technical Details

Audio DAC output on pin P0.26, ADC input on P0.25, configured via PINSEL1 registers directly
Timer0 match register set to pclk / DATA_FREQ - 1 for precise audio sample timing
PCLK divider read from PCLKSEL0 to correctly derive peripheral clock before setting timer
USB audio returns to menu on joystick center press via polling inside the audio loop
Game uses rand() for wall spawn position and manual coordinate math for collision detection

Known Limitation
Returning to the main menu during audio playback required polling the joystick inside the audio_main() loop. While this works for menu exit, it means the audio subsystem fully occupies the main thread during playback — a proper fix would use an interrupt-driven audio pipeline with a state machine architecture, or an RTOS task for audio handling, allowing the menu to remain responsive without blocking.
Tools

Keil MDK / µVision
CMSIS
GLCD, LED, ADC, Joystick board support libraries

Language
C — bare metal, no OS
