/*	----------------------------------------------------------------------------
	Led Dimmer
	----------------------------------------------------------------------------
	author: 			Régis Blanchot
	description:	This example shows how to fade a LED connected to PWM pin
					      using PWM functions.
 	first release:25/02/2012
	last update:	25/02/2012
	pinguino ide:	> 9.5
	----------------------------------------------------------------------------
	wiring:			pin ----- +LED- ----- 330 Ohm Resistor ----- GND
	----------------------------------------------------------------------------
	NB:			PIC18 PINGUINO x550	 can only use pins	D11, D12
	    		PIC18 PINGUINO 26j50 can only use pins	D10, D11
					PIC32 PINGUINO		   can only use pins	D0, D1, D2
					PIC32 PINGUINO OTG	 can only use pins	D0, D1, D2
					PIC32 PINGUINO 220	 can only use pins	D2, D3, D11, D12, D13
	--------------------------------------------------------------------------*/

#define MYLED 11

u8 i = 0;
s8 dir;

void setup()
{
    // Frequency (Hz) must be high to avoid blinking effect
    PWM.setFrequency((36*1000));
}

void loop()
{
    // Duty Cycle from 0% to 50% and back
    if (i > 50) dir = -1;
    if (i <= 0) dir =  1;
    i = i + dir;
    // Duty Cycle is a percentage measure of the time that the LED is physically on.
    PWM.setPercentDutyCycle(MYLED, 50);
    // Delay of 25 ms
    delay(25);
}
