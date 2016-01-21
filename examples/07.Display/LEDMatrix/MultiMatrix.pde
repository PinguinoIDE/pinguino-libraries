#define MATRIX 3 // number of matrix

void setup()
{
    /*
    We need a LedControl to work with.
    pin 0 is connected to the DataIn  
    pin 1 is connected to the CLK 
    pin 2 is connected to LOAD 
    */
    LedControl.init(0, 1, 2, MATRIX);

    /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
    */
    LedControl.powerOn();
    /* Set the brightness to a medium value (0~15 possible values) */
    LedControl.setIntensity(8);
    /* and clear the display */
    LedControl.clearAll();
}

void loop()
{ 
    /* scroll the string 1 pixel to the left */
    LedControl.scroll("   3x Led Matrix Scroll Demo - www.pinguino.cc   ");
}
