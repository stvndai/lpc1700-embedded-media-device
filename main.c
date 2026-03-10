#include <stdio.h>
#include <stdlib.h>
#include "Blinky.h"
#include "LPC17xx.h"
#include "GLCD.h"
#include "LED.h"
#include "Board_ADC.h"
#include "Board_Joystick.h"

#define __FI        1       /* Font index 16x24 */
#define __USE_LCD   0       /* Uncomment to use the LCD */

/* ITM Stimulus Port definitions for printf */
#define ITM_Port8(n)    (*((volatile unsigned char  *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short *)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long  *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

int fputc(int ch, FILE *f) {
    if (DEMCR & TRCENA) {
        while (ITM_Port32(0) == 0);
        ITM_Port8(0) = ch;
    }
    return ch;
}

/* Global state */
int option;
int changeOption;

extern int music_play(void);

static volatile uint16_t AD_dbg;

uint16_t ADC_last;          /* Last converted value */
extern uint8_t clock_ms;    /* Imported from IRQ.c */

/* Function prototypes */
void game(void);
void photoGallery(void);
void musicPlayer(void);
extern int audio_main(void);

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main(void) {

    LED_Init();     /* LED Initialization */

#ifdef __USE_LCD
    GLCD_Init();
    GLCD_Clear(White);
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(0, 0, __FI, "    Media Center    ");
    GLCD_DisplayString(1, 0, __FI, "   COE718 demo      ");
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Blue);
    GLCD_DisplayString(4, 0, __FI, "Photo Gallery");
    GLCD_DisplayString(6, 0, __FI, "MP3 Player");
    GLCD_DisplayString(8, 0, __FI, "Car game");
#endif

    SysTick_Config(SystemCoreClock / 100);  /* Generate interrupt each 10 ms */
    option = 0;

    while (1) {

        /* Wrap menu option */
        if (option == 3) {
            option = 0;
        } else if (option == -1) {
            option = 3;
        }

        /* Joystick navigation */
        if (Joystick_GetState() == JOYSTICK_DOWN) {
            if (changeOption == 1) {
                changeOption = 0;
                option++;
            }
            LED_Out(1);
        } else if (Joystick_GetState() == JOYSTICK_UP) {
            if (changeOption == 1) {
                changeOption = 0;
                option--;
            }
            LED_Out(2);
        } else if (Joystick_GetState() == JOYSTICK_CENTER) {
            LED_Out(16);

            switch (option) {
                case 0:
                    photoGallery();
                    break;
                case 1:
                    musicPlayer();
                    break;
                case 2:
                    game();
                    break;
            }
        } else {
            changeOption = 1;
        }

        /* Highlight selected menu item */
        switch (option) {
            case 0:
                GLCD_SetBackColor(Blue);
                GLCD_SetTextColor(White);
                GLCD_DisplayString(4, 0, __FI, "Photo Gallery");
                GLCD_SetBackColor(White);
                GLCD_SetTextColor(Blue);
                GLCD_DisplayString(6, 0, __FI, "MP3 Player");
                GLCD_DisplayString(8, 0, __FI, "Car Game");
                break;

            case 1:
                GLCD_SetBackColor(Blue);
                GLCD_SetTextColor(White);
                GLCD_DisplayString(6, 0, __FI, "MP3 Player");
                GLCD_SetBackColor(White);
                GLCD_SetTextColor(Blue);
                GLCD_DisplayString(8, 0, __FI, "Car Game");
                GLCD_DisplayString(4, 0, __FI, "Photo Gallery");
                break;

            case 2:
                GLCD_SetBackColor(Blue);
                GLCD_SetTextColor(White);
                GLCD_DisplayString(8, 0, __FI, "Car Game");
                GLCD_SetBackColor(White);
                GLCD_SetTextColor(Blue);
                GLCD_DisplayString(6, 0, __FI, "MP3 Player");
                GLCD_DisplayString(4, 0, __FI, "Photo Gallery");
                break;
        }

        /* Redraw header */
        GLCD_SetBackColor(Black);
        GLCD_SetTextColor(White);
        GLCD_DisplayString(0, 0, __FI, "    Media Center    ");
        GLCD_DisplayString(1, 0, __FI, "   COE718 demo      ");
        GLCD_SetBackColor(White);
        GLCD_SetTextColor(Blue);
    }
}

/*----------------------------------------------------------------------------
  Car Dodging Game
 *----------------------------------------------------------------------------*/
void game(void) {

    extern unsigned char PLAYER[];
    extern unsigned char WALL[];

    unsigned int playerTop   = 210;
    unsigned int playerLeft  = 140;
    unsigned int playerRight = playerLeft + 23;

    unsigned int wallLeft    = 20;
    unsigned int wallRight   = wallLeft + 61;
    unsigned int wallBottom  = 30;
    unsigned int keepRunning = 1;

    GLCD_Clear(White);

    /* Start screen */
    while (keepRunning) {
        GLCD_SetBackColor(Blue);
        GLCD_SetTextColor(White);
        GLCD_DisplayString(3, 0, __FI, "use joystick to move");
        GLCD_DisplayString(5, 0, __FI, "press center start");
        GLCD_DisplayString(7, 0, __FI, "press up go back");

        if (Joystick_GetState() == JOYSTICK_CENTER) {
            GLCD_Clear(White);
            keepRunning = 0;
        } else if (Joystick_GetState() == JOYSTICK_UP) {
            return;
        }
    }

    /* Game loop */
    keepRunning = 1;
    while (keepRunning) {

        /* Reset wall when it reaches bottom */
        if (wallBottom == 220) {
            wallBottom = 20;
            wallLeft   = (rand() % (0 - 235 + 1)) + 0;
            wallRight  = wallLeft + 61;
        }

        /* Player movement */
        if (Joystick_GetState() == JOYSTICK_LEFT) {
            playerLeft -= 5;
        } else if (Joystick_GetState() == JOYSTICK_RIGHT) {
            playerLeft += 5;
        }

        /* Boundary clamping */
        if (playerLeft <= 6) {
            playerLeft += 10;
        }
        if (playerRight >= 315) {
            playerLeft -= 10;
        }

        /* Sync derived positions */
        playerRight = playerLeft + 23;
        wallRight   = wallLeft + 61;

        /* Collision detection */
        if (wallBottom >= playerTop - 18) {
            if ((playerLeft < wallRight) && (playerRight > wallRight)) {
                keepRunning = 0;
            }
            if ((playerRight > wallLeft) && (playerLeft < wallLeft)) {
                keepRunning = 0;
            }
            if ((playerLeft > wallLeft) && (playerRight < wallRight)) {
                keepRunning = 0;
            }
        }

        wallBottom += 10;

        GLCD_Clear(White);
        GLCD_Bitmap(playerLeft, playerTop, 23, 20, PLAYER);
        GLCD_Bitmap(wallLeft, wallBottom, 61, 15, WALL);
    }

    /* Game over screen */
    keepRunning = 1;
    GLCD_Clear(White);

    while (keepRunning) {
        GLCD_SetBackColor(Blue);
        GLCD_SetTextColor(White);
        GLCD_DisplayString(4, 0, __FI, "game over");
        GLCD_DisplayString(6, 0, __FI, "center to menu");

        if (Joystick_GetState() == JOYSTICK_CENTER) {
            GLCD_Clear(White);
            keepRunning = 0;
        }
    }
}

/*----------------------------------------------------------------------------
  Photo Gallery
 *----------------------------------------------------------------------------*/
void photoGallery(void) {

    extern unsigned char PIC0[];
    extern unsigned char PIC1[];
    extern unsigned char PIC2[];

    int keeprunning = 1;

    GLCD_Clear(White);
    GLCD_SetBackColor(Blue);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(4, 0, __FI, "Use joystick to view");
    GLCD_DisplayString(5, 0, __FI, "down: picture 1");
    GLCD_DisplayString(6, 0, __FI, "left: picture 2");
    GLCD_DisplayString(7, 0, __FI, "right: picture 3");
    GLCD_DisplayString(8, 0, __FI, "center: menu");

    while (keeprunning) {

        if (Joystick_GetState() == JOYSTICK_DOWN) {
            GLCD_Clear(White);
            GLCD_Bitmap(0, 0, 320, 240, PIC0);

        } else if (Joystick_GetState() == JOYSTICK_RIGHT) {
            GLCD_Clear(White);
            GLCD_Bitmap(0, 0, 320, 240, PIC1);

        } else if (Joystick_GetState() == JOYSTICK_LEFT) {
            GLCD_Clear(White);
            GLCD_Bitmap(0, 0, 320, 240, PIC2);

        } else if (Joystick_GetState() == JOYSTICK_UP) {
            GLCD_Clear(White);
            GLCD_SetBackColor(Blue);
            GLCD_SetTextColor(White);
            GLCD_DisplayString(4, 0, __FI, "Use joystick to view");
            GLCD_DisplayString(5, 0, __FI, "down: picture 1");
            GLCD_DisplayString(6, 0, __FI, "left: picture 2");
            GLCD_DisplayString(7, 0, __FI, "right: picture 3");
            GLCD_DisplayString(8, 0, __FI, "center: menu");

        } else if (Joystick_GetState() == JOYSTICK_CENTER) {
            GLCD_Clear(White);
            keeprunning = 0;
        }
    }
}

/*----------------------------------------------------------------------------
  Music Player
 *----------------------------------------------------------------------------*/
void musicPlayer(void) {
    audio_main();
}

/*----------------------------------------------------------------------------
  Audio Main - USB audio streaming with Timer0 sample-rate control
 *----------------------------------------------------------------------------*/
int audio_main(void) {

    volatile uint32_t pclkdiv, pclk;

    SystemClockUpdate();

    /* Configure P0.25 as ADC input (AIN2) and P0.26 as DAC output (AOUT) */
    LPC_PINCON->PINSEL1 &= ~((0x03 << 18) | (0x03 << 20));
    LPC_PINCON->PINSEL1 |=  ((0x01 << 18) | (0x02 << 20));

    /* Enable ADC peripheral clock */
    LPC_SC->PCONP |= (1 << 12);

    LPC_ADC->CR = 0x00200E04;   /* ADC: 10-bit AIN2 @ 4MHz */
    LPC_DAC->CR = 0x00008000;   /* DAC output set to midpoint */

    /* Derive peripheral clock for Timer0 match register */
    pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
    switch (pclkdiv) {
        default:
        case 0x00: pclk = SystemFrequency / 4; break;
        case 0x01: pclk = SystemFrequency;     break;
        case 0x02: pclk = SystemFrequency / 2; break;
        case 0x03: pclk = SystemFrequency / 8; break;
    }

    /* Configure Timer0 for audio sample rate */
    LPC_TIM0->MR0 = pclk / DATA_FREQ - 1;  /* Match value for sample rate */
    LPC_TIM0->MCR = 3;                      /* Interrupt and reset on MR0 */
    LPC_TIM0->TCR = 1;                      /* Enable Timer0 */

    /* Display connected screen */
    GLCD_Init();
    GLCD_Clear(Blue);
    GLCD_SetTextColor(White);
    GLCD_SetBackColor(Blue);
    GLCD_DisplayString(2, 0, 0, " Connected ");
    GLCD_DisplayString(3, 0, 0, " play music ");
    GLCD_DisplayString(5, 0, 0, " joystick center ");
    GLCD_DisplayString(6, 0, 0, " to main menu ");

    /* Initialize USB audio */
    NVIC_EnableIRQ(TIMER0_IRQn);
    USB_Init();
    NVIC_EnableIRQ(USB_IRQn);
    USB_Reset();
    USB_SetAddress(0);
    USB_Connect(TRUE);

    /*
     * Known limitation: audio playback occupies this loop entirely.
     * Menu exit works via joystick polling, but the main thread is blocked
     * during playback. A proper fix would use an interrupt-driven audio
     * pipeline or RTOS task to allow concurrent menu responsiveness.
     */
    while (1) {
        if (Joystick_GetState() == JOYSTICK_CENTER) {
            return 0;
        }
    }
}
