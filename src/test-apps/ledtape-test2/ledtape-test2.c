/* File:   ledtape-test2.c
   Author: B Mitchell, UCECE
   Date:   14 April 2021
   Descr:  Test ledtape
*/

#include <pio.h>
#include "target.h"
#include "pacer.h"
#include "ledbuffer.h"
#include "delay.h"

#define NUM_LEDS 26
#define LEDTAPE_PIO PB0_PIO

/*
    This is an alternative method for driving the LED tape using the ledbuffer
    module that is included in the ledtape driver.

    The buffer acts like a small framebuffer upon which you can set RGB values
    at specific positions (not GRB, it handles the translation automatically).
    It also makes it easy to make patterns, shuffle them allow the strip, and
    clear it later. See ledbuffer.h for more details (CTRL-Click it in VS Code).
*/

int
main (void)
{
    int redValue = 255;
    int greenValue = 0;
    int blueValue = 0;
    int count = 0;

    ledbuffer_t* leds = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);

    pacer_init(10);

    while (1)
    {
        //pacer_wait();
        //ledbuffer_clear(leds);
        for (;redValue >= 0, blueValue < 255; redValue--, blueValue++) 
        {
            ledbuffer_set(leds, NUM_LEDS / 2, redValue, greenValue, blueValue);
            ledbuffer_write (leds);
            ledbuffer_advance (leds, 1);
        }
        for (;blueValue>=0, greenValue < 255; blueValue--, greenValue++)
        {
            ledbuffer_set(leds, NUM_LEDS / 2, redValue, greenValue, blueValue);
            ledbuffer_write (leds);
            ledbuffer_advance (leds, 1);
        }
        for (;greenValue>=0, redValue < 255; greenValue--, redValue++)
        {
            ledbuffer_set(leds, NUM_LEDS / 2, redValue, greenValue, blueValue);
            ledbuffer_write (leds);
            ledbuffer_advance (leds, 1);
        }
    }
}