//	watchdog.c
//	pic32-pinguino watchdog lib
//	djpark@astsb.info
//  11-06-2015 : rblanchot@gmail.com : renamed all the functions

#ifndef __WATCHDOG__
#define __WATCHDOG__

#include <p32xxxx.h>

//--	watchdog in lib
#define Watchdog_enable()         (WDTCONSET = _WDTCON_ON_MASK)
#define Watchdog_disable()        (WDTCONCLR = _WDTCON_ON_MASK)
#define Watchdog_clear()          (WDTCONSET = _WDTCON_WDTCLR_MASK)
#define Watchdog_clearEvent()     (RCONCLR = _RCON_WDTO_MASK)
#define Watchdog_readEvent()      (RCONbits.WDTO)
#define Watchdog_readPostscaler() (WDTCONbits.WDTPSTA)

u8 boot_from_watchdog = 0;

void watchdog_init()
{
	// enable watchdog (8.2 seconds)
	if (Watchdog_readEvent())
		boot_from_watchdog = 1;
	//Watchdog_enable();
	Watchdog_disable();
	Watchdog_clearEvent();
	Watchdog_clear();
}

#define Watchdog_event() (boot_from_watchdog)

#endif	// __WATCHDOG__

