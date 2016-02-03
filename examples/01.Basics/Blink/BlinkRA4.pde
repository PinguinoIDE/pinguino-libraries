/*  ----------------------------------------------------------------------------
    Blink a LED connected to RA4
    author:	Régis Blanchot
    --------------------------------------------------------------------------*/

void setup()
{
    TRISAbits.TRISA4 = OUTPUT;
}

void loop()
{
    LATAbits.LATA4 ^= 1;
    delay(50);
}
