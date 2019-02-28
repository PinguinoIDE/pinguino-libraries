/*  ----------------------------------------------------------------------------
    blink built-in led with help from interrupt library
    ----------------------------------------------------------------------------
    author:        Régis Blanchot
    first release: 19/12/2010
    last update:   25/11/2013
    pinguino ide:  > 11.4
    -------------------------------------------------------------------------*/

u16 volatile flag = 0;
u16 volatile cycles;

void setup()
{
    pinMode(USERLED, OUTPUT);
    
    // Set Timer1 overflow to 1ms
    // 1 ms = 1 / 1 000 sec
    // 1 cycle = Tcy = 1 / Fcy = 4 / Fosc (Fcy = Fosc / 4)
    // nb of cycles in 1 ms = ( 1 / 1000 ) / Tcy
    //                      = Fcy / 1000
    //                      = System.getPeripheralFrequency() / 1000    
    cycles = 0xFFFF - ( System.getPeripheralFrequency() / 1000 );
    TMR1H = highByte(cycles);
    TMR1L =  lowByte(cycles);
    T1CON = T1_ON | T1_16BIT | T1_SOURCE_FOSCDIV4 | T1_PS_1_1;

    Int.clearFlag(INT_TMR1);
    Int.enable(INT_TMR1);
    Int.start();
}

void loop()
{
    // Whatever you want here
}

void interrupt()
{
    if (Int.isFlagSet(INT_TMR1) == 1)
    {
        flag++;
        TMR1H = highByte(cycles);
        TMR1L =  lowByte(cycles);
        Int.clearFlag(INT_TMR1);
        if (flag >= 1000) // 1000 ms
        {
            flag = 0;
            toggle(USERLED);
        }        
    }
}
