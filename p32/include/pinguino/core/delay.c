/*	----------------------------------------------------------------------------
    delay.c
    Régis Blanchot
    ----------------------------------------------------------------------------
    Modified JPM 21/12/2010
    - delay_us replaced with Delayus
    - delay_ms replaced with Delayms
    Modified RB 10/01/2011
    - use of GetSystemClock() function to fit to every CPU configuration
    Modified RB 12/02/2011
    - use of Core Timer (GetCP0Count() function)
    --------------------------------------------------------------------------*/

#ifndef __DELAY_C
    #define __DELAY_C

//#include <typedef.h>
#include <system.c>

/*	--------------------------------------------------------------------
    Wait us microseconds
    Uses CP0 Count register which counts at half the CPU rate
    ------------------------------------------------------------------*/

void Delayus(int us)
{
    // get start ticks
    int start = GetCP0Count();

    // CP0Count counts at half the CPU rate
    int Fcp0 = GetSystemClock() / 1000000 / 2;		// max = 40 for 80MHz

    // calculate last tick number for the given number of microseconds
    int stop = start + us * Fcp0;

    // wait until count reaches the stop value
    if (stop > start)
        while (GetCP0Count() < stop);
    else
        while (GetCP0Count() > start || GetCP0Count() < stop); 
} 

/*	--------------------------------------------------------------------
    Wait ms milliseconds
    ------------------------------------------------------------------*/

void Delayms(int ms)
{
    do
    {
        Delayus(1000); // 1 ms
    }
    while(--ms);
}

#endif	/* __DELAY_C */
