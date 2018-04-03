/*  ---------------------------------------------------------------------------
    Blink a LED connected to RC2
    author:	Régis Blanchot
    --------------------------------------------------------------------------*/

void setup()
{
    TRISBbits.TRISB13 = OUTPUT;
}

void loop()
{
    LATBbits.LATB13 ^= 1;
    delay(500);
}
