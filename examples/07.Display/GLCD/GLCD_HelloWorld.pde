/**
    Author:    Régis Blanchot (Nov. 2016)
    Tested on: Pinguino 47J53
    Output:    Graphic LCD 128x64 with KS0108 Controller
    Wiring :   (Panel E)
    
    1	Vss	Ground on breadboard
    2	Vdd	+5V on breadboard
    3 	Vout	Center pin of 10k pot
    4	RS	Pin 8
    5	RW	Pin 9
    6	E	Pin 10
    7	DB0	Pin 0
    8	DB1	Pin 1
    9	DB2	Pin 2
    10	DB3	Pin 3
    11	DB4	Pin 4
    12	DB5	Pin 5
    13	DB6	Pin 6
    14	DB7	Pin 7
    15	CS1	Pin 11
    16	CS2	Pin 12
    17	/RST	Pin 13
    18	Vee	An end of a 10k pot
    19	LED+	+5V on breadboard
    20	LED-	Ground on breadboard
**/

#include <fonts/font5x7.h>        // font system
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
//#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

u8 i;

void setup()
{
    pinMode(USERLED, OUTPUT);
	
    // Initialize the LCD
    // rs (di), rw, en, cs1, cs2, rst, d0 to d7
    GLCD.init(8,9,10,11,14,13,0,1,2,3,4,5,6,7);

    // Set font
    //GLCD.setFont(font5x7);
    //GLCD.setFont(Corsiva12);
    //GLCD.setFont(Arial14);
    GLCD.setFont(ArialBold14);
    //GLCD.setFont(VerdanaBold28);

    //Print "Hello World !"
    GLCD.setCursor(0, 2);
    GLCD.printCenter("Hello World!");
    
    // Wait for 5 seconds (5000 ms)
    delay(5000);
    GLCD.clearScreen();

    // Set a new font
    GLCD.setFont(font5x7);
}

//u8 x, y, angle, radius;

void loop()
{
    GLCD.printf("i=%d\r\n", i++);
    /* Low footprint version :
    GLCD.print("i=");
    GLCD.printNumber(i++, DEC);
    GLCD.print("\r\n");
    */
    toggle(USERLED);
/*
    for (x = 0; x < KS0108.screen.width; x++)
    {    
        angle = x * 360 / KS0108.screen.width;
        radius = KS0108.screen.height / 2;
        y = radius - (radius * cos100(angle)) / 100;
        GLCD.drawPixel(x, y);
        y = radius - (radius * sin100(angle)) / 100;
        GLCD.clearScreen();
        GLCD.setCursor(0, 8);
        GLCD.printf("(x,y)=(%d,%d)  ", x, y);
        GLCD.drawCircle(x, y, 3);
        toggle(USERLED);
        delay(100);
    }
*/
}
