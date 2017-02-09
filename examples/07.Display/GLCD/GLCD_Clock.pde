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

//Buttons
#define HOUR 68
#define MIN 69

//Degrees
#define A60 360/60
#define A12 360/12

//Clock size
#define RADIUS 18

//Fonts
#include <fonts/font5x7.h>        // font system
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
//#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

//global vars
u8 counter = 0;
// 10h15mn37s
u8 hour = 10;
u8 min  = 15;
u8 sec  = 37;

void setup()
{
    pinMode(USERLED, OUTPUT);
    
    //Setup buttons
    pinMode(HOUR, INPUT);
    pinMode(MIN, INPUT);
    
    //Setup GLCD
    // Initialize the LCD
    // rs (di), rw, en, cs1, cs2, rst, d0 to d7
    GLCD.init(8,9,10,11,14,13,0,1,2,3,4,5,6,7);
}

void loop()
{
    u8 xo = KS0108.screen.width/2;
    u8 yo = KS0108.screen.height/2;
    u8 x, y;
    
    if (counter>9)
    {
        counter = 0;
        sec++;
    }
    
    if (sec>59)
    {
        sec = 0;
        min++;
    }
    
    if (min>59)
    {
        min = 0;
        hour++;
    }
    
    if (hour>11)
    {
        hour = 0;
    }
    
    GLCD.setFont(ArialBold14);
    GLCD.goto(0,0);
    
    GLCD.printf("%02d:%02d:%02d", hour, min, sec);
    /* or ...
    if (hour < 10)
        GLCD.printChar('0');
    GLCD.printNumber(hour, DEC);
    
    GLCD.printChar(':');
    if (min < 10)
        GLCD.printChar('0');
    GLCD.printNumber(min, DEC);
    
    GLCD.printChar(':');
    if (sec < 10)
        GLCD.printChar('0');
    GLCD.printNumber(sec, DEC);
    */
    
    //Display ticker 1/100 seconds ...
    GLCD.setFont(font5x7);
    GLCD.printNumber(counter, DEC);
    
    //Draw the clock         
    GLCD.drawCircle(xo, yo, RADIUS);
    //Seconds
    x = xo + (RADIUS * cos100(A60*sec-90)) / 100;
    y = yo + (RADIUS * sin100(A60*sec-90)) / 100;
    GLCD.drawLine(xo, yo, x, y);
    //Minutes
    x = xo + (RADIUS * cos100(A60*min-90)) / 100;
    y = yo + (RADIUS * sin100(A60*min-90)) / 100;
    GLCD.drawLine(xo, yo, x, y);
    //Hours
    x = xo + (RADIUS * cos100(A12*hour-90)) / 100;
    y = yo + (RADIUS * sin100(A12*hour-90)) / 100;
    GLCD.drawLine(xo, yo, x, y);

    //Wait for 100ms
    delay(50);
    toggle(USERLED);
    
    if(!counter)
        GLCD.clearScreen();
    
    counter++;
 }
