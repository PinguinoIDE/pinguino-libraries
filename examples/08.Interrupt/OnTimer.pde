/*	----------------------------------------------------------------------------
	interrupt library demo
	----------------------------------------------------------------------------
	author:		Régis Blanchot
	first release:	19/12/2010
	last update:	06/02/2015
	pinguino ide:	11
--------------------------------------------------------------------------*/

void blink() { toggle(USERLED); }

void setup()
{
    pinMode(USERLED,OUTPUT);

    // Use Timer to toggle USERLED every 500 ms
    OnTimer0(blink, INT_MILLISEC, 500);
    //OnTimer1(blink, INT_MILLISEC, 500);
    //OnTimer2(blink, INT_MILLISEC, 500);
}

void loop()
{
    // your code here
}
