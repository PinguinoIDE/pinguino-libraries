/*  ---------------------------------------------------------------------------
    Blink a LED connected to RC2
    author:	Régis Blanchot
    --------------------------------------------------------------------------*/

void setup()
{
    TRISCbits.TRISC2 = OUTPUT;
}

void loop()
{
    LATCbits.LATC2 ^= 1;
    delay(500);
}
