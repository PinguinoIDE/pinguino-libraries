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

u8 x, y, angle, radius;

void setup()
{
    pinMode(USERLED, OUTPUT);
	
    // Initialize the LCD
    // rs (di), rw, en, cs1, cs2, rst, d0 to d7
    GLCD.init(8,9,10,11,14,13,0,1,2,3,4,5,6,7);
}

void loop()
{
    radius = KS0108.screen.height / 2;
    
    x = (x + 1) % KS0108.screen.width;
    angle = x * 360 / KS0108.screen.width;
    y = radius - (radius * cos100(angle)) / 100;
    //y = radius + radius * sinr(angle);
    GLCD.drawPixel(x, y);

    if (x==0)
        toggle(USERLED);
}
