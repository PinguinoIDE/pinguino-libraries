/*	----------------------------------------------------------------------------
	interrupt library demo
	----------------------------------------------------------------------------
	author:		Régis Blanchot
	first release:	16/12/2015
	last update:	16/12/2015
	pinguino ide:	12
 	boards:		8-bit ONLY
	--------------------------------------------------------------------------*/


void blink()
{
    if (INTRB & INTRB4) toggle(0);
    if (INTRB & INTRB7) toggle(USERLED);  
}

void setup()
{
    OnChangePin4to7(blink, INTRB4|INTRB7);
}

void loop()
{
}
