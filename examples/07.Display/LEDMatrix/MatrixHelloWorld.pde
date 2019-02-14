#define MATRIX 3 // number of matrix

u8 i;

void setup()
{
    // SPI SOFTWARE
    // pin 3 is connected to the DataIn 
    // pin 1 is connected to the CLK 
    // pin 13 is connected to the CS  
    LedControl.init(SPISW, 3, 1, 13, MATRIX);

    // SPI HARDWARE
    //LedControl.init(SPI1, MATRIX);


    // MAX72XX are in power-saving mode on startup,
    // we have to do a wakeup call
    LedControl.powerOn();
    // Set the brightness to a medium value (0~15 possible values)
    LedControl.setIntensity(1);
}

void loop()
{
    // display chars one after one
    LedControl.clearAll();
    for (i='A'; i<='Z'; i++)
    {
        LedControl.printChar(i);
        delay(333);
    }
    // display the string char by char
    LedControl.clearAll();
    for (i=0; i<=5; i++)
    {
        LedControl.print("Hello World!");
        delay(1000);
    }
    // scroll the string 1 pixel to the left
    LedControl.clearAll();
    for (i=0; i<(15*8); i++)
    {
        LedControl.scroll("www.pinguino.cc");
    }

    // display the number with printf
    LedControl.clearAll();
    for (i=0; i<=255; i++)
        LedControl.printf("%03d", i);

    // display the number with printNumber
    for (i=0; i<=255; i++)
    {
        LedControl.clearAll();
        LedControl.printNumber(i, DEC);
        delay(150);
    }
}
